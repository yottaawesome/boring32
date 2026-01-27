export module boring32:async.cmdrunner;
import std;
import :win32;
import :error;
import :strings;
import :raii;

namespace Boring32::Async
{
    constexpr int BufferSize = 4096;

    std::string ReadFromPipe(Win32::HANDLE hChildStd_OUT_Rd)
    {
        char chBuf[BufferSize];
        Win32::HANDLE hParentStdOut = Win32::GetStdHandle(Win32::StdOutputHandle);

        std::string result;
        for (;;)
        {
            Win32::DWORD dwRead;
            bool bSuccess = Win32::ReadFile(hChildStd_OUT_Rd, chBuf, BufferSize, &dwRead, nullptr);
            if (not bSuccess or dwRead == 0)
                break;

            result += std::string(chBuf, dwRead);
        }
        return result;
    }

    Win32::PROCESS_INFORMATION CreateChildProcess(
        const std::wstring& path,
        const std::wstring& cmdline,
        Win32::HANDLE hChildStd_OUT_Wr,
        Win32::HANDLE hChildStd_IN_Rd
    )
    {
        Win32::PROCESS_INFORMATION piProcInfo{ 0 };
        Win32::STARTUPINFO siStartInfo{
            .cb = sizeof(Win32::STARTUPINFO),
            .dwFlags = Win32::StartFUsesStdHandle,
            .hStdInput = hChildStd_IN_Rd,
            .hStdOutput = hChildStd_OUT_Wr,
            .hStdError = hChildStd_OUT_Wr,
        };
        bool bSuccess = false;

        // Create the child process. 
        bSuccess = Win32::CreateProcessW(
            path.empty() ? nullptr : const_cast<wchar_t*>(path.c_str()),
            const_cast<wchar_t*>(cmdline.c_str()),     // command line 
            nullptr,          // process security attributes 
            nullptr,          // primary thread security attributes 
            true,          // handles are inherited 
            0,             // creation flags 
            nullptr,          // use parent's environment 
            nullptr,          // use parent's current directory 
            &siStartInfo,  // STARTUPINFO pointer 
            &piProcInfo     // receives PROCESS_INFORMATION 
        );
        if (not bSuccess)
            throw Error::Win32Error(Win32::GetLastError(), "CreateProcessW() failed");

        return piProcInfo;
    }

    std::string ParseWMICGetBios(const std::string& cmdOutput)
    {
        std::vector tokens = Strings::TokeniseString(cmdOutput, "\r\r\n");
        bool nextLine = false;
        std::string serialNumber;
        for (const std::string token : tokens)
        {
            if (nextLine)
            {
                serialNumber = token;
                break;
            }
            nextLine = token.starts_with("SerialNumber");
        }

        // Seems to come with two extra spaces at the end
        if (serialNumber.size() > 2)
        {
            serialNumber.pop_back();
            serialNumber.pop_back();
        }

        Strings::Trim(serialNumber);
        return serialNumber;
    }
}

export namespace Boring32::Async
{
    template<auto FParser = nullptr>
    auto Exec(const std::wstring& cmd)
    {
        // Get a handle to an input file for the parent. 
        // This example assumes a plain text file and uses string output to verify data flow. 
        if (cmd.empty())
            throw Error::Boring32Error("Please specify a cmd command.");

        // Set the bInheritHandle flag so pipe handles are inherited.
        Win32::SECURITY_ATTRIBUTES saAttr{
            .nLength = sizeof(Win32::SECURITY_ATTRIBUTES),
            .lpSecurityDescriptor = nullptr,
            .bInheritHandle = true
        };

        // Create a pipe for the child process's STDOUT. 
        Win32::HANDLE hChildStd_OUT_Rd = nullptr;
        Win32::HANDLE hChildStd_OUT_Wr = nullptr;
        if (not Win32::CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0))
            throw Error::Win32Error(GetLastError(), "CreatePipe() failed");
        RAII::HandleUniquePtr ptrChildStdOutWr = RAII::HandleUniquePtr(hChildStd_OUT_Wr);
        RAII::HandleUniquePtr ptrChildStdOutRd = RAII::HandleUniquePtr(hChildStd_OUT_Rd);

        // Ensure the read handle to the pipe for STDOUT is not inherited.
        if (not Win32::SetHandleInformation(hChildStd_OUT_Rd, Win32::HandleFlagInherit, 0))
            throw Error::Win32Error(GetLastError(), "CreatePipe() failed");

        // Create a pipe for the child process's STDIN. 
        Win32::HANDLE hChildStd_IN_Rd = nullptr;
        Win32::HANDLE hChildStd_IN_Wr = nullptr;
        if (not Win32::CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0))
            throw Error::Win32Error(GetLastError(), "CreatePipe() failed");
        RAII::HandleUniquePtr ptrChildStdInRd = RAII::HandleUniquePtr(hChildStd_IN_Rd);
        RAII::HandleUniquePtr ptrChildStdInWr = RAII::HandleUniquePtr(hChildStd_IN_Wr);

        // Ensure the write handle to the pipe for STDIN is not inherited. 
        if (not Win32::SetHandleInformation(hChildStd_IN_Wr, Win32::HandleFlagInherit, 0))
            throw Error::Win32Error(GetLastError(), "CreatePipe() failed");

        // Create the child process. 
        Win32::PROCESS_INFORMATION childProc = CreateChildProcess(
            LR"(C:\Windows\System32\cmd.exe)",
            cmd,
            hChildStd_OUT_Wr,
            hChildStd_IN_Rd
        );
        RAII::HandleUniquePtr ptrChildProcess = RAII::HandleUniquePtr(childProc.hProcess);
        RAII::HandleUniquePtr ptrChildThread = RAII::HandleUniquePtr(childProc.hThread);

        // As per the official sample, these need to be closed.
        ptrChildStdOutWr.reset();
        ptrChildStdInRd.reset();

        if constexpr (not std::is_null_pointer_v<decltype(FParser)>)
        {
            // Read from pipe that is the standard output for child process. 
            std::string output = ReadFromPipe(hChildStd_OUT_Rd);
            return FParser(output);
        }
    }

    template<Strings::FixedString FCmd, auto FParser = nullptr>
    struct CmdRunner
    {
        // Once MSVC supports C++23's static operator(), this can made static.
        auto operator()() const
        {
            return Exec<FParser>(std::wstring{ FCmd });
        }

        static auto Exec()
        {
            return ::Exec<FParser>(std::wstring{ FCmd });
        }
    };   
}

export namespace Boring32::Async::Commands
{
    constexpr Strings::FixedString GetWMICBiosCommand = LR"(C:\Windows\System32\cmd.exe /c wmic bios get serialnumber)";

    // Once MSVC supports C++23's static operator(), this can be simplified
    // into a using statement.
    constexpr CmdRunner<GetWMICBiosCommand, ParseWMICGetBios> GetBIOS;
}