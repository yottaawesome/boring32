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
	using ::LARGE_INTEGER;
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
	using ::PROCESSENTRY32W;
	using ::PTIMERAPCROUTINE;
	using ::JOBOBJECT_EXTENDED_LIMIT_INFORMATION;
	using ::JOBOBJECTINFOCLASS;
	using ::SRWLOCK;
	using ::PROCESS_INFORMATION;
	using ::PAPCFUNC;
	using ::STARTUPINFO;
	using ::SYNCHRONIZATION_BARRIER;
	using ::WAITORTIMERCALLBACK;
	using ::PTP_CALLBACK_INSTANCE;
	using ::PTP_WORK;
	using ::TP_POOL;
	using ::TP_CALLBACK_ENVIRON;
	using ::OVERLAPPED;
	using ::VS_FIXEDFILEINFO;
	using ::COMPRESSOR_HANDLE;
	using ::DECOMPRESSOR_HANDLE;
	using ::BCRYPT_KEY_HANDLE;

	enum class CompressionType : DWORD
	{
		NotSet = 0,
		MSZIP = COMPRESSION_FORMAT_LZNT1,
		XPRESS = COMPRESSION_FORMAT_XPRESS,
		XPRESSHuffman = COMPRESSION_FORMAT_XPRESS_HUFF,
		LZMS = COMPRESSION_FORMAT_XP10
	};

	namespace MemoryProtection
	{
		constexpr auto PageReadWrite = PAGE_READWRITE;
	}

	enum class FileMapAccess : unsigned long
	{
		All = FILE_MAP_ALL_ACCESS,
		Execute = FILE_MAP_EXECUTE,
		Read = FILE_MAP_READ,
		Write = FILE_MAP_WRITE
	};

	inline ::DWORD LowDWord(auto _qw) noexcept
	{
		return LODWORD(_qw);
	}

	inline ::DWORD HighDWord(auto _qw) noexcept
	{
		return HIDWORD(_qw);
	}

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

	inline bool HrFailed(const HRESULT hr) noexcept
	{
		return FAILED(hr);
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

	constexpr auto MsiInstallContextMachine = MSIINSTALLCONTEXT_MACHINE;

	const HANDLE InvalidHandleValue = INVALID_HANDLE_VALUE;

	constexpr auto SddlRevision1 = SDDL_REVISION_1;

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

	constexpr auto Infinite = INFINITE;

	constexpr auto CryptProtectMemoryBlockSize = CRYPTPROTECTMEMORY_BLOCK_SIZE;

	namespace BCryptChainingMode
	{
		constexpr auto CipherBlockChaining = BCRYPT_CHAIN_MODE_CBC;
		constexpr auto CbcMac = BCRYPT_CHAIN_MODE_CCM;
		constexpr auto CipherFeedback = BCRYPT_CHAIN_MODE_CFB;
		constexpr auto ElectronicCodebook = BCRYPT_CHAIN_MODE_ECB;
		constexpr auto GaloisCounterMode = BCRYPT_CHAIN_MODE_GCM;
	}

	enum class EncryptionType : DWORD
	{
		SameProcess = CRYPTPROTECTMEMORY_SAME_PROCESS,
		CrossProcess = CRYPTPROTECTMEMORY_CROSS_PROCESS,
		SameLogon = CRYPTPROTECTMEMORY_SAME_LOGON
	};

	namespace ErrorCodes
	{
		constexpr auto NoError = NOERROR;
		constexpr auto Success = ERROR_SUCCESS;
		constexpr auto BufferOverflow = ERROR_BUFFER_OVERFLOW;
		constexpr auto UnknownProperty = ERROR_UNKNOWN_PROPERTY;
		constexpr auto NoMoreItems = ERROR_NO_MORE_ITEMS;
		constexpr auto MoreData = ERROR_MORE_DATA;
		constexpr auto InsufficientBuffer = ERROR_INSUFFICIENT_BUFFER;
		constexpr auto Timeout = ERROR_TIMEOUT;
		constexpr auto IoPending = ERROR_IO_PENDING;
		constexpr auto AbandonedWait0 = ERROR_ABANDONED_WAIT_0;
	}

	namespace NTStatus // winnt.h
	{
		constexpr auto Pending = STATUS_PENDING;
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

	constexpr auto Th32csInherit = TH32CS_INHERIT;
	constexpr auto Th32csSnapAll = TH32CS_SNAPALL;
	constexpr auto Th32csSnapHeapList = TH32CS_SNAPHEAPLIST;
	constexpr auto Th32csSnapModule = TH32CS_SNAPMODULE;
	constexpr auto Th32csSnapModule32 = TH32CS_SNAPMODULE32;
	constexpr auto Th32csSnapProcess = TH32CS_SNAPPROCESS;
	constexpr auto Th32csSnapThread = TH32CS_SNAPTHREAD;

	constexpr auto MaximumWaitObjects = MAXIMUM_WAIT_OBJECTS;
	constexpr auto WaitObject0 = WAIT_OBJECT_0;
	constexpr auto WaitFailed = WAIT_FAILED;
	constexpr auto WaitSuccess = WAIT_OBJECT_0;
	constexpr auto WaitTimeout = WAIT_TIMEOUT;
	constexpr auto WaitAbandoned = WAIT_ABANDONED;
	constexpr auto WaitIoCompletion = WAIT_IO_COMPLETION;

	enum class WaitResult : unsigned long
	{
		Failed = WAIT_FAILED,
		Success = WAIT_OBJECT_0,
		Timeout = WAIT_TIMEOUT,
		Abandoned = WAIT_ABANDONED,
		IoCompletion = WAIT_IO_COMPLETION
	};

	constexpr auto JobObjectAllAccess = JOB_OBJECT_ALL_ACCESS;

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
	using ::LeaveCriticalSection;
	using ::EnterCriticalSection;
	using ::WaitForSingleObjectEx;
	using ::WaitForSingleObject;
	using ::WaitForMultipleObjectsEx;
	using ::WaitForMultipleObjects;
	using ::OpenEventW;
	using ::CreateEventW;
	using ::SetEvent;
	using ::ResetEvent;
	using ::CreateWaitableTimerW;
	using ::SetWaitableTimer;
	using ::CreateToolhelp32Snapshot;
	using ::OpenWaitableTimerW;
	using ::CancelWaitableTimer;
	using ::Process32FirstW;
	using ::ProcessIdToSessionId;
	using ::Process32NextW;
	using ::SetInformationJobObject;
	using ::AssignProcessToJobObject;
	using ::CreateJobObjectW;
	using ::OpenJobObjectW;
	using ::InitializeCriticalSection;
	using ::ReleaseSRWLockShared;
	using ::AcquireSRWLockShared;
	using ::AcquireSRWLockExclusive;
	using ::ReleaseSRWLockExclusive;
	using ::TryAcquireSRWLockShared;
	using ::TryAcquireSRWLockExclusive;
	using ::InitializeSRWLock;
	using ::CreateFileMappingW;
	using ::OpenFileMappingW;
	using ::ReleaseSemaphore;
	using ::CreateSemaphoreW;
	using ::OpenSemaphoreW;
	using ::QueueUserAPC;
	using ::CreateMutexW;
	using ::OpenMutexW;
	using ::ReleaseMutex;
	using ::WaitOnAddress;
	using ::WakeByAddressSingle;
	using ::WakeByAddressAll;
	using ::CreateProcessW;
	using ::InitializeSynchronizationBarrier;
	using ::EnterSynchronizationBarrier;
	using ::DeleteSynchronizationBarrier;
	using ::TerminateThread;
	using ::SuspendThread;
	using ::ResumeThread;
	using ::GetExitCodeThread;
	using ::SetThreadDescription;
	using ::GetThreadDescription;
	using ::_beginthreadex;
	using ::_get_errno;
	using ::ChangeTimerQueueTimer;
	using ::DeleteTimerQueueTimer;
	using ::CreateTimerQueueTimer;
	using ::CreateTimerQueue;
	using ::DeleteTimerQueueEx;
	using ::SetThreadpoolThreadMaximum;
	using ::SetThreadpoolThreadMinimum;
	using ::DestroyThreadpoolEnvironment;
	using ::CreateThreadpoolWork;
	using ::SubmitThreadpoolWork;
	using ::InitializeThreadpoolEnvironment;
	using ::SetThreadpoolCallbackPool;
	using ::SetThreadpoolCallbackRunsLong;
	using ::CreateThreadpool;
	using ::CloseThreadpool;
	using ::RtlSecureZeroMemory;
	using ::MapViewOfFile;
	using ::CreateFileMappingW;
	using ::OpenFileMappingW;
	using ::UnmapViewOfFile;
	using ::GetFileVersionInfoSizeW;
	using ::GetFileVersionInfoW;
	using ::VerQueryValueW;
	using ::CreateDirectoryW;
	using ::ConvertStringSecurityDescriptorToSecurityDescriptorW;
	using ::MoveFileExW;
	using ::WriteFile;
	using ::ReadFile;
	using ::CreateIoCompletionPort;
	using ::GetQueuedCompletionStatus;
	using ::Compress;
	using ::ResetCompressor;
	using ::CreateCompressor;
	using ::CloseCompressor;
	using ::CloseDecompressor;
	using ::Decompress;
	using ::CreateDecompressor;
	using ::ResetDecompressor;
	using ::CryptProtectMemory;
	using ::CryptUnprotectMemory;
	using ::BCryptDestroyKey;

	using ::IP_ADAPTER_ADDRESSES;
	using ::GetAdaptersAddresses;
}
