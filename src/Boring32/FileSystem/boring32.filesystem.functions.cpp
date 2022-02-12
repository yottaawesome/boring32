module;

#include <string>
#include <format>
#include <vector>
#include <stdexcept>
#include <source_location>
#include <filesystem>
#include <Windows.h>

module boring32.filesystem.functions;
import boring32.error.win32error;
import boring32.strings;

namespace Boring32::FileSystem
{
	std::wstring GetFileVersion(const std::wstring& filePath)
	{
        // Adapted from https://stackoverflow.com/questions/940707/how-do-i-programmatically-get-the-version-of-a-dll-or-exe-file
        // See also https://docs.microsoft.com/en-us/windows/win32/menurc/version-information
        if (!std::filesystem::exists(filePath))
            throw std::runtime_error(
                std::format("{}: file {} does not exist", __FUNCSIG__, Strings::ConvertString(filePath)));

        DWORD verHandle = 0;
        // https://docs.microsoft.com/en-us/windows/win32/api/winver/nf-winver-getfileversioninfosizew
        const DWORD verSize = GetFileVersionInfoSizeW(filePath.c_str(), &verHandle);
        if (!verSize)
            throw Error::Win32Error(std::source_location::current(), "GetFileVersionInfoSizeW() failed",GetLastError());

        std::vector<std::byte> verData(verSize);
        // https://docs.microsoft.com/en-us/windows/win32/api/winver/nf-winver-getfileversioninfow
        if (!GetFileVersionInfoW(filePath.c_str(), verHandle, verSize, &verData[0]))
            throw Error::Win32Error(std::source_location::current(), "GetFileVersionInfoW() failed", GetLastError());

        UINT size = 0;
        LPBYTE lpBuffer = nullptr;
        // https://docs.microsoft.com/en-us/windows/win32/api/winver/nf-winver-verqueryvaluew
        if (!VerQueryValueW(&verData[0], L"\\", reinterpret_cast<void**>(&lpBuffer), &size))
            throw std::runtime_error(__FUNCSIG__": could not determine version info (VerQueryValueW() failed)");
        if (!size)
            throw std::runtime_error(__FUNCSIG__": could not determine version info (size was zero)");

        //https://docs.microsoft.com/en-us/windows/win32/api/verrsrc/ns-verrsrc-vs_fixedfileinfo
        const VS_FIXEDFILEINFO* verInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(lpBuffer);
        if (verInfo->dwSignature != 0xfeef04bd)
            throw std::runtime_error(__FUNCSIG__": could not determine version info (invalid signature)");

        // Doesn't matter if you are on 32 bit or 64 bit,
        // DWORD is always 32 bits, so first two revision numbers
        // come from dwFileVersionMS, last two come from dwFileVersionLS
        return std::format(
            L"{}.{}.{}.{}", // major.minor.build.revision
            (verInfo->dwFileVersionMS >> 16) & 0xffff,
            (verInfo->dwFileVersionMS >> 0) & 0xffff,
            (verInfo->dwFileVersionLS >> 16) & 0xffff,
            (verInfo->dwFileVersionLS >> 0) & 0xffff
        );
	}
}