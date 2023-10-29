module;

// See https://learn.microsoft.com/en-us/cpp/build/walkthrough-header-units
// https://learn.microsoft.com/en-us/cpp/build/walkthrough-import-stl-header-units
#pragma warning(disable:4005)
#pragma warning(disable:5106)
// The above need to be disabled because of
// https://developercommunity.visualstudio.com/t/warning-C4005:-Outptr:-macro-redefinit/1546919
// No idea when this will get fixed, MS seems to be taking their time with it.
// Including SDKDDKVer.h defines the highest available Windows platform.
// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
#include <SDKDDKVer.h>
#include <Windows.h>
#include <Msi.h>
#include <MsiQuery.h>
#include <memoryapi.h>
#include <Comdef.h>
#include <sddl.h>
#include <wlanapi.h> // unfortunately, not importable
#include <wtypes.h>
#include <guiddef.h>
#include <pathcch.h> // unfortunately, not importable
#include <Objbase.h>
#include <netlistmgr.h>
#include <Rpc.h>
#include <shlwapi.h>
#include <compressapi.h> //unfortunately, not importable
#include <psapi.h> // module file mapping is invalid.
#include <winsock2.h>
#include <iphlpapi.h> // module file mapping is invalid.
#include <iptypes.h> // module file mapping is invalid.
#include <Ntsecapi.h>
#include <taskschd.h>
#include <wrl/client.h>
#include <TlHelp32.h>
#include <Jobapi2.h>
#include <threadpoolapiset.h>
#include <process.h>
#include <bcrypt.h>
#include <dpapi.h>
#include <wincrypt.h>
#include <cryptuiapi.h>
#include <winnt.h>
#include <wincodec.h>
#include <xaudio2.h>
#include <winhttp.h>
#include <ws2tcpip.h>
// See https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-sch_credentials
#define SCHANNEL_USE_BLACKLISTS
#include <schannel.h>
#include <Schnlsp.h>
// Must be defined for security.h
// see https://stackoverflow.com/questions/11561475/sspi-header-file-fatal-error
#define SECURITY_WIN32
#include <security.h>

export module boring32.win32;

// Experimental module intended to decouple the codebase from having to import Windows headers.
export namespace Boring32::Win32
{
	using ::HMODULE;
	using ::SECURITY_ATTRIBUTES;
	using ::DWORD;
	using ::UINT;
	using ::USHORT;
	using ::HANDLE;
	using ::PHANDLE;
	using ::UNICODE_STRING;
	using ::PUNICODE_STRING;
	using ::PVOID;
	using ::ULONG_PTR;
	using ::SIZE_T;
	using ::PSIZE_T;
	using ::PLARGE_INTEGER;
	using ::ULONG;
	using ::PHANDLE;
	using ::ACCESS_MASK;
	using ::PULONG;
	using ::HMODULE;
	using ::NTSTATUS;
	using ::BOOL;
	using ::MSIHANDLE;
	using ::MSIINSTALLCONTEXT;
	using ::PMSIHANDLE;
	using ::PSID;
	using ::HRESULT;
	using ::SLIST_ENTRY;
	using ::PSLIST_ENTRY;
	using ::SLIST_HEADER;
	using ::PSLIST_HEADER;
	using ::MEMORYSTATUSEX;
	using ::SYSTEM_INFO;
	using ::SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX;
	using ::LOGICAL_PROCESSOR_RELATIONSHIP;
	using ::COMPUTER_NAME_FORMAT;
	using ::FILETIME;
	using ::ULARGE_INTEGER;
	using ::LPCSTR;
	using ::COINIT;
	using ::EOLE_AUTHENTICATION_CAPABILITIES;
	using ::CRITICAL_SECTION;

	inline HRESULT MakeHResult(const long severity, const long facility, const long code) noexcept
	{
		return MAKE_HRESULT(severity, facility, code);
	}

	inline long Facility(const HRESULT hr) noexcept
	{
		return HRESULT_FACILITY(hr);
	}

	inline long Code(const HRESULT hr) noexcept
	{
		return HRESULT_CODE(hr);
	}

	inline long Severity(const HRESULT hr) noexcept
	{
		return HRESULT_SEVERITY(hr);
	}

	template<typename T = LPCTSTR>
	inline consteval T MsiDbOpen_CreateDirect() noexcept { return (T)MSIDBOPEN_CREATEDIRECT; }

	template<typename T = LPCTSTR>
	inline consteval T MsiDbOpen_Create() noexcept { return (T)MSIDBOPEN_CREATE; }

	template<typename T = LPCTSTR>
	inline consteval T MsiDbOpen_Direct() noexcept { return (T)MSIDBOPEN_DIRECT; }
		
	template<typename T = LPCTSTR>
	inline consteval T MsiDbOpen_ReadOnly() noexcept { return (T)MSIDBOPEN_READONLY; }
		
	template<typename T = LPCTSTR>
	inline consteval T MsiDbOpen_Transact() noexcept { return (T)MSIDBOPEN_TRANSACT; }
		
	constexpr unsigned long long MsiDbOpen_PatchFile = MSIDBOPEN_PATCHFILE;

