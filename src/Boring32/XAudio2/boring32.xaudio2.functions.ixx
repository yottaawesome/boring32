module;

#include <Windows.h>

export module boring32.xaudio2.functions;

export namespace Boring32::XAudio2
{
	void FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
}