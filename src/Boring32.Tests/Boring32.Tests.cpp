#include <format>
#include <iostream>
#include <windows.h>
#include "Boring32/include/Raii/Win32Handle.hpp"

#include "Experiments1Main.hpp"

import boring32.strings;

int main(int argc, char** args)
{
	//Experiments1Main();

	Boring32::Raii::Win32Handle m_readHandle;
	Boring32::Raii::Win32Handle m_writeHandle;
	bool succeeded = CreatePipe(&m_readHandle, &m_writeHandle, nullptr, 512);
	if (succeeded == false)
		throw std::runtime_error("Failed to create anonymous pipe");

	return 0;
}
