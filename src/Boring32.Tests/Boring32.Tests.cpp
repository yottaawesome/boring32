#include <format>
#include <iostream>
#include <windows.h>
#include "Boring32/include/Raii/Win32Handle.hpp"

#include "Experiments1Main.hpp"

import boring32.strings;

struct Test
{
	static void DoIt() { std::wcout << L"OK\n"; }
};

template<typename T>
void X()
{
	T::DoIt();
}


int main(int argc, char** args)
{
	//Experiments1Main();
	X<Test>();

	Boring32::Raii::Win32Handle m_readHandle;
	Boring32::Raii::Win32Handle m_writeHandle;
	if (!CreatePipe(&m_readHandle, &m_writeHandle, nullptr, 512))
		throw std::runtime_error("Failed to create anonymous pipe");

	m_readHandle = m_writeHandle;

	return 0;
}
