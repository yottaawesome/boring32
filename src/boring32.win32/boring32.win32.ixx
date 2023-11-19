module;

#include "framework.h"

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
	using ::LPWSTR;
	using ::BYTE;
	using ::DATA_BLOB;
	using ::PUCHAR;
	using ::PCCERT_CHAIN_CONTEXT;
	using ::CERT_CHAIN_CONTEXT;
	using ::CERT_NAME_BLOB;
	using ::HCERTSTORE;
	using ::CERT_CHAIN_FIND_BY_ISSUER_PARA;
	using ::CERT_CONTEXT;
	using ::PCCERT_CONTEXT;
	using ::CERT_ENHKEY_USAGE;
	using ::CERT_USAGE_MATCH;
	using ::CERT_CHAIN_PARA;
	using ::CRYPTUI_WIZ_IMPORT_SRC_INFO;
	using ::LPSTR;
	using ::SYSTEMTIME;
	using ::FILETIME;
	using ::TIME_ZONE_INFORMATION;
	using ::SC_HANDLE;
	using ::LPCWSTR;
	using ::DWORD_PTR;
	using ::LSTATUS;
	using ::HKEY;
	using ::LONG;

	using ::IXAudio2;
	using ::IXAudio2MasteringVoice;
	using ::IXAudio2SourceVoice;

	using ::Microsoft::WRL::ComPtr;

	// See https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certfindcertificateinstore
	enum class StoreFindType : DWORD
	{
		Any = CERT_FIND_ANY,
		CertId = CERT_FIND_CERT_ID,
		CtlUsage = CERT_FIND_CTL_USAGE,
		EnhKeyUsage = CERT_FIND_ENHKEY_USAGE,
		Existing = CERT_FIND_EXISTING,
		Hash = CERT_FIND_HASH,
		HasPrivateKey = CERT_FIND_HAS_PRIVATE_KEY,
		IssuerAttr = CERT_FIND_ISSUER_ATTR,
		IssuerName = CERT_FIND_ISSUER_NAME,
		IssuerOf = CERT_FIND_ISSUER_OF,
		IssuerStr = CERT_FIND_ISSUER_STR,
		KeyIdentifier = CERT_FIND_KEY_IDENTIFIER,
		KeySpec = CERT_FIND_KEY_SPEC,
		Md5Hash = CERT_FIND_MD5_HASH,
		FindProperty = CERT_FIND_PROPERTY,
		PublicKey = CERT_FIND_PUBLIC_KEY,
		Sha1Hash = CERT_FIND_SHA1_HASH,
		SignatureHash = CERT_FIND_SIGNATURE_HASH,
		SubjectAttr = CERT_FIND_SUBJECT_ATTR,
		SubjectCert = CERT_FIND_SUBJECT_CERT,
		SubjectName = CERT_FIND_SUBJECT_NAME,
		SubjectStr = CERT_FIND_SUBJECT_STR,
		CrossCertDistPoints = CERT_FIND_CROSS_CERT_DIST_POINTS,
		PubKeyMd5Hash = CERT_FIND_PUBKEY_MD5_HASH
	};

	// https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-value-types
	enum class ValueTypes : DWORD
	{
		None = REG_NONE,
		Binary = REG_BINARY,
		DWord = REG_DWORD,
		DWordLittleEndian = REG_DWORD_LITTLE_ENDIAN,
		DWordBigEndian = REG_DWORD_BIG_ENDIAN,
		ExpandableString = REG_EXPAND_SZ,
		Link = REG_LINK,
		MultiString = REG_MULTI_SZ,
		QWord = REG_QWORD,
		QWordLittleEndian = REG_QWORD_LITTLE_ENDIAN,
		String = REG_SZ,
	};

	constexpr auto _REG_LATEST_FORMAT = REG_LATEST_FORMAT;

	constexpr auto _KEY_ALL_ACCESS = KEY_ALL_ACCESS;

	using ::SERVICE_CONTROL_STATUS_REASON_PARAMS;

	const auto _HKEY_CLASSES_ROOT = HKEY_CLASSES_ROOT;
	const auto _HKEY_CURRENT_CONFIG = HKEY_CURRENT_CONFIG;
	const auto _HKEY_CURRENT_USER = HKEY_CURRENT_USER;
	const auto _HKEY_LOCAL_MACHINE = HKEY_LOCAL_MACHINE;
	const auto _HKEY_USERS = HKEY_USERS;

	constexpr auto TimeZoneIdInvalid = TIME_ZONE_ID_INVALID;
	constexpr auto CRYPTUI_WIZ_IGNORE_NO_UI_FLAG_FOR_CSPS = 0x0002; // not defined according to msdn
	constexpr auto CryptUiWizIgnoreNoUiFlagForCsps = CRYPTUI_WIZ_IGNORE_NO_UI_FLAG_FOR_CSPS;
	constexpr auto CryptUiWizNoUi = CRYPTUI_WIZ_NO_UI;
	constexpr auto CryptUiWizImportAllowCert = CRYPTUI_WIZ_IMPORT_ALLOW_CERT;

	constexpr auto UsageMatchTypeAnd = USAGE_MATCH_TYPE_AND;

	const auto StdOutputHandle = STD_OUTPUT_HANDLE;

	enum class EncryptOptions
	{
		LocalMachine = CRYPTPROTECT_LOCAL_MACHINE,
		UiForbidden = CRYPTPROTECT_UI_FORBIDDEN,
		Audit = CRYPTPROTECT_AUDIT
	};

	enum class DecryptOptions
	{
		UiForbidden = CRYPTPROTECT_UI_FORBIDDEN,
		VerifyProtection = CRYPTPROTECT_VERIFY_PROTECTION
	};

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
	const auto InvalidSetFilePointer = INVALID_SET_FILE_POINTER;
	const auto FileCurrent = FILE_CURRENT;
	constexpr auto FileBegin = FILE_BEGIN;
	constexpr auto S_Ok = S_OK;
	// This an option for converting macros to constexpr without having the preprocessor
	// mess up the constexpr variable's name. Another option is to add the prefix _ to
	// be a suffix instead.
	constexpr const wchar_t* _INSTALLPROPERTY_PACKAGENAME = INSTALLPROPERTY_PACKAGENAME;

	constexpr auto StartFUsesStdHandle = STARTF_USESTDHANDLES;

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

	constexpr auto CryptStringBase64 = CRYPT_STRING_BASE64;
	constexpr auto CryptStringNoCrLf = CRYPT_STRING_NOCRLF;

	constexpr auto X509AsnEncoding = X509_ASN_ENCODING;
	constexpr auto CertX500NameStr = CERT_X500_NAME_STR;
	constexpr auto CertNameStrForceUtf8DirStrFlag = CERT_NAME_STR_FORCE_UTF8_DIR_STR_FLAG;
	constexpr auto CertChainFindByIssuer = CERT_CHAIN_FIND_BY_ISSUER;
	constexpr auto CertChainRevocationCheckChainExcludeRoot = CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;
	constexpr auto Pkcs7AsnEncoding = PKCS_7_ASN_ENCODING;
	constexpr auto CertFindHasPrivateKey = CERT_FIND_HAS_PRIVATE_KEY;

	constexpr auto LocaleNameInvariant = LOCALE_NAME_INVARIANT;

	inline bool BCryptSuccess(NTSTATUS status) noexcept
	{
		return BCRYPT_SUCCESS(status);
	}

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
		constexpr auto PipeBusy = ERROR_PIPE_BUSY;
	}

	namespace NTStatus // winnt.h
	{
		constexpr auto Pending = STATUS_PENDING;
	}

	namespace CryptoErrorCodes
	{
		constexpr auto NotFound = CRYPT_E_NOT_FOUND;
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

	constexpr auto XAudio2DefaultProcessor = XAUDIO2_DEFAULT_PROCESSOR;

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
	using ::CryptProtectData;
	using ::CryptUnprotectData;
	using ::BCryptEncrypt;
	using ::BCryptDecrypt;
	using ::CryptBinaryToStringA;
	using ::CryptBinaryToStringW;
	using ::CertStrToNameW;
	using ::CertNameToStrW;
	using ::CertFindChainInStore;
	using ::CertDuplicateCertificateChain;
	using ::CertGetCertificateChain;
	using ::CryptUIWizImport;
	using ::CertFreeCertificateChain;
	using ::CertFindCertificateInStore;
	using ::CryptStringToBinaryW;
	using ::EncodePointer;
	using ::DecodePointer;
	using ::GetStdHandle;
	using ::CreatePipe;
	using ::SetHandleInformation;
	using ::XAudio2Create;
	using ::SetFilePointer;
	using ::SystemTimeToFileTime;
	using ::GetSystemTimeAsFileTime;
	using ::FileTimeToSystemTime;
	using ::GetDateFormatEx;
	using ::GetTimeFormatEx;
	using ::GetTimeZoneInformation;
	using ::GetSystemTime;
	using ::CloseServiceHandle;
	using ::StartServiceW;
	using ::ControlServiceExW;
	using ::DeleteService;
	using ::QueryServiceStatusEx;
	using ::ControlServiceExW;
	using ::QueryServiceConfigW;
	using ::OpenSCManagerW;
	using ::OpenServiceW;
	using ::CoCreateInstance;
	using ::HeapCreate;
	using ::HeapCompact;
	using ::HeapValidate;
	using ::HeapLock;
	using ::HeapUnlock;
	using ::HeapAlloc;
	using ::HeapDestroy;
	using ::HeapFree;
	using ::GetProcessHeap;
	using ::CreatePipe;
	using ::SetNamedPipeHandleState;
	using ::PeekNamedPipe;
	using ::CreateMailslotW;
	using ::ConnectNamedPipe;
	using ::GlobalFree;
	using ::WaitNamedPipeW;
	using ::RegGetValueW;
	using ::RegSetValueExW;
	using ::RegNotifyChangeKeyValue;
	using ::SHDeleteKeyW;
	using ::RegDeleteTreeW;
	using ::RegOpenKeyExW;
	using ::RegQueryInfoKeyW;
	using ::RegEnumValueW;
	using ::RegSaveKeyExW;
	using ::RegCloseKey;

	using ::SecBuffer;
	using ::SecBufferDesc;
	using ::SECURITY_STATUS;
	using ::SCHANNEL_CRED;
	using ::CredHandle;
	using ::PCredHandle;
	using ::TimeStamp;
	using ::SCH_CREDENTIALS;
	using ::CtxtHandle;
	using ::SecHandle;
	using ::FreeContextBuffer;
	using ::FreeCredentialsHandle;
	using ::AcquireCredentialsHandleW;
	using ::InitializeSecurityContextW;
	constexpr auto _SEC_E_OK = SEC_E_OK;
	constexpr auto _SP_PROT_TLS1 = SP_PROT_TLS1;
	constexpr auto _SCH_CRED_NO_DEFAULT_CREDS = SCH_CRED_NO_DEFAULT_CREDS;
	constexpr auto _SCH_CRED_MANUAL_CRED_VALIDATION = SCH_CRED_MANUAL_CRED_VALIDATION;
	constexpr auto _SECPKG_CRED_OUTBOUND = SECPKG_CRED_OUTBOUND;
	constexpr auto _UNISP_NAME_W = UNISP_NAME_W;
	constexpr auto _SCHANNEL_CRED_VERSION = SCHANNEL_CRED_VERSION;
	constexpr auto _SCH_CREDENTIALS_VERSION = SCH_CREDENTIALS_VERSION;
	constexpr auto _ISC_REQ_CONFIDENTIALITY = ISC_REQ_CONFIDENTIALITY;
	constexpr auto _ISC_REQ_ALLOCATE_MEMORY = ISC_REQ_ALLOCATE_MEMORY;
	enum class BufferType
	{
		Token = SECBUFFER_TOKEN,
		Data = SECBUFFER_DATA
	};

	using ::HKEY__;

	using ::CLSCTX;
	using ::_bstr_t;
	using ::ITaskService;
	using ::ITaskFolder;
	using ::CLSID_TaskScheduler;
	using ::IID_ITaskService;
	using ::IP_ADAPTER_ADDRESSES;
	using ::_variant_t;
	using ::GetAdaptersAddresses;

	constexpr auto _RRF_RT_REG_SZ = RRF_RT_REG_SZ;
	constexpr auto _RRF_RT_REG_DWORD = RRF_RT_REG_DWORD;
	constexpr auto _RRF_RT_REG_QWORD = RRF_RT_REG_QWORD;
	constexpr auto _REG_NOTIFY_CHANGE_LAST_SET = REG_NOTIFY_CHANGE_LAST_SET;

	constexpr auto _SERVICE_STOP_REASON_FLAG_PLANNED = SERVICE_STOP_REASON_FLAG_PLANNED;
	constexpr auto _SERVICE_STOP_REASON_MAJOR_NONE = SERVICE_STOP_REASON_MAJOR_NONE;
	constexpr auto _SERVICE_STOP_REASON_MINOR_NONE = SERVICE_STOP_REASON_MINOR_NONE;
	constexpr auto _SERVICE_CONTROL_STATUS_REASON_INFO = SERVICE_CONTROL_STATUS_REASON_INFO;
	constexpr auto _SERVICE_CONTROL_STOP = SERVICE_CONTROL_STOP;
	constexpr auto _SERVICE_RUNNING = SERVICE_RUNNING;
	constexpr auto _SERVICES_ACTIVE_DATABASE = SERVICES_ACTIVE_DATABASE;
	constexpr auto _SC_MANAGER_ALL_ACCESS = SC_MANAGER_ALL_ACCESS;
	constexpr auto _SERVICE_ALL_ACCESS = SERVICE_ALL_ACCESS;
	using ::SC_STATUS_TYPE;

	using ::QUERY_SERVICE_CONFIGW;
	using ::SERVICE_STATUS_PROCESS;
	using ::SC_STATUS_TYPE;

	// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
	enum class AllocationType
	{
		Commit = MEM_COMMIT,
		Reserve = MEM_RESERVE,
		Reset = MEM_RESET,
		Undo = MEM_RESET_UNDO
	};

	enum class AllocationTypeOptional
	{
		LargePages = MEM_LARGE_PAGES,
		Physical = MEM_PHYSICAL,
		TopDown = MEM_TOP_DOWN,
		WriteWatch = MEM_WRITE_WATCH
	};

	enum class HeapCreateOptions : ::DWORD
	{
		EnableExecute = HEAP_CREATE_ENABLE_EXECUTE,
		GenerateExceptions = HEAP_GENERATE_EXCEPTIONS,
		NoSerialise = HEAP_NO_SERIALIZE
	};

	enum class HeapAllocOptions : ::DWORD
	{
		None = 0,
		ZeroMemory = HEAP_ZERO_MEMORY,
		GenerateExceptions = HEAP_GENERATE_EXCEPTIONS,
		NoSerialise = HEAP_NO_SERIALIZE
	};

	constexpr auto _PIPE_ACCESS_DUPLEX = PIPE_ACCESS_DUPLEX;
	constexpr auto _FILE_FLAG_OVERLAPPED = FILE_FLAG_OVERLAPPED;
	constexpr auto _PIPE_TYPE_MESSAGE = PIPE_TYPE_MESSAGE;
	constexpr auto _PIPE_READMODE_MESSAGE = PIPE_READMODE_MESSAGE;
	constexpr auto _PIPE_WAIT = PIPE_WAIT;
	constexpr auto _PIPE_REJECT_REMOTE_CLIENTS = PIPE_REJECT_REMOTE_CLIENTS;
	constexpr auto _PIPE_ACCEPT_REMOTE_CLIENTS = PIPE_ACCEPT_REMOTE_CLIENTS;

	using ::DisconnectNamedPipe;
	using ::FlushFileBuffers;
	using ::CancelIo;
	using ::CancelIoEx;
	using ::CreateNamedPipeW;

	using ::IIDFromString;
	using ::UuidFromStringW;
	using ::RPC_STATUS; 
	using ::RPC_WSTR;
	constexpr auto _RPC_S_OK = RPC_S_OK;
	using ::UuidIsNil;
	using ::GUID;
	using ::IsEqualGUID;

	using ::PathCchRemoveFileSpec;
	constexpr auto _S_FALSE = S_FALSE;
	using ::StringFromGUID2;
	using ::CoCreateGuid;
	using ::CLSID_NetworkListManager;
	using ::NLM_CONNECTIVITY;
	using ::GetModuleFileNameW;
	using ::IID_INetworkListManager;
	using ::INetworkListManager;

	using ::CLSID_WICImagingFactory2;
	using ::IWICBitmapDecoder;
	using ::WICDecodeOptions;
	using ::IWICFormatConverter;
	using ::IWICImagingFactory;

	constexpr auto _OPEN_EXISTING = OPEN_EXISTING;

	namespace WinHttp
	{
		using ::URL_COMPONENTS;
		using ::WinHttpCrackUrl;
		using ::HINTERNET;
		using ::WINHTTP_AUTOPROXY_OPTIONS;
		using ::WinHttpGetProxyForUrl;
		constexpr auto _WINHTTP_ACCESS_TYPE_NAMED_PROXY = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
		constexpr auto _WINHTTP_AUTOPROXY_CONFIG_URL = WINHTTP_AUTOPROXY_CONFIG_URL;
		using ::WinHttpGetProxyForUrl;
		constexpr auto _WINHTTP_OPTION_PROXY = WINHTTP_OPTION_PROXY;
		using ::WinHttpSetOption;
		using ::WINHTTP_PROXY_INFO;
		constexpr auto _WINHTTP_NO_PROXY_NAME = WINHTTP_NO_PROXY_NAME;
		using ::WinHttpOpen;
		constexpr auto _WINHTTP_NO_PROXY_BYPASS = WINHTTP_NO_PROXY_BYPASS;
		using ::WinHttpCloseHandle;
		using ::WinHttpOpenRequest;
		using ::WinHttpSendRequest;
		using ::WinHttpQueryHeaders;
		using ::WinHttpReceiveResponse;
		using ::WinHttpQueryDataAvailable;
		using ::WinHttpReadData;
		using ::WinHttpConnect;

		constexpr auto _WINHTTP_NO_REFERER = WINHTTP_NO_REFERER;
		constexpr auto _WINHTTP_DEFAULT_ACCEPT_TYPES = WINHTTP_DEFAULT_ACCEPT_TYPES;
		constexpr auto _WINHTTP_FLAG_SECURE = WINHTTP_FLAG_SECURE;
		constexpr auto _WINHTTP_OPTION_SECURITY_FLAGS = WINHTTP_OPTION_SECURITY_FLAGS;
		constexpr auto _SECURITY_FLAG_IGNORE_UNKNOWN_CA = SECURITY_FLAG_IGNORE_UNKNOWN_CA;
		constexpr auto _SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE = SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
		constexpr auto _SECURITY_FLAG_IGNORE_CERT_CN_INVALID = SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
		constexpr auto _SECURITY_FLAG_IGNORE_CERT_DATE_INVALID = SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
		constexpr auto _WINHTTP_NO_ADDITIONAL_HEADERS = WINHTTP_NO_ADDITIONAL_HEADERS;
		constexpr auto _WINHTTP_NO_REQUEST_DATA = WINHTTP_NO_REQUEST_DATA;
		constexpr auto _WINHTTP_QUERY_STATUS_CODE = WINHTTP_QUERY_STATUS_CODE;
		constexpr auto _WINHTTP_QUERY_FLAG_NUMBER = WINHTTP_QUERY_FLAG_NUMBER;
		constexpr auto _WINHTTP_HEADER_NAME_BY_INDEX = WINHTTP_HEADER_NAME_BY_INDEX;
		constexpr auto _WINHTTP_NO_HEADER_INDEX = WINHTTP_NO_HEADER_INDEX;
		constexpr auto _WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY = WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY;
		constexpr auto _WINHTTP_ACCESS_TYPE_NO_PROXY = WINHTTP_ACCESS_TYPE_NO_PROXY;
	}
}
