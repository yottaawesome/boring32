﻿import std;
////import <win32.hpp>;
import boring32;
import boring32.win32;
//#include "versionhelpers.h"
//#include "Experiments1Main.hpp"
//
import boring32;

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
	Boring32::RAII::Win32Handle hToken;
	if (not Boring32::Win32::OpenProcessToken(Boring32::Win32::GetCurrentProcess(), Boring32::Win32::_TOKEN_QUERY, &hToken))
		throw Boring32::Error::Win32Error(Boring32::Win32::GetLastError(), "OpenProcessToken() failed");

	// Create a SID for the BUILTIN\Administrators group.
	Boring32::Win32::PSID rawSID = nullptr;
	Boring32::Win32::SID_IDENTIFIER_AUTHORITY SIDAuth = Boring32::Win32::_SECURITY_NT_AUTHORITY;
	const bool succeeded = Boring32::Win32::AllocateAndInitializeSid(
		&SIDAuth,
		2,
		Boring32::Win32::_SECURITY_BUILTIN_DOMAIN_RID,
		Boring32::Win32::_DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&rawSID
	);
	if (not succeeded) 
		throw Boring32::Error::Win32Error(Boring32::Win32::GetLastError(), "AllocateAndInitializeSid() failed");

	Boring32::RAII::SIDUniquePtr pSID(rawSID);
	if (Boring32::Security::SearchTokenGroupsForSID(hToken, pSID.get()))
		std::wcout << "User is a member of BUILTIN\\Administrators\n";
	else
		std::wcout << "User is not a member of BUILTIN\\Administrators\n";
}

void EnumerateTokenGroups()
{
	// Open a handle to the access token for the calling process.
	Boring32::RAII::Win32Handle hToken;
	if (not OpenProcessToken(GetCurrentProcess(), Boring32::Win32::_TOKEN_QUERY, &hToken))
		throw Boring32::Error::Win32Error(GetLastError(), "OpenProcessToken() failed");
	Boring32::Security::EnumerateTokenGroups(hToken);
}

void EnumerateTokenPrivileges()
{
	// Open a handle to the access token for the calling process.
	Boring32::RAII::Win32Handle hToken;
	if (not Boring32::Win32::OpenProcessToken(GetCurrentProcess(), Boring32::Win32::_TOKEN_QUERY, &hToken))
		throw Boring32::Error::Win32Error(GetLastError(), "OpenProcessToken() failed");
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
		auto x = (std::make_exception_ptr(args), ...);

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
	Boring32::Compression::Decompressor decompressor(Boring32::Win32::CompressionType::MSZIP);
	auto x = decompressor.DecompressBuffer({ std::byte(0x1), std::byte(0x2) });
}

