export module boring32:xaudio2_functions;
import boring32.shared;
import boring32.error;

namespace Boring32::XAudio2
{
	constexpr Win32::DWORD fourccRIFF = 'FFIR';
	constexpr Win32::DWORD fourccDATA = 'atad';
	constexpr Win32::DWORD fourccFMT = 'tmf'; // originally ' tmf';
	constexpr Win32::DWORD fourccWAVE = 'EVAW';
	constexpr Win32::DWORD fourccXWMA = 'AMWX';
	constexpr Win32::DWORD fourccDPDS = 'sdpd';
}

export namespace Boring32::XAudio2
{
	void FindChunk(
		Win32::HANDLE hFile, 
		Win32::DWORD fourcc, 
		Win32::DWORD& dwChunkSize,
		Win32::DWORD& dwChunkDataPosition
	)
    {
        Win32::DWORD result = Win32::SetFilePointer(hFile, 0, nullptr, Win32::FileBegin);
        if (result == Win32::InvalidSetFilePointer)
            throw Error::Win32Error(
                "Win32::SetFilePointer() failed",
                Win32::GetLastError()
            );

        Win32::DWORD dwChunkType;
        Win32::DWORD dwChunkDataSize;
        Win32::DWORD dwRIFFDataSize = 0;
        Win32::DWORD dwFileType;
        Win32::DWORD bytesRead = 0;
        Win32::DWORD dwOffset = 0;

        Win32::HRESULT hr = Win32::S_Ok;
        while (hr == Win32::S_Ok)
        {
            Win32::DWORD dwRead;
            if (!Win32::ReadFile(hFile, &dwChunkType, sizeof(Win32::DWORD), &dwRead, nullptr))
            {
                const auto lastError = Win32::GetLastError();
                throw Error::Win32Error(
                    "Win32::ReadFile() failed",
                    lastError
                );
            }

            if (!Win32::ReadFile(hFile, &dwChunkDataSize, sizeof(Win32::DWORD), &dwRead, nullptr))
            {
                const auto lastError = Win32::GetLastError();
                throw Error::Win32Error(
                    "Win32::ReadFile() failed",
                    lastError
                );
            }

            switch (dwChunkType)
            {
                case fourccRIFF:
                    dwRIFFDataSize = dwChunkDataSize;
                    dwChunkDataSize = 4;
                    if (!Win32::ReadFile(hFile, &dwFileType, sizeof(Win32::DWORD), &dwRead, nullptr))
                    {
                        const auto lastError = Win32::GetLastError();
                        throw Error::Win32Error(
                            "Win32::ReadFile() failed",
                            lastError
                        );
                    }
                    break;

                default:
                    result = Win32::SetFilePointer(
                        hFile,
                        dwChunkDataSize,
                        nullptr,
                        Win32::FileCurrent
                    );
                    if (result == Win32::InvalidSetFilePointer)
                    {
                        const auto lastError = Win32::GetLastError();
                        throw Error::Win32Error(
                            "Win32::SetFilePointer() failed",
                            lastError
                        );
                    }
            }

            dwOffset += sizeof(Win32::DWORD) * 2;

            if (dwChunkType == fourcc)
            {
                dwChunkSize = dwChunkDataSize;
                dwChunkDataPosition = dwOffset;
                break;
            }
            dwOffset += dwChunkDataSize;
            if (bytesRead >= dwRIFFDataSize)
                throw Error::Boring32Error("bytesRead >= dwRIFFDataSize");
        }
    }
}