	constexpr const wchar_t* InstallProperty_ProductName = INSTALLPROPERTY_PRODUCTNAME;
	constexpr const wchar_t* InstallProperty_PackageName = INSTALLPROPERTY_PACKAGENAME;

	const HANDLE InvalidHandleValue = INVALID_HANDLE_VALUE;

	constexpr unsigned long GenericRead = GENERIC_READ;
	constexpr unsigned long GenericWrite = GENERIC_WRITE;
	constexpr unsigned long OpenAlways = OPEN_ALWAYS;
	constexpr unsigned long FileAttributeNormal = FILE_ATTRIBUTE_NORMAL;
	constexpr auto DuplicateSameAccess = DUPLICATE_SAME_ACCESS;

	constexpr auto FormatMessageAllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER;
	constexpr auto FormatMessageFromSystem = FORMAT_MESSAGE_FROM_SYSTEM;
	constexpr auto FormatMessageIgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS;
	constexpr auto FormatMessageFromHModule = FORMAT_MESSAGE_FROM_HMODULE;

	constexpr int HandleFlagInherit = HANDLE_FLAG_INHERIT;

	constexpr auto MemoryAllocationAlignment = MEMORY_ALLOCATION_ALIGNMENT;

	constexpr auto ProcessQueryInformation = PROCESS_QUERY_INFORMATION;

	constexpr auto CpUtf8 = CP_UTF8;
	constexpr auto WcNoBestFitChars = WC_NO_BEST_FIT_CHARS;

	namespace ErrorCodes
	{
		constexpr auto Success = ERROR_SUCCESS;
		constexpr auto BufferOverflow = ERROR_BUFFER_OVERFLOW;
		constexpr auto UnknownProperty = ERROR_UNKNOWN_PROPERTY;
		constexpr auto NoMoreItems = ERROR_NO_MORE_ITEMS;
		constexpr auto MoreData = ERROR_MORE_DATA;
		constexpr auto InsufficientBuffer = ERROR_INSUFFICIENT_BUFFER;
	}

	enum class RPCCAuthLevel : unsigned long
	{
		Default = RPC_C_AUTHN_LEVEL_DEFAULT,
		None = RPC_C_AUTHN_LEVEL_NONE,
		Connect = RPC_C_AUTHN_LEVEL_CONNECT,
		Call = RPC_C_AUTHN_LEVEL_CALL,
		Pkt = RPC_C_AUTHN_LEVEL_PKT,
		PktIntegrity = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY,
		PktPrivacy = RPC_C_AUTHN_LEVEL_PKT_PRIVACY
	};

	enum class RPCCImpLevel : unsigned long
	{
		Default = RPC_C_IMP_LEVEL_DEFAULT,
		Anonymous = RPC_C_IMP_LEVEL_ANONYMOUS,
		Identify = RPC_C_IMP_LEVEL_IDENTIFY,
		Impersonate = RPC_C_IMP_LEVEL_IMPERSONATE,
		Delegate = RPC_C_IMP_LEVEL_DELEGATE
	};

	using ::K32EnumDeviceDrivers;
	using ::K32EnumProcesses;
	using ::GetLastError;
	using ::GetProcAddress;
	using ::GetModuleHandleW;
	using ::DuplicateHandle;
	using ::GetCurrentProcess;
	using ::GetHandleInformation;
	using ::SetHandleInformation;
	using ::GetProcAddress;
	using ::FormatMessageA;
	using ::FormatMessageW;
	using ::LoadLibraryW;
	using ::LoadLibraryExW;
	using ::FreeLibrary;
	using ::MsiIsProductElevatedW;
	using ::MsiOpenPackageW;
	using ::MsiCloseHandle;
	using ::MsiGetProductInfoExW;
	using ::MsiOpenDatabaseW;
	using ::MsiDatabaseOpenViewW;
	using ::MsiViewExecute;
	using ::MsiViewFetch;
	using ::MsiRecordGetStringW;
	using ::MsiEnumProductsExW;
	using ::CreateFileW;
	using ::CloseHandle;
	using ::LocalFree;
	using ::FreeSid;
	using ::InitializeSListHead;
	using ::QueryDepthSList;
	using ::InterlockedPopEntrySList;
	using ::InterlockedPushEntrySList;
	using ::InterlockedFlushSList;
	using ::GetComputerNameExW;
	using ::GetPhysicallyInstalledSystemMemory;
	using ::GlobalMemoryStatusEx;
	using ::GetTickCount64;
	using ::GetSystemTimeAdjustment;
	using ::GetSystemInfo;
	using ::GetLogicalProcessorInformationEx;
	using ::GetProcessTimes;
	using ::GetProcessHandleCount;
	using ::GetExitCodeProcess;
	using ::K32GetModuleFileNameExW;
	using ::OpenProcess;
	using ::GetProcessId;
	using ::WideCharToMultiByte;
	using ::MultiByteToWideChar;
	using ::CoUninitialize;
	using ::GetCurrentThreadId;
	using ::CoInitializeSecurity;
	using ::CoInitializeEx;
	using ::InitializeCriticalSectionEx;
	using ::InitializeCriticalSectionAndSpinCount;
	using ::DeleteCriticalSection;

	using ::IP_ADAPTER_ADDRESSES;
	using ::GetAdaptersAddresses;
}
