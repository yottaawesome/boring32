﻿#include <format>
#include <iostream>
#include <functional>
#include <utility>
#include <vector>
#include <stdexcept>
#include <source_location>
#include <windows.h>
#include <objbase.h>
#include "Experiments1Main.hpp"

import boring32.raii;
import boring32.strings;
import boring32.error;
import boring32.raii;
import boring32.security;
import boring32.com;
import boring32.filesystem;
import boring32.winhttp;
import boring32.winsock;
import boring32.xaudio2;
import boring32.compression;
import boring32.util;
import boring32.services;
import boring32.time;
import boring32.computer;
import boring32.async;

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

struct Q
{
	virtual ~Q() {}
	Q() = default;
	Q(const Q& other) = default;
	virtual std::wstring QQ() { return L"An unknown error occurred"; }
	int m_i;
};

struct W : public virtual Q
{
	W(int i) { m_i = i; }
	W(const W& other)
	{
		m_i = other.m_i;
	}
	virtual ~W() {}
	virtual std::wstring QQ() override { return std::to_wstring(m_i); }
};

struct E : public virtual Q
{
	E(int i) { m_i = i; }
	E(const E& other) 
	{
		m_i = other.m_i;
	}
	virtual ~E() {}
	virtual std::wstring QQ() override { return std::to_wstring(m_i); }
};	

template<typename...R>
struct Error : public virtual R...
{
	virtual ~Error() {}

	//using R::R()...;
	Error() {}
	Error(const R... args) : R(args)... {}

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

template<typename...R>
struct TP
{
	TP(const R... args)
	{
		(std::make_exception_ptr(args), ...);

		m_ptrs = { (std::make_exception_ptr(args))... };
	}
	std::vector<std::exception_ptr> m_ptrs;
};

void RandomStuff()
{
	TP t(std::runtime_error("AAAA"), std::runtime_error("BBBB"));

	//SocketTest();
	//throw Boring32::Error::NtStatusError(std::source_location::current(), "Blah blah", 0x40000026);
	W w(1);
	E e(2);
	Error X(w, e);
	X.QQ();
}

void Compression()
{
	//throw Boring32::Compression::CompressionError(std::source_location::current(), "Blah");
	Boring32::Compression::Decompressor decompressor(Boring32::Compression::CompressionType::MSZIP);
	auto x = decompressor.DecompressBuffer({ std::byte(0x1), std::byte(0x2) });
}

void SocketTest()
{
	Boring32::WinSock::WinSockInit init(2, 2);
	Boring32::WinSock::Socket socket(L"142.250.70.164", 1215);
	socket.Open();
	socket.Connect();
	socket.Send({ std::byte(0x5) });
}

void Resolving()
{
	Boring32::WinSock::WinSockInit init;
	std::vector<Boring32::WinSock::NetworkingAddress> names =
		Boring32::WinSock::Resolve(L"www.google.com");
	for (const Boring32::WinSock::NetworkingAddress& x : names)
	{
		std::cout << x.Value << std::endl;
	}
}

void OtherStuff()
{
	std::wcout << Boring32::Computer::GetFormattedHostName(ComputerNameDnsFullyQualified) << std::endl;

	//Resolving();

	//SYSTEMTIME st;
	//GetSystemTime(&st);
	//std::wcout << Boring32::Time::GetTimeAsUtcString(st).c_str() << std::endl;

	//Boring32::Async::Semaphore s(false, 0, 10);
	//s.Release(0);

	//Compression();
	//SocketTest();
	//Boring32::Util::GloballyUniqueID id(L"AAAAA");

	//SC_HANDLE handle = Boring32::Services::OpenServiceControlManager(SC_MANAGER_CONNECT);
	//SC_HANDLE handle2 = Boring32::Services::OpenServiceHandle(handle, L"Bonjour Service", SERVICE_QUERY_STATUS);
	//auto x = Boring32::Services::GetServiceStatus(handle2);
}

//template<typename A, X = std::invoke_result<std::function<A>>::type>
//X Blah(const std::function<A>& func)
//{
//	return func(1);
//}

//template<typename T>
//auto Blah(const std::function<T>& func)
//{
//	return func({ 1 });
//}

struct P
{
	int operator()(int x)
	{
		return x;
	}
};

//template<typename T>
//std::function<T>::result_type Blah(const std::function<T>& func)
//{
//	return func(1);
//}

int func(int a) noexcept { return 1; }

int TestStuff() {
	auto lam = [](int a) noexcept -> int { return 1; };
	// works fine for a function with this signature: 
	static_assert(requires(int a) { { func(a) } ->std::same_as<int>; });
	// the following three conditions each evaluate to false for the lambda
	static_assert(requires(int a) {
		lam(a);
		{lam(a)}->std::same_as<int>;
			requires std::is_same_v<decltype(lam(a)), int>;
	});
	return 0;
}

int func2(int a) noexcept { return 1; }
int TestStuff2() {
	auto lam = [](int a) noexcept -> int { return 1; };
	// works fine for a function with this signature: 
	static_assert(requires(int a) { { func2(a) } ->std::same_as<int>; });
	// the following three conditions each evaluate to false for the lambda
	static_assert(
		requires(int a) 
		{
			lam(a);
			{lam(a)}->std::same_as<int>;
			requires std::is_same_v<decltype(lam(a)), int>;
		}
	);
	return 0;
}

template<typename T>
concept IsFuncAcceptsIntReturnsVoid = requires(T& func, int x)
{
	// This will also work
	//{ std::declval<T>()(x) }->std::same_as<void>;
	{ func(x) }->std::same_as<void>;
};

void templateStuff()
{
	auto m = [](int y) {};
	constexpr bool yyy = IsFuncAcceptsIntReturnsVoid<decltype(m)>;


	TestStuff();
	TestStuff2();
	std::vector<int> Y{ 1 };

	Boring32::Async::SyncedContainer<std::vector<int>> testContainer;
	testContainer.PushBack(4);
	testContainer.PushBack(5);
	//BB([](std::vector<int>::const_reference x) { std::wcout << std::format(L"{}\n", x); });
	testContainer([](std::vector<int>& x) {});
	testContainer([](int x) { std::cout << x << std::endl; });
	//BB.ForEach([](int& x) { });

	Boring32::Async::Synced<int> AA(3);
	std::wcout << std::format(L"{}\n", AA());
	HANDLE a = 0;
	Boring32::Raii::BasicHandle s;
	s = a;
}

int main(int argc, char** args) try
{
	Boring32::COM::COMThreadScope scope(COINIT::COINIT_MULTITHREADED);
	std::cout << Boring32::Util::IsConnectedToInternet();


	return 0;
}
catch (const std::exception& ex)
{
	
	Boring32::Error::PrintExceptionInfo(ex);
	return -1;
}