void SocketTest()
{
	Boring32::WinSock::WinSockInit init(2, 2);
	Boring32::WinSock::TCPSocket socket(L"142.250.70.164", 1215);
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
	Boring32::WirelessLAN::Session wlanSession;

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
	Boring32::Win32::HANDLE a = 0;
	Boring32::RAII::BasicHandle s;
	s = a;
}
//
//void BlahBlah()
//{
//	const auto procInfos = Boring32::Computer::ProcessInfo::FromCurrentProcesses();
//	for (const auto& proc : procInfos)
//	{
//		std::wcout << std::format(L"Process path: {}\n", proc.GetPath());
//	}
//	return;
//
//	Boring32::WirelessLAN::Session s;
//	auto x = s.Interfaces.GetAll();
//	std::cout << (int)x.at(0).GetState() << std::endl;
//}
//
//struct O
//{
//	int P = 5;
//};
//
//enum class Mask : int
//{
//	ServicePackMinor = 1,
//	ServicePackMajor = 2,
//	Build = 4,
//	Minor = 8,
//	Major = 16
//};
////DEFINE_ENUM_FLAG_OPERATORS(Mask)
//bool operator&(const int i, const Mask m)
//{
//	return i & static_cast<int>(m);
//}
//bool operator&(const Mask m, const int i)
//{
//	return i & static_cast<int>(m);
//}
//int operator|(const Mask a, const Mask b)
//{
//	return static_cast<int>(a) | static_cast<int>(b);
//}
//int operator|(const int a, const Mask b)
//{
//	return static_cast<int>(a) | static_cast<int>(b);
//}
//
//// This requires a manifest file in the binary that 
//// specifies Win10 version support, or it will always 
//// fail on Win10 checks.
//bool MeetsMinimumOSVersion(
//	const unsigned major,
//	const unsigned minor,
//	const unsigned build,
//	const unsigned short spMajor,
//	const unsigned short spMinor,
//	unsigned fieldMask
//)
//{
//	// According to https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-verifyversioninfow
//	// you also need to test the Minor and ServicePack* fields when testing Major version.
//	if (fieldMask & Mask::Major)
//	{
//		fieldMask |= static_cast<int>(Mask::Minor);
//		fieldMask |= static_cast<int>(Mask::ServicePackMajor);
//		fieldMask |= static_cast<int>(Mask::ServicePackMinor);
//	}
//
//	DWORDLONG condition = 0;
//	DWORD flags = 0;
//	if (fieldMask & Mask::Major)
//	{
//		condition = VerSetConditionMask(
//			condition,
//			VER_MAJORVERSION,
//			VER_GREATER_EQUAL
//		);
//		flags |= VER_MAJORVERSION;
//	}
//	if (fieldMask & Mask::Minor)
//	{
//		condition = VerSetConditionMask(
//			condition,
//			VER_MINORVERSION,
//			VER_GREATER_EQUAL
//		);
//		flags |= VER_MINORVERSION;
//	}
//	if (fieldMask & Mask::Build)
//	{
//		condition = VerSetConditionMask(
//			condition,
//			VER_BUILDNUMBER,
//			VER_GREATER_EQUAL
//		);
//		flags |= VER_BUILDNUMBER;
//	}
//	if (fieldMask & Mask::ServicePackMinor)
//	{
//		condition = VerSetConditionMask(
//			condition,
//			VER_SERVICEPACKMINOR,
//			VER_GREATER_EQUAL
//		);
//		flags |= VER_SERVICEPACKMINOR;
//	}
//	if (fieldMask & Mask::ServicePackMajor)
//	{
//		condition = VerSetConditionMask(
//			condition,
//			VER_SERVICEPACKMAJOR,
//			VER_GREATER_EQUAL
//		);
//		flags |= VER_SERVICEPACKMAJOR;
//	}
//
//	OSVERSIONINFOEXW versionInfo
//	{
//		.dwOSVersionInfoSize = sizeof(versionInfo),
//		.dwMajorVersion = (fieldMask & Mask::Major) ? major : 0,
//		.dwMinorVersion = (fieldMask & Mask::Minor) ? minor : 0,
//		.dwBuildNumber = (fieldMask & Mask::Build) ? build : 0,
//		.wServicePackMajor = (fieldMask & Mask::ServicePackMajor) ? spMajor : 0ui16,
//		.wServicePackMinor = (fieldMask & Mask::ServicePackMinor) ? spMinor : 0ui16
//	};
//	// This function is deprecated as of Win10
//	const bool succeeded = VerifyVersionInfoW(
//		&versionInfo,
//		flags,
//		condition
//	);
//	const auto lastError = GetLastError();
//	if (not succeeded && lastError != ERROR_OLD_WIN_VERSION)
//		throw Boring32::Error::Win32Error("VerifyVersionInfoW() failed", lastError);
//
//	return succeeded;
//}
//
//BOOL Is_WinXP_SP2_or_Later()
//{
//	OSVERSIONINFOEX osvi;
//	DWORDLONG dwlConditionMask = 0;
//	int op = VER_GREATER_EQUAL;
//
//	// Initialize the OSVERSIONINFOEX structure.
//
//	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
//	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
//	osvi.dwMajorVersion = 10;
//	osvi.dwMinorVersion = 0;
//	osvi.wServicePackMajor = 0;
//	osvi.wServicePackMinor = 0;
//
//	// Initialize the condition mask.
//
//	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
//	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
//	VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, op);
//	VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMINOR, op);
//
//	// Perform the test.
//
//	return VerifyVersionInfo(
//		&osvi,
//		VER_MAJORVERSION | VER_MINORVERSION |
//		VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
//		dwlConditionMask);
//}
//
//namespace x
//{
//	struct blah {};
//}
//
//void Blah(int [[x::blah]]blah)
//{
//
//}
//
//void RandomCrap()
//{
//	//std::wcout << MeetsVersion(10, 0, 1809, Mask::Major | Mask::Build) << std::endl;
//
//	OSVERSIONINFOEXW vi
//	{
//		.dwOSVersionInfoSize = sizeof(vi),
//		.dwMajorVersion = 10,
//		.dwMinorVersion = 0,
//		.dwBuildNumber = 1809,
//		.wServicePackMajor = 0,
//		.wServicePackMinor = 0
//	};
//	DWORDLONG condition = 0;
//	condition = VerSetConditionMask(
//		condition,
//		VER_MAJORVERSION,
//		VER_GREATER_EQUAL
//	);
//	condition = VerSetConditionMask(
//		condition,
//		VER_MINORVERSION,
//		VER_GREATER_EQUAL
//	);
//	condition = VerSetConditionMask(
//		condition,
//		VER_BUILDNUMBER,
//		VER_GREATER_EQUAL
//	);
//	condition = VerSetConditionMask(
//		condition,
//		VER_SERVICEPACKMAJOR,
//		VER_GREATER_EQUAL
//	);
//	condition = VerSetConditionMask(
//		condition,
//		VER_SERVICEPACKMINOR,
//		VER_GREATER_EQUAL
//	);
//	bool succeeded = VerifyVersionInfoW(
//		&vi,
//		VER_MAJORVERSION
//		| VER_MINORVERSION
//		| VER_SERVICEPACKMAJOR
//		| VER_SERVICEPACKMINOR
//		| VER_BUILDNUMBER
//		, condition
//	);
//	const auto lastError = GetLastError();
//	if (not succeeded && lastError != ERROR_OLD_WIN_VERSION)
//	{
//		throw Boring32::Error::Win32Error(__FUNCSIG__": VerifyVersionInfoW()", lastError);
//	}
//
//	IsWindowsVersionOrGreater(10, 0, 1809);
//
//	std::wcout
//		<< std::format(
//			L"{}:{}\n",
//			MeetsMinimumOSVersion(
//				10,
//				0,
//				1809,
//				0,
//				0,
//				Mask::Build | Mask::Major
//			),
//			IsWindows10OrGreater());
//
//	/*OSVERSIONINFOEX v{
//		.dwOSVersionInfoSize = sizeof(v)
//	};
//	GetVersionExW(
//		&v,
//		VER_MAJORVERSION | VER_BUILDNUMBER,
//	);
//	std::wcout << std::format(
//		L"{}.{}.{}\n",
//		v.dwMajorVersion,
//		v.dwMinorVersion,
//		v.dwBuildNumber
//	);*/
//	//GetProductInfo();
//}
//
//struct message_and_location
//{
//	std::string_view message;
//	std::source_location loc;
//
//	template<typename T>
//	message_and_location(T&& msg, std::source_location loc = std::source_location::current())
//		: message{ std::forward<T>(msg) }
//		, loc{ loc }
//	{}
//};
//
//template<typename... Args>
//void foo(message_and_location ml, Args&&... args) {}
//
//template<typename...T>
//struct OP
//{
//	OP(const T&... t){}
//};
//
//template<typename...Args>
////void ArgsTest(Args... a)
//void ArgsTest(message_and_location m1, Args...args)
//{
//	//OP b(a...);
//	std::wcout << m1.loc.line() << std::endl;
//}
//
//template<int N>
//struct Number
//{
//	char blah[N];
//};
//
//template<bool N>
//struct Throw
//{
//
//};
//
//template<>
//struct Throw<false>
//{
//	using Type = bool;
//};
//
//template<>
//struct Throw<true>
//{
//	using Type = void;
//};
//
//template<bool THROW>
//auto TestOptionalThrow() noexcept(not THROW)
//{
//	bool succeeded = true; // result of some native api call
//
//	if constexpr (not THROW)
//		return succeeded;		
//	else if (not succeeded)
//		throw std::runtime_error("Some error");
//}
//
//enum class Lum
//{
//	One,
//	Two
//};
//
//template<Lum VAL>
//void Run(Lum v = VAL) 
//{
//
//}
//
//template<>
//void Run<Lum::One>(Lum v) {}
//template<>
//void Run<Lum::Two>(Lum v) {}
//
//
//enum class SomeEnum : unsigned long
//{
//	//SentinelMin = 0,
//	ValidValue = 1,
//	SentinelMax
//};
//
////constexpr void Check(int v = 0)
////{
////	static_assert(v < 1);
////}
//
//void DontDoThis()
//{
//	// https://stackoverflow.com/questions/55790420/is-string-view-really-promoting-use-after-free-errors
//	std::string s = "Hellooooooooooooooo ";
//	std::string_view sv = s + "World\n";
//	std::cout << sv;
//}
//
//template<Boring32::Strings::FixedString TName, Boring32::Registry::ValueTypes TType, auto DefaultGetter = [] {} >
//using RegValue = Boring32::Registry::RegistryValue<
//	HKEY_LOCAL_MACHINE,
//	LR"(SOFTWARE\Microsoft\OneDrive)",
//	TName,
//	TType,
//	DefaultGetter
//>;
//
//int main(int argc, char** args) try
//{
//	/*constexpr int i = 1;
//	OPP<2> p;
//	p();*/
//
//	constexpr wchar_t iol[] = L"UpdateBeginTimestampTryCountODSU";
//
//	using Odsu = RegValue<
//		//L"UpdateBeginTimestampTryCountODSU",
//		iol,
//		Boring32::Registry::ValueTypes::DWord,
//		[] { return 1; }
//	>;
//
//	using Odsu2 = RegValue<L"UpdateBeginTimestampTryCountODSU", Boring32::Registry::ValueTypes::DWord>;
//
//	using CurrentVersionPath = Boring32::Registry::RegistryValue <
//		HKEY_LOCAL_MACHINE,
//		LR"(SOFTWARE\Microsoft\OneDrive)",
//		L"CurrentVersionPath",
//		Boring32::Registry::ValueTypes::String,
//		[] { return L""; }
//	> ;
//
//	std::wcout << std::format(L"The registry value {}\\{} is {}\n", Odsu::GetSubKey(), Odsu::GetValueName(), Odsu::Read());
//	std::wcout << std::format(L"The registry value {}\\{} is {}\n", CurrentVersionPath::GetSubKey(), CurrentVersionPath::GetValueName(), CurrentVersionPath::Read());
//
//
//	Boring32::Async::SlimRWProtectedObject<int> g(3);
//	g.Mutate([](const int& x) {});
//
//	Boring32::Memory::Heap h(0, 0);
//	auto ptr = h.NewPtr<int>(10);
//	ptr.reset();
//
//	Boring32::Util::Enum<SomeEnum, SomeEnum::ValidValue> s;
//	SomeEnum def = s.Default();
//	bool u = s.IsValid(1);
//	const int k = 0;
//	//Check(k);
//
//	Number<5> n;
//	TestOptionalThrow<true>();
//	TestOptionalThrow<false>();
//
//	//OP i(1,1);
//	/*ArgsTest("HAHAHA what", 7);
//	throw Boring32::Error::Boring32Error("A {}", std::source_location::current(), 1);	*/
//	Boring32::SSPI::Credential c;
//}
//catch (const std::exception& ex)
//{
//	Boring32::Error::PrintExceptionInfo(ex);
//	return -1;
//}

