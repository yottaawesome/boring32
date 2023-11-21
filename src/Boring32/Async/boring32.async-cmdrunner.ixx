export module boring32.async:cmdrunner;
import std;
import std.compat;
import boring32.win32;
import boring32.error;
import boring32.strings;

namespace Boring32::Async
{
    struct HandlerDeleter
    {
        void operator()(Win32::HANDLE h) { Win32::CloseHandle(h); }
    };
    using HandleUniquePtr = std::unique_ptr<std::remove_pointer_t<Win32::HANDLE>, HandlerDeleter>;

    constexpr int BUFSIZE = 4096;

    std::string ReadFromPipe(Win32::HANDLE hChildStd_OUT_Rd)
    {
        char chBuf[BUFSIZE];
        Win32::HANDLE hParentStdOut = Win32::GetStdHandle(Win32::StdOutputHandle);

        std::string result;
        for (;;)
        {
            Win32::DWORD dwRead;
            bool bSuccess = Win32::ReadFile(hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, nullptr);
            if (!bSuccess || dwRead == 0)
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
        if (!bSuccess)
        {
            const auto lastError = Win32::GetLastError();
            throw Error::Win32Error("CreateProcessW() failed", lastError);
        }

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
        if (!Win32::CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0))
        {
            const auto lastError = GetLastError();
            throw Error::Win32Error("CreatePipe() failed", lastError);
        }
        HandleUniquePtr ptrChildStdOutWr = HandleUniquePtr(hChildStd_OUT_Wr);
        HandleUniquePtr ptrChildStdOutRd = HandleUniquePtr(hChildStd_OUT_Rd);

        // Ensure the read handle to the pipe for STDOUT is not inherited.
        if (!Win32::SetHandleInformation(hChildStd_OUT_Rd, Win32::HandleFlagInherit, 0))
        {
            const auto lastError = GetLastError();
            throw Error::Win32Error("CreatePipe() failed", lastError);
        }

        // Create a pipe for the child process's STDIN. 
        Win32::HANDLE hChildStd_IN_Rd = nullptr;
        Win32::HANDLE hChildStd_IN_Wr = nullptr;
        if (!Win32::CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0))
        {
            const auto lastError = GetLastError();
            throw Error::Win32Error("CreatePipe() failed", lastError);
        }
        HandleUniquePtr ptrChildStdInRd = HandleUniquePtr(hChildStd_IN_Rd);
        HandleUniquePtr ptrChildStdInWr = HandleUniquePtr(hChildStd_IN_Wr);

        // Ensure the write handle to the pipe for STDIN is not inherited. 
        if (!Win32::SetHandleInformation(hChildStd_IN_Wr, Win32::HandleFlagInherit, 0))
        {
            const auto lastError = GetLastError();
            throw Error::Win32Error("CreatePipe() failed", lastError);
        }

        // Create the child process. 
        Win32::PROCESS_INFORMATION childProc = CreateChildProcess(
            LR"(C:\Windows\System32\cmd.exe)",
            cmd,
            hChildStd_OUT_Wr,
            hChildStd_IN_Rd
        );
        HandleUniquePtr ptrChildProcess = HandleUniquePtr(childProc.hProcess);
        HandleUniquePtr ptrChildThread = HandleUniquePtr(childProc.hThread);

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