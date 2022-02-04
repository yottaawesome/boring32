#include <format>
#include <iostream>
#include <stdexcept>
#include <source_location>
#include <windows.h>
#include "Experiments1Main.hpp"

import boring32.raii.win32handle;
import boring32.strings;
import boring32.error.win32error;
import boring32.error.errorbase;
import boring32.raii.uniqueptrs;
import boring32.security.functions;
import boring32.filesystem;
import boring32.winhttp.winhttperror;

struct Test
{
	static void DoIt() { std::wcout << L"OK\n"; }
};

template<typename T>
void X()
{
	T::DoIt();
}

void SearchTokenForAdminGroup()
{
	// Open a handle to the access token for the calling process.
	Boring32::Raii::Win32Handle hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		throw Boring32::Error::Win32Error(__FUNCSIG__ ": OpenProcessToken() failed", GetLastError());

	// Create a SID for the BUILTIN\Administrators group.
	PSID rawSID = nullptr;
	SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
	const bool succeeded = AllocateAndInitializeSid(
		&SIDAuth,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&rawSID
	);
	if (!succeeded) 
		throw Boring32::Error::Win32Error(__FUNCSIG__ ": AllocateAndInitializeSid() failed", GetLastError());

	Boring32::Raii::SidUniquePtr pSID(rawSID);
	if (Boring32::Security::SearchTokenGroupsForSID(hToken, pSID.get()))
		std::wcout << "User is a member of BUILTIN\\Administrators\n";
	else
		std::wcout << "User is not a member of BUILTIN\\Administrators\n";
}

void EnumerateTokenGroups()
{
	// Open a handle to the access token for the calling process.
	Boring32::Raii::Win32Handle hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		throw Boring32::Error::Win32Error(__FUNCSIG__ ": OpenProcessToken() failed", GetLastError());
	Boring32::Security::EnumerateTokenGroups(hToken);
}

void EnumerateTokenPrivileges()
{
	// Open a handle to the access token for the calling process.
	Boring32::Raii::Win32Handle hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		throw Boring32::Error::Win32Error(__FUNCSIG__ ": OpenProcessToken() failed", GetLastError());
	Boring32::Security::EnumerateTokenPrivileges(hToken);
}

void PrintFileVersion()
{
	std::wcout
		<< Boring32::FileSystem::GetFileVersion(LR"(C:\Program Files\Notepad++\notepad++.exe)")
		<< std::endl;
}

typedef int Z;
typedef int Y;

void R(Z x) {}

int main(int argc, char** args) try
{
	Y y = 1;
	R(y);


	throw Boring32::WinHttp::WinHttpError(std::source_location::current(), "AAA", 12001);
	//Experiments1Main();
	//throw Boring32::Error::ErrorBase<std::runtime_error>(std::source_location::current(), "AAAA");
	//PrintFileVersion();
	//SearchTokenForAdminGroup();
	//EnumerateTokenGroups();
	//EnumerateTokenPrivileges();
	return 0;
}
catch (const std::exception& ex)
{
	std::wcout << ex.what() << std::endl;
	return -1;
}