namespace TestRange
{
	struct XA
	{
		int Value = 0;
		XA(){};
		XA(int x) : Value(x) {}

		auto operator<=>(const XA&) const = default;

		XA operator+(const XA& other) noexcept
		{
			return Value + other.Value;
		}

		XA operator++(int) noexcept // pre-increment takes no argument
		{
			return XA{ Value++ };
		}

		XA& operator+=(const XA& other) noexcept
		{
			Value += other.Value;
			return *this;
		}
	};

	template<typename T>
	concept Increment = requires(T t)
	{
		t++;
	};

	void Run()
	{
		Boring32::Util::Range<int> blahA(5, 10);

		XA a(5);
		XA b(10);
		constexpr bool bb = Increment<XA>;
		Boring32::Util::Range<XA> blahB(a, b);
		blahB.Next(std::nothrow);
	}
}

template<typename T>
concept b = requires(T t)
{
	t == true;
};

struct LL
{
	operator bool() { return true; }
};

b auto F(bool B)
{
	if (B)
		return true;
	return false;
}

template<typename T>
concept SomeC = requires(T t)
{
	requires noexcept(t.What());
};

//template<typename T>
//concept Formattable = std::convertible_to<T, Boring32::Strings::FixedStringN> or std::is_integral_v<T, int>;

