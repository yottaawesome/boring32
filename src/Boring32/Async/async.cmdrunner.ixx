export module boring32:async.cmdrunner;
import std;
import :win32;
import :error;
import :strings;
import :raii;

namespace Boring32::Async
{
    auto ReadFromPipe(Win32::HANDLE childStdOutRd) -> std::string
    {
		if (not childStdOutRd)
            throw Error::Boring32Error("Invalid handle for reading from child process.");
        constexpr auto BufferSize = 4096;
        auto readBuffer = std::array<char, BufferSize>{};
        auto result = std::string{};
        for (;;)
        {
            auto bytesRead = Win32::DWORD{};
            auto success = Win32::ReadFile(childStdOutRd, readBuffer.data(), BufferSize, &bytesRead, nullptr);
            if (not success or bytesRead == 0)
                break;
            result += std::string(readBuffer.data(), bytesRead);
        }
        return result;
    }

    auto CreateChildProcess(
        const std::wstring& path,
        const std::wstring& cmdline,
        Win32::HANDLE childStdOutWr,
        Win32::HANDLE childStdInRd
    ) -> Win32::PROCESS_INFORMATION
    {
        auto piProcInfo = Win32::PROCESS_INFORMATION{};
        auto siStartInfo = Win32::STARTUPINFO{
            .cb = sizeof(Win32::STARTUPINFO),
            .dwFlags = Win32::StartFUsesStdHandle,
            .hStdInput = childStdInRd,
            .hStdOutput = childStdOutWr,
            .hStdError = childStdOutWr,
        };

        // Create the child process. 
        auto success = Win32::CreateProcessW(
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
        if (not success)
            throw Error::Win32Error{ Win32::GetLastError(), "CreateProcessW() failed" };

        return piProcInfo;
    }

    auto ParseWMICGetBios(const std::string& cmdOutput) -> std::string
    {
        auto tokens = std::vector<std::string>{ Strings::TokeniseString(cmdOutput, "\r\r\n") };
        auto nextLine = false;
        auto serialNumber = std::string{};
        for (const auto& token : tokens)
        {
            if (nextLine)
            {
                serialNumber = token;
                break;
            }
            nextLine = token.starts_with("SerialNumber");
        }

        // Seems to come with two extra spaces at the end
		while (serialNumber.ends_with(' '))
             serialNumber.pop_back();

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
        auto saAttr = Win32::SECURITY_ATTRIBUTES{
            .nLength = sizeof(Win32::SECURITY_ATTRIBUTES),
            .lpSecurityDescriptor = nullptr,
            .bInheritHandle = true
        };

        // Create a pipe for the child process's STDOUT. 
        auto childStdOutRd = Win32::HANDLE{};
        auto childStdOutWr = Win32::HANDLE{};
        if (not Win32::CreatePipe(&childStdOutRd, &childStdOutWr, &saAttr, 0))
            throw Error::Win32Error(GetLastError(), "CreatePipe() failed");
        auto ptrChildStdOutWr = RAII::HandleUniquePtr(childStdOutWr);
        auto ptrChildStdOutRd = RAII::HandleUniquePtr(childStdOutRd);

        // Ensure the read handle to the pipe for STDOUT is not inherited.
        if (not Win32::SetHandleInformation(childStdOutRd, Win32::HandleFlagInherit, 0))
            throw Error::Win32Error(GetLastError(), "CreatePipe() failed");

        // Create a pipe for the child process's STDIN. 
        auto childStdInRd = Win32::HANDLE{};
        auto childStdInWr = Win32::HANDLE{};
        if (not Win32::CreatePipe(&childStdInRd, &childStdInWr, &saAttr, 0))
            throw Error::Win32Error(GetLastError(), "CreatePipe() failed");
        auto ptrChildStdInRd = RAII::HandleUniquePtr(childStdInRd);
        auto ptrChildStdInWr = RAII::HandleUniquePtr(childStdInWr);
        // Ensure the write handle to the pipe for STDIN is not inherited. 
        if (not Win32::SetHandleInformation(childStdInWr, Win32::HandleFlagInherit, 0))
            throw Error::Win32Error(GetLastError(), "CreatePipe() failed");

        // Create the child process. 
        auto childProc = CreateChildProcess(
            LR"(C:\Windows\System32\cmd.exe)",
            cmd,
            childStdOutWr,
            childStdInRd
        );
        auto ptrChildProcess = RAII::HandleUniquePtr(childProc.hProcess);
        auto ptrChildThread = RAII::HandleUniquePtr(childProc.hThread);

        // As per the official sample, these need to be closed.
        ptrChildStdOutWr.reset();
        ptrChildStdInRd.reset();

        if constexpr (not std::is_null_pointer_v<decltype(FParser)>)
        {
            // Read from pipe that is the standard output for child process. 
            auto output = ReadFromPipe(childStdOutRd);
            return FParser(output);
        }
    }

    template<Strings::FixedString FCmd, auto FParser = nullptr>
    struct CmdRunner
    {
        // Once MSVC supports C++23's static operator(), this can made static.
        static auto operator()()
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
    constexpr auto GetWMICBiosCommand = Strings::FixedString{LR"(C:\Windows\System32\cmd.exe /c wmic bios get serialnumber)"};
    constexpr auto GetBIOS = CmdRunner<GetWMICBiosCommand, ParseWMICGetBios>{};
}
