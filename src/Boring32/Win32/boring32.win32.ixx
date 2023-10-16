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
	template<typename T = LPCTSTR>
	inline constexpr T MsiDbOpen_CreateDirect() noexcept { return (T)MSIDBOPEN_CREATEDIRECT; }

	template<typename T = LPCTSTR>
	inline constexpr T MsiDbOpen_Create() noexcept { return (T)MSIDBOPEN_CREATE; }

	template<typename T = LPCTSTR>
	inline constexpr T MsiDbOpen_Direct() noexcept { return (T)MSIDBOPEN_DIRECT; }
		
	template<typename T = LPCTSTR>
	inline constexpr T MsiDbOpen_ReadOnly() noexcept { return (T)MSIDBOPEN_READONLY; }
		
	template<typename T = LPCTSTR>
	inline constexpr T MsiDbOpen_Transact() noexcept { return (T)MSIDBOPEN_TRANSACT; }
		
	constexpr unsigned long long MsiDbOpen_PatchFile = MSIDBOPEN_PATCHFILE;

	constexpr const wchar_t* InstallProperty_ProductName = INSTALLPROPERTY_PRODUCTNAME;
	constexpr const wchar_t* InstallProperty_PackageName = INSTALLPROPERTY_PACKAGENAME;

	namespace ErrorCodes
	{
		constexpr auto Success = ERROR_SUCCESS;
		constexpr auto BufferOverflow = ERROR_BUFFER_OVERFLOW;
		constexpr auto UnknownProperty = ERROR_UNKNOWN_PROPERTY;
		constexpr auto NoMoreItems = ERROR_NO_MORE_ITEMS;
		constexpr auto MoreData = ERROR_MORE_DATA;
	}

	using ::DWORD;
	using ::UINT;
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

	using ::GetLastError;
	using ::GetProcAddress;
	using ::GetModuleHandleW;
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

	using ::IP_ADAPTER_ADDRESSES;
	using ::GetAdaptersAddresses;
}