//template<Boring32::Strings::FixedStringN TFmt, typename...TArgs>
//struct GG
//{
//	GG(TArgs&&...args)
//		: S(std::vformat(TFmt.ToView(), std::make_format_args(std::forward<TArgs>(args)...)))
//	{ }
//
//	const std::string& What() const noexcept
//	{
//		return S;
//	}
//
//	std::string S;
//};

//using CC = GG<"Haha {} {}", std::string_view, std::string_view, int>;
//using KK = GG<"Haha {} {}", std::string_view, std::string_view, int>;

struct FF : public std::runtime_error
{
	const char* GHJ = "blah";
	FF(SomeC auto&& s)
		: std::runtime_error(s.What())
	{ }
};

int main()
{
	try
	{
		//OO("A", "B", 1);
		//throw FF(KK{ "A", "B", 1 });
	}
	catch (const FF& e)
	{
		std::cout << e.what() << std::endl;
	}


	const char* x = "{}";
	const Boring32::Error::MessageLocationTrace e{ x };
	Boring32::Strings::SafeVFormat < [](const std::exception&) noexcept -> std::string { return {}; } > (e.Message, "a");

	F(true);

	auto p = Boring32::Shell::GetKnownFolderPath(
		Boring32::Win32::Shell::WellKnownFolders::FOLDERID_LocalAppData
	);

	TestRange::Run();
	return 0;
}