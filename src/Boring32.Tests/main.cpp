#include <format>
#include <iostream>
#include <stdexcept>
#include <source_location>
#include <windows.h>
#include "Experiments1Main.hpp"

import boring32.raii.win32handle;
import boring32.strings;
import boring32.error;
import boring32.raii.uniqueptrs;
import boring32.security.functions;
import boring32.filesystem;
import boring32.winhttp.winhttperror;
import boring32.winsock;
import boring32.xaudio2;

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
		throw Boring32::Error::Win32Error(std::source_location::current(), "OpenProcessToken() failed", GetLastError());

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
		throw Boring32::Error::Win32Error(std::source_location::current(), "AllocateAndInitializeSid() failed", GetLastError());

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
		throw Boring32::Error::Win32Error(std::source_location::current(), "OpenProcessToken() failed", GetLastError());
	Boring32::Security::EnumerateTokenGroups(hToken);
}

void EnumerateTokenPrivileges()
{
	// Open a handle to the access token for the calling process.
	Boring32::Raii::Win32Handle hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		throw Boring32::Error::Win32Error(std::source_location::current(), "OpenProcessToken() failed", GetLastError());
	Boring32::Security::EnumerateTokenPrivileges(hToken);
}

void PrintFileVersion()
{
	std::wcout
		<< Boring32::FileSystem::GetFileVersion(LR"(C:\Program Files\Notepad++\notepad++.exe)")
		<< std::endl;
}

void SocketTest()
{
	Boring32::WinSock::WinSockInit init(2,2);
	Boring32::WinSock::Socket socket(L"www.google.com", 80);
	socket.Connect();
}

struct Q
{
	virtual ~Q() {}
	virtual std::wstring QQ() { return L"An unknown error occurred"; }
};

struct W : public virtual Q
{
	virtual ~W() {}
	virtual std::wstring QQ() override { return L"A COM error occurred"; }
};

struct E : public virtual Q
{
	virtual ~E() {}
	virtual std::wstring QQ() override { return L"An XAudio2 error occurred"; }
};

template<typename...R>
struct Error : public virtual R...
{
	virtual ~Error() {}
	virtual void Blah(const std::wstring& str) { std::wcout << str << std::endl; }
	virtual std::wstring QQ() override 
	{ 
		// https://stackoverflow.com/questions/43322854/multiple-inheritance-with-variadic-templates-how-to-call-function-for-each-base
		// Call QQ() on each base class...
		//(R::QQ(), ...);
		// Forward the results of calling QQ on each base class to a vector
		//std::vector strings = { (R::QQ())... };
		// Call Blah() once for each instance of QQ()
		(Blah(R::QQ()), ...);
		return L"";
	}
};


int main(int argc, char** args) try
{
	//SocketTest();
	//throw Boring32::Error::NtStatusError(std::source_location::current(), "Blah blah", 0x40000026);
	Error<W, E> X;
	X.QQ();
	return 0;
}
catch (const std::exception& ex)
{
	//print_exception_info2(ex);
	std::wcout << ex.what() << std::endl;
	return -1;
}