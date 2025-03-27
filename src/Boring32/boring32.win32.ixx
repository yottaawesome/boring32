module;

#include "framework.h"

export module boring32.win32;

template<auto VValue, typename TValue = decltype(VValue)>
struct Constant
{
	constexpr operator TValue() const noexcept
	{
		return VValue;
	}
};

// Experimental module intended to decouple the codebase from having to import Windows headers.
export namespace Boring32::Win32
{
	constexpr auto _SECURITY_BUILTIN_DOMAIN_RID = SECURITY_BUILTIN_DOMAIN_RID;
	constexpr auto _DOMAIN_ALIAS_RID_ADMINS = DOMAIN_ALIAS_RID_ADMINS;
	using 
		::HMODULE,
		::SECURITY_ATTRIBUTES,
		::DWORD,
		::UINT,
		::USHORT,
		::HANDLE,
		::PHANDLE,
		::UNICODE_STRING,
		::PUNICODE_STRING,
		::PVOID,
		::ULONG_PTR,
		::SIZE_T,
		::PSIZE_T,
		::PLARGE_INTEGER,
		::LARGE_INTEGER,
		::ULONG,
		::PHANDLE,
		::ACCESS_MASK,
		::PULONG,
		::HMODULE,
		::NTSTATUS,
		::BOOL,
		::MSIHANDLE,
		::MSIINSTALLCONTEXT,
		::PMSIHANDLE,
		::PSID,
		::HRESULT,
		::SLIST_ENTRY,
		::PSLIST_ENTRY,
		::SLIST_HEADER,
		::PSLIST_HEADER,
		::MEMORYSTATUSEX,
		::SYSTEM_INFO,
		::SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX,
		::LOGICAL_PROCESSOR_RELATIONSHIP,
		::COMPUTER_NAME_FORMAT,
		::FILETIME,
		::ULARGE_INTEGER,
		::LPCSTR,
		::PCSTR,
		::COINIT,
		::EOLE_AUTHENTICATION_CAPABILITIES,
		::CRITICAL_SECTION,
		::PROCESSENTRY32W,
		::PTIMERAPCROUTINE,
		::JOBOBJECT_EXTENDED_LIMIT_INFORMATION,
		::JOBOBJECTINFOCLASS,
		::SRWLOCK,
		::PROCESS_INFORMATION,
		::PAPCFUNC,
		::STARTUPINFO,
		::SYNCHRONIZATION_BARRIER,
		::WAITORTIMERCALLBACK,
		::PTP_CALLBACK_INSTANCE,
		::PTP_WORK,
		::TP_POOL,
		::TP_CALLBACK_ENVIRON,
		::OVERLAPPED,
		::VS_FIXEDFILEINFO,
		::COMPRESSOR_HANDLE,
		::DECOMPRESSOR_HANDLE,
		::BCRYPT_KEY_HANDLE,
		::LPWSTR,
		::BYTE,
		::DATA_BLOB,
		::PUCHAR,
		::PCCERT_CHAIN_CONTEXT,
		::CERT_CHAIN_CONTEXT,
		::CERT_NAME_BLOB,
		::HCERTSTORE,
		::CERT_CHAIN_FIND_BY_ISSUER_PARA,
		::CERT_CONTEXT,
		::PCCERT_CONTEXT,
		::CERT_ENHKEY_USAGE,
		::CERT_USAGE_MATCH,
		::CERT_CHAIN_PARA,
		::CRYPTUI_WIZ_IMPORT_SRC_INFO,
		::LPSTR,
		::SYSTEMTIME,
		::FILETIME,
		::TIME_ZONE_INFORMATION,
		::SC_HANDLE,
		::LPCWSTR,
		::DWORD_PTR,
		::LSTATUS,
		::LONG,
		::LPOVERLAPPED,
		::LPVOID,
		::LPDWORD,
		::LSA_UNICODE_STRING,
		::SECURITY_DESCRIPTOR_CONTROL,
		::PSECURITY_DESCRIPTOR,
		::SID_IDENTIFIER_AUTHORITY,
		::PSID_IDENTIFIER_AUTHORITY,
		::PDWORD,
		::LSA_HANDLE,
		::LSA_OBJECT_ATTRIBUTES,
		::ACCESS_MASK,
		::SECURITY_IMPERSONATION_LEVEL,
		::TOKEN_TYPE,
		::LUID,
		::TOKEN_PRIVILEGES,
		::TOKEN_MANDATORY_LABEL,
		::TOKEN_INFORMATION_CLASS,
		::PTOKEN_GROUPS,
		::SID_NAME_USE,
		::PBYTE,
		::LUID_AND_ATTRIBUTES,
		::PRIVILEGE_SET,
		::SID_NAME_USE,
		::SECURITY_DESCRIPTOR,
		::HWND,
		::HMENU,
		::UINT_PTR,
		::WPARAM,
		::LPARAM,
		::LRESULT,
		::CERT_SIMPLE_CHAIN,
		::CERT_CHAIN_POLICY_PARA,
		::CERT_CHAIN_POLICY_STATUS,
		::BCRYPT_ALG_HANDLE,
		::BCRYPT_KEY_HANDLE,
		::TOKEN_STATISTICS,
		::TOKEN_GROUPS,
		::PrivilegeCheck,
		::LsaClose,
		::LsaOpenPolicy,
		::LsaNtStatusToWinError,
		::LsaAddAccountRights,
		::LsaRemoveAccountRights,
		::DuplicateTokenEx,
		::GetCurrentProcess,
		::OpenProcessToken,
		::LookupPrivilegeValueW,
		::AdjustTokenPrivileges,
		::SetTokenInformation,
		::GetTokenInformation,
		::GetLengthSid,
		::EqualSid,
		::LookupAccountSidW,
		::CheckTokenMembership,
		::LookupPrivilegeNameW,
		::K32EnumDeviceDrivers,
		::K32EnumProcesses,
		::GetLastError,
		::GetProcAddress,
		::GetModuleHandleW,
		::DuplicateHandle,
		::GetCurrentProcess,
		::GetHandleInformation,
		::SetHandleInformation,
		::GetProcAddress,
		::FormatMessageA,
		::FormatMessageW,
		::LoadLibraryW,
		::LoadLibraryExW,
		::FreeLibrary,
		::MsiIsProductElevatedW,
		::MsiOpenPackageW,
		::MsiCloseHandle,
		::MsiGetProductInfoExW,
		::MsiOpenDatabaseW,
		::MsiDatabaseOpenViewW,
		::MsiViewExecute,
		::MsiViewFetch,
		::MsiRecordGetStringW,
		::MsiEnumProductsExW,
		::CreateFileW,
		::CloseHandle,
		::LocalFree,
		::FreeSid,
		::InitializeSListHead,
		::QueryDepthSList,
		::InterlockedPopEntrySList,
		::InterlockedPushEntrySList,
		::InterlockedFlushSList,
		::GetComputerNameExW,
		::GetPhysicallyInstalledSystemMemory,
		::GlobalMemoryStatusEx,
		::GetTickCount64,
		::GetSystemTimeAdjustment,
		::GetSystemInfo,
		::GetLogicalProcessorInformationEx,
		::GetProcessTimes,
		::GetProcessHandleCount,
		::GetExitCodeProcess,
		::K32GetModuleFileNameExW,
		::OpenProcess,
		::GetProcessId,
		::WideCharToMultiByte,
		::MultiByteToWideChar,
		::CoUninitialize,
		::GetCurrentThreadId,
		::CoInitializeSecurity,
		::CoInitializeEx,
		::InitializeCriticalSectionEx,
		::InitializeCriticalSectionAndSpinCount,
		::DeleteCriticalSection,
		::LeaveCriticalSection,
		::EnterCriticalSection,
		::WaitForSingleObjectEx,
		::WaitForSingleObject,
		::WaitForMultipleObjectsEx,
		::WaitForMultipleObjects,
		::OpenEventW,
		::CreateEventW,
		::SetEvent,
		::ResetEvent,
		::CreateWaitableTimerW,
		::SetWaitableTimer,
		::CreateToolhelp32Snapshot,
		::OpenWaitableTimerW,
		::CancelWaitableTimer,
		::Process32FirstW,
		::ProcessIdToSessionId,
		::Process32NextW,
		::SetInformationJobObject,
		::AssignProcessToJobObject,
		::CreateJobObjectW,
		::OpenJobObjectW,
		::InitializeCriticalSection,
		::ReleaseSRWLockShared,
		::AcquireSRWLockShared,
		::AcquireSRWLockExclusive,
		::ReleaseSRWLockExclusive,
		::TryAcquireSRWLockShared,
		::TryAcquireSRWLockExclusive,
		::InitializeSRWLock,
		::CreateFileMappingW,
		::OpenFileMappingW,
		::ReleaseSemaphore,
		::CreateSemaphoreW,
		::OpenSemaphoreW,
		::QueueUserAPC,
		::CreateMutexW,
		::OpenMutexW,
		::ReleaseMutex,
		::WaitOnAddress,
		::WakeByAddressSingle,
		::WakeByAddressAll,
		::CreateProcessW,
		::InitializeSynchronizationBarrier,
		::EnterSynchronizationBarrier,
		::DeleteSynchronizationBarrier,
		::TerminateThread,
		::SuspendThread,
		::ResumeThread,
		::GetExitCodeThread,
		::SetThreadDescription,
		::GetThreadDescription,
		::_beginthreadex,
		::_get_errno,
		::ChangeTimerQueueTimer,
		::DeleteTimerQueueTimer,
		::CreateTimerQueueTimer,
		::CreateTimerQueue,
		::DeleteTimerQueueEx,
		::SetThreadpoolThreadMaximum,
		::SetThreadpoolThreadMinimum,
		::DestroyThreadpoolEnvironment,
		::CreateThreadpoolWork,
		::SubmitThreadpoolWork,
		::InitializeThreadpoolEnvironment,
		::SetThreadpoolCallbackPool,
		::SetThreadpoolCallbackRunsLong,
		::CreateThreadpool,
		::CloseThreadpool,
		::RtlSecureZeroMemory,
		::MapViewOfFile,
		::CreateFileMappingW,
		::OpenFileMappingW,
		::UnmapViewOfFile,
		::GetFileVersionInfoSizeW,
		::GetFileVersionInfoW,
		::VerQueryValueW,
		::CreateDirectoryW,
		::ConvertStringSecurityDescriptorToSecurityDescriptorW,
		::MoveFileExW,
		::WriteFile,
		::ReadFile,
		::CreateIoCompletionPort,
		::GetQueuedCompletionStatus,
		::Compress,
		::ResetCompressor,
		::CreateCompressor,
		::CloseCompressor,
		::CloseDecompressor,
		::Decompress,
		::CreateDecompressor,
		::ResetDecompressor,
		::CryptProtectMemory,
		::CryptUnprotectMemory,
		::BCryptDestroyKey,
		::CryptProtectData,
		::CryptUnprotectData,
		::BCryptEncrypt,
		::BCryptDecrypt,
		::CryptBinaryToStringA,
		::CryptBinaryToStringW,
		::CertStrToNameW,
		::CertNameToStrW,
		::CertFindChainInStore,
		::CertDuplicateCertificateChain,
		::CertGetCertificateChain,
		::CryptUIWizImport,
		::CertFreeCertificateChain,
		::CertFindCertificateInStore,
		::CryptStringToBinaryW,
		::EncodePointer,
		::DecodePointer,
		::GetStdHandle,
		::CreatePipe,
		::SetHandleInformation,
		::XAudio2Create,
		::SetFilePointer,
		::SystemTimeToFileTime,
		::GetSystemTimeAsFileTime,
		::FileTimeToSystemTime,
		::GetDateFormatEx,
		::GetTimeFormatEx,
		::GetTimeZoneInformation,
		::GetSystemTime,
		::CloseServiceHandle,
		::StartServiceW,
		::ControlServiceExW,
		::DeleteService,
		::QueryServiceStatusEx,
		::ControlServiceExW,
		::QueryServiceConfigW,
		::OpenSCManagerW,
		::OpenServiceW,
		::CoCreateInstance,
		::HeapCreate,
		::HeapCompact,
		::HeapValidate,
		::HeapLock,
		::HeapUnlock,
		::HeapAlloc,
		::HeapDestroy,
		::HeapFree,
		::GetProcessHeap,
		::CreatePipe,
		::SetNamedPipeHandleState,
		::PeekNamedPipe,
		::CreateMailslotW,
		::ConnectNamedPipe,
		::GlobalFree,
		::SHDeleteKeyW,
		::WaitNamedPipeW,
		::RevertToSelf,
		::ImpersonateLoggedOnUser,
		::GetSecurityDescriptorControl,
		::ConvertSidToStringSidW,
		::GetSidSubAuthorityCount,
		::IsValidSid,
		::GetSidIdentifierAuthority,
		::GetSidSubAuthority,
		::ConvertStringSidToSidW,
		::AllocateAndInitializeSid,
		::DeleteBoundaryDescriptor,
		::ClosePrivateNamespace,
		::CreateBoundaryDescriptorW,
		::CreateWellKnownSid,
		::AddSIDToBoundaryDescriptor,
		::CreatePrivateNamespaceW,
		::OpenPrivateNamespaceW,
		::Sleep,
		::CertVerifyTimeValidity,
		::CertFreeCertificateContext,
		::CertGetPublicKeyLength,
		::CertDuplicateCertificateContext,
		::CertGetCertificateContextProperty,
		::CertVerifyCertificateChainPolicy,
		::CertDuplicateStore,
		::CertCloseStore,
		::CertAddCertificateContextToStore,
		::CertOpenSystemStoreW,
		::CertOpenStore,
		::CertEnumCertificatesInStore,
		::CertDeleteCertificateFromStore,
		::CertGetStoreProperty,
		::__fastfail,
		::BCryptCloseAlgorithmProvider,
		::BCryptGenerateSymmetricKey,
		::BCryptGetProperty,
		::BCryptSetProperty,
		::BCryptEncrypt,
		::BCryptDecrypt,
		::BCryptOpenAlgorithmProvider,
		::CreateWindowExW,
		::SetWindowSubclass,
		::GetCurrentProcessToken
		;

	constexpr auto TokenAllAccess = TOKEN_ALL_ACCESS;

	namespace i18n
	{
		using ::LCMapStringEx;

		namespace Locales
		{
			constexpr auto Invariant = LOCALE_NAME_INVARIANT;
		};

		enum LcMap
		{
			LowerCase = LCMAP_LOWERCASE,
			UpperCase = LCMAP_UPPERCASE
		};
	}

	constexpr auto _CERT_SIGNATURE_HASH_PROP_ID = CERT_SIGNATURE_HASH_PROP_ID;
	constexpr auto _CERT_SIGN_HASH_CNG_ALG_PROP_ID = CERT_SIGN_HASH_CNG_ALG_PROP_ID;

	constexpr auto SePrivilegeEnabled = SE_PRIVILEGE_ENABLED;
	constexpr auto SeGroupIntegrity = SE_GROUP_INTEGRITY;
	constexpr auto SeGroupEnabled = SE_GROUP_ENABLED;
	constexpr auto SeGroupUseForDenyOnly = SE_GROUP_USE_FOR_DENY_ONLY;
	constexpr auto SePrivilegeEnabledByDefault = SE_PRIVILEGE_ENABLED_BY_DEFAULT;
	constexpr auto SePrivilegeRemoved= SE_PRIVILEGE_REMOVED;
	constexpr auto SePrivilegeUsedForAccess= SE_PRIVILEGE_USED_FOR_ACCESS;
	constexpr auto PrivilegeSetAllNecessary = PRIVILEGE_SET_ALL_NECESSARY;

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

	using ::SERVICE_CONTROL_STATUS_REASON_PARAMS;

	constexpr WORD MakeWord(DWORD a, DWORD b)
	{
		return MAKEWORD(a, b);
	}

	constexpr auto TimeZoneIdInvalid = TIME_ZONE_ID_INVALID;
	constexpr auto CRYPTUI_WIZ_IGNORE_NO_UI_FLAG_FOR_CSPS = 0x0002; // not defined according to msdn
	constexpr auto _CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT = CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT;
	constexpr auto CryptUiWizIgnoreNoUiFlagForCsps = CRYPTUI_WIZ_IGNORE_NO_UI_FLAG_FOR_CSPS;
	constexpr auto CryptUiWizNoUi = CRYPTUI_WIZ_NO_UI;
	constexpr auto CryptUiWizImportAllowCert = CRYPTUI_WIZ_IMPORT_ALLOW_CERT;
	constexpr auto _CRYPTUI_WIZ_IMPORT_SUBJECT_FILE = CRYPTUI_WIZ_IMPORT_SUBJECT_FILE;
	using ::CRYPT_HASH_BLOB;

	constexpr auto UsageMatchTypeAnd = USAGE_MATCH_TYPE_AND;

	constexpr Constant<STD_OUTPUT_HANDLE> StdOutputHandle;

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

	constexpr ::DWORD LowDWord(auto _qw) noexcept
	{
		return LODWORD(_qw);
	}

	constexpr ::DWORD HighDWord(auto _qw) noexcept
	{
		return HIDWORD(_qw);
	}

	constexpr HRESULT MakeHResult(const long severity, const long facility, const long code) noexcept
	{
		return MAKE_HRESULT(severity, facility, code);
	}

	constexpr long Facility(const HRESULT hr) noexcept
	{
		return HRESULT_FACILITY(hr);
	}

	constexpr long Code(const HRESULT hr) noexcept
	{
		return HRESULT_CODE(hr);
	}

	constexpr long Severity(const HRESULT hr) noexcept
	{
		return HRESULT_SEVERITY(hr);
	}

	constexpr bool HrFailed(const HRESULT hr) noexcept
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
	constexpr auto _CERT_HASH_PROP_ID = CERT_HASH_PROP_ID;

	constexpr auto LocaleNameInvariant = LOCALE_NAME_INVARIANT;

	constexpr bool BCryptSuccess(NTSTATUS status) noexcept
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
		constexpr auto NoneMapped = ERROR_NONE_MAPPED;
		constexpr auto NotAllAssigned = ERROR_NOT_ALL_ASSIGNED;
		constexpr auto FileNotFound = ERROR_FILE_NOT_FOUND;
		constexpr auto NoMoreFiles = ERROR_NO_MORE_FILES;
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

	constexpr auto _KEY_READ = KEY_READ;

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

	namespace WinSafer
	{
		using ::SAFER_LEVEL_HANDLE;
		using 
			::SaferCreateLevel,
			::SaferCloseLevel,
			::SaferComputeTokenFromLevel;

		enum class Scope
		{
			Machine = SAFER_SCOPEID_MACHINE,
			User = SAFER_SCOPEID_USER
		};

		enum class Level
		{
			Constrained = SAFER_LEVELID_CONSTRAINED,
			Disallowed = SAFER_LEVELID_DISALLOWED,
			FullyTrusted = SAFER_LEVELID_FULLYTRUSTED,
			NormalUser = SAFER_LEVELID_NORMALUSER,
			Untrusted = SAFER_LEVELID_UNTRUSTED
		};

		enum class Flags
		{
			Open = SAFER_LEVEL_OPEN
		};

		enum TokenFlags : ::DWORD
		{
			NullIfEqual = SAFER_TOKEN_NULL_IF_EQUAL,
			CompareOnly = SAFER_TOKEN_COMPARE_ONLY,
			MakeInert = SAFER_TOKEN_MAKE_INERT,
			WantFlags = SAFER_TOKEN_WANT_FLAGS
		};
	}
	
	namespace FailFast
	{
		enum
		{
			FatalExit = FAST_FAIL_FATAL_APP_EXIT // FatalAppExit is deffed to something else
		};
	}

	const auto _CERT_STORE_PROV_SYSTEM_REGISTRY_W = CERT_STORE_PROV_SYSTEM_REGISTRY_W;
	constexpr auto _CERT_STORE_OPEN_EXISTING_FLAG = CERT_STORE_OPEN_EXISTING_FLAG;
	constexpr auto _CERT_SYSTEM_STORE_LOCAL_MACHINE = CERT_SYSTEM_STORE_LOCAL_MACHINE;
	const auto _CERT_STORE_PROV_MEMORY = CERT_STORE_PROV_MEMORY;
	constexpr auto _CERT_SYSTEM_STORE_CURRENT_USER = CERT_SYSTEM_STORE_CURRENT_USER;

	constexpr auto _CERT_STORE_LOCALIZED_NAME_PROP_ID = CERT_STORE_LOCALIZED_NAME_PROP_ID;
	enum ChainVerificationPolicy : std::uintptr_t
	{
		Base = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_BASE),
		Authenticode = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_AUTHENTICODE),
		AuthenticodeTS = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_AUTHENTICODE_TS),
		SSL = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_SSL),
		BasicConstraints = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_BASIC_CONSTRAINTS),
		NTAuth = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_NT_AUTH),
		MicrosoftRoot = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_MICROSOFT_ROOT),
		EV = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_EV),
		SSLF12 = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_SSL_F12)
	};
	enum class CertAddDisposition : DWORD
	{
		AddAlways = CERT_STORE_ADD_ALWAYS,
		AddNew = CERT_STORE_ADD_NEW,
		AddNewer = CERT_STORE_ADD_NEWER,
		AddNewerInheritProperties = CERT_STORE_ADD_NEWER_INHERIT_PROPERTIES,
		ReplaceExisting = CERT_STORE_ADD_REPLACE_EXISTING,
		ReplaceExistingInheritProperties = CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES,
		AddUseExisting = CERT_STORE_ADD_USE_EXISTING
	};
	enum class CertStoreCloseOptions : DWORD
	{
		Default = 0,
		CheckNonFreedResources = CERT_CLOSE_STORE_CHECK_FLAG,
		ForceFreeMemory = CERT_CLOSE_STORE_FORCE_FLAG
	};

	
	constexpr auto _BCRYPT_OBJECT_LENGTH = BCRYPT_OBJECT_LENGTH;
	constexpr auto _BCRYPT_BLOCK_LENGTH = BCRYPT_BLOCK_LENGTH;
	constexpr auto _BCRYPT_CHAINING_MODE = BCRYPT_CHAINING_MODE;
	constexpr auto _BCRYPT_AES_ALGORITHM = BCRYPT_AES_ALGORITHM;
	constexpr auto _BCRYPT_BLOCK_PADDING = BCRYPT_BLOCK_PADDING;

	// https://blog.katastros.com/a?ID=00750-8d94e2aa-ea28-4faf-b67d-57642f88b0bb
	constexpr bool NT_SUCCESS(const NTSTATUS status) noexcept
	{
		return status >= 0;
	}

	constexpr bool NT_ERROR(const NTSTATUS status) noexcept
	{
		return status < 0;
	}

	using ::WELL_KNOWN_SID_TYPE;

	constexpr auto _SECURITY_MAX_SID_SIZE = SECURITY_MAX_SID_SIZE;

	// WLAN
	using ::PWLAN_INTERFACE_INFO_LIST;
	using ::WLAN_INTERFACE_INFO_LIST;
	using ::WLAN_OPCODE_VALUE_TYPE;
	using ::WLAN_INTF_OPCODE;
	using ::PWLAN_INTERFACE_INFO, ::WLAN_INTERFACE_INFO;
	using ::DOT11_AUTH_CIPHER_PAIR;
	using ::WLAN_AUTH_CIPHER_PAIR_LIST;
	using ::WLAN_INTF_OPCODE;
	using ::WLAN_CONNECTION_ATTRIBUTES;
	using ::WLAN_STATISTICS;
	using ::WLAN_INTERFACE_CAPABILITY;
	using ::DOT11_BSS_TYPE;
	using ::WlanOpenHandle;
	using ::WlanCloseHandle;
	using ::WlanFreeMemory;
	using ::WlanEnumInterfaces;
	using ::WlanQueryInterface;
	using ::WlanGetInterfaceCapability;
	// https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/ne-wlanapi-wlan_interface_state-r1
	enum class InterfaceState : int
	{
		// this is an enum type, so consider converting
		NotReady = wlan_interface_state_not_ready,
		Connected = wlan_interface_state_connected,
		AdHocNetworkFormed = wlan_interface_state_ad_hoc_network_formed,
		Disconnecting = wlan_interface_state_disconnecting,
		Disconnected = wlan_interface_state_disconnected,
		Associating = wlan_interface_state_associating,
		Discovering = wlan_interface_state_discovering,
		Authenticating = wlan_interface_state_authenticating
	};
	//

	constexpr auto _SID_MAX_SUB_AUTHORITIES = SID_MAX_SUB_AUTHORITIES;
	constexpr auto _TOKEN_QUERY = TOKEN_QUERY;

	using ::PTOKEN_USER;
	using ::TOKEN_USER;

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
	constexpr auto Sec_E_OK = SEC_E_OK;
	constexpr auto _SP_PROT_TLS1 = SP_PROT_TLS1;
	constexpr auto _SCH_CRED_NO_DEFAULT_CREDS = SCH_CRED_NO_DEFAULT_CREDS;
	constexpr auto _SCH_CRED_MANUAL_CRED_VALIDATION = SCH_CRED_MANUAL_CRED_VALIDATION;
	constexpr auto _SECPKG_CRED_OUTBOUND = SECPKG_CRED_OUTBOUND;
	constexpr auto _UNISP_NAME_W = UNISP_NAME_W;
	constexpr auto _SCHANNEL_CRED_VERSION = SCHANNEL_CRED_VERSION;
	constexpr auto _SCH_CREDENTIALS_VERSION = SCH_CREDENTIALS_VERSION;
	constexpr auto _ISC_REQ_CONFIDENTIALITY = ISC_REQ_CONFIDENTIALITY;
	constexpr auto _ISC_REQ_ALLOCATE_MEMORY = ISC_REQ_ALLOCATE_MEMORY;
	constexpr auto _SECURITY_LOCAL_SYSTEM_RID = SECURITY_LOCAL_SYSTEM_RID;
	constexpr ::SID_IDENTIFIER_AUTHORITY _SECURITY_NT_AUTHORITY = SECURITY_NT_AUTHORITY;
	enum class BufferType
	{
		Token = SECBUFFER_TOKEN,
		Data = SECBUFFER_DATA
	};


	constexpr auto DaclSecurityInformation =  DACL_SECURITY_INFORMATION;
	
	using ::TASK_CREATION;
	using ::TASK_LOGON_TYPE;
	using ::CLSCTX;
	using ::_bstr_t;
	using ::ITaskService;
	using ::ITaskFolder;
	using ::IRegisteredTask;
	using ::ITaskDefinition;
	using ::ITriggerCollection;
	using ::IRegisteredTaskCollection;
	using ::ITrigger;
	using ::IDailyTrigger;
	using ::IRepetitionPattern;
	using ::IRunningTask;
	using ::CLSID_TaskScheduler;
	using ::IID_ITaskService;
	using ::IP_ADAPTER_ADDRESSES;
	using ::_variant_t;
	using ::VARENUM;
	using ::TASK_TRIGGER_TYPE2;
	using ::GetAdaptersAddresses;
	constexpr auto VariantTrue = VARIANT_TRUE;
	constexpr auto VariantFalse = VARIANT_FALSE;

	namespace Services::StopReason
	{
		enum
		{
			FlagPlanned = SERVICE_STOP_REASON_FLAG_PLANNED,
			MajorNone = SERVICE_STOP_REASON_MAJOR_NONE,
			MinorNone = SERVICE_STOP_REASON_MINOR_NONE
		};
	}
	namespace Services::Control
	{
		enum
		{
			StatusReasonInfo = SERVICE_CONTROL_STATUS_REASON_INFO,
			Stop = SERVICE_CONTROL_STOP
		};
	}
	
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

	constexpr auto PipeAccessDuplex = PIPE_ACCESS_DUPLEX;
	constexpr auto FileFlagOverlapped = FILE_FLAG_OVERLAPPED;
	constexpr auto PipeTypeMessage = PIPE_TYPE_MESSAGE;
	constexpr auto PipeReadModeMessage = PIPE_READMODE_MESSAGE;
	constexpr auto PipeWait = PIPE_WAIT;
	constexpr auto PipeRejectRemoteClients = PIPE_REJECT_REMOTE_CLIENTS;
	constexpr auto PipeAcceptRemoteClients = PIPE_ACCEPT_REMOTE_CLIENTS;

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
	using ::timeval;
	constexpr auto _NS_ALL = NS_ALL;
	constexpr auto _NS_DNS = NS_DNS;
	
	using ::CoTaskMemFree;
}

export namespace Boring32::Win32::WinSock
{
	using
		::ADDRINFOW,
		::ADDRINFOEXW,
		::ADDRINFOEX,
		::WSAData,
		::PADDRINFOW,
		::sockaddr_in,
		::PADDRINFOEXW,
		::sockaddr_in6,
		::SOCKET,
		::IPPROTO,
		::FreeAddrInfoW,
		::FreeAddrInfoExW,
		::WSAStartup,
		::WSACleanup,
		::htonl,
		::inet_ntop,
		::WSAGetLastError,
		::GetAddrInfoW,
		::GetAddrInfoExW,
		::closesocket,
		::socket,
		::send,
		::recv,
		::getsockopt,
		::connect,
		::setsockopt
		;
	constexpr auto _INET_ADDRSTRLEN = INET_ADDRSTRLEN;
	constexpr auto _INET6_ADDRSTRLEN = INET6_ADDRSTRLEN;

	namespace AddressFamily
	{
		enum Family
		{
			IPv4 = AF_INET,
			IPv6 = AF_INET6,
			Unspecified = AF_UNSPEC
		};
	}
	
	constexpr auto _WSA_IO_PENDING = WSA_IO_PENDING;
	constexpr auto _INVALID_SOCKET = INVALID_SOCKET;
	constexpr auto _IPPROTO_IP = IPPROTO_IP;
	constexpr auto _SOCK_STREAM = SOCK_STREAM;
	constexpr auto _SOCKET_ERROR = SOCKET_ERROR;
	constexpr auto _IP_TTL = IP_TTL;
	constexpr auto _TCP_MAXRT = TCP_MAXRT;
	constexpr auto _IPV6_UNICAST_HOPS = IPV6_UNICAST_HOPS;
	constexpr auto _IPPROTO_TCP = IPPROTO_TCP;
}

export namespace Boring32::Win32::WinHttp
{
	using
		::URL_COMPONENTS,
		::WINHTTP_AUTOPROXY_OPTIONS,
		::HINTERNET,
		::WINHTTP_PROXY_INFO,
		::WINHTTP_STATUS_CALLBACK,
		::WINHTTP_WEB_SOCKET_STATUS,
		::WINHTTP_WEB_SOCKET_BUFFER_TYPE,
		::WINHTTP_ASYNC_RESULT,
		::WINHTTP_WEB_SOCKET_CLOSE_STATUS,
		::WinHttpCrackUrl,
		::WinHttpGetProxyForUrl,
		::WinHttpGetProxyForUrl,
		::WinHttpSetOption,
		::WinHttpOpen,
		::WinHttpCloseHandle,
		::WinHttpOpenRequest,
		::WinHttpSendRequest,
		::WinHttpQueryHeaders,
		::WinHttpReceiveResponse,
		::WinHttpQueryDataAvailable,
		::WinHttpReadData,
		::WinHttpConnect,
		::WinHttpSetOption,
		::WinHttpWebSocketCompleteUpgrade,
		::WinHttpWebSocketReceive,
		::WinHttpSendRequest,
		::WinHttpSetStatusCallback,
		::WinHttpWebSocketClose,
		::WinHttpQueryHeaders,
		::WinHttpWebSocketSend
		;

	namespace SecurityFlags
	{
		constexpr auto IgnoreUnknownCa = SECURITY_FLAG_IGNORE_UNKNOWN_CA;
		constexpr auto IgnoreCertWrongUsage = SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
		constexpr auto IgnoreCertCnInvalid = SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
		constexpr auto IgnoreCertDateInvalid = SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
	}

	namespace Options
	{
		constexpr auto Proxy = WINHTTP_OPTION_PROXY;
		constexpr auto SecurityFlags = WINHTTP_OPTION_SECURITY_FLAGS;
		constexpr auto ContextValue = WINHTTP_OPTION_CONTEXT_VALUE;
		constexpr auto ClientCertContext = WINHTTP_OPTION_CLIENT_CERT_CONTEXT;
		constexpr auto UpgradeToWebSocket = WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET;
	}

	constexpr auto AccessTypeNamedProxy = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
	constexpr auto AutoProxyConfigUrl = WINHTTP_AUTOPROXY_CONFIG_URL;
	constexpr auto NoProxyName = WINHTTP_NO_PROXY_NAME;
	constexpr auto NoProxyBypass = WINHTTP_NO_PROXY_BYPASS;
	constexpr auto NoReferer = WINHTTP_NO_REFERER;
	constexpr auto DefaultAcceptTypes = WINHTTP_DEFAULT_ACCEPT_TYPES;
	constexpr auto FlagSecure = WINHTTP_FLAG_SECURE;
	constexpr auto NoAdditionalHeaders = WINHTTP_NO_ADDITIONAL_HEADERS;
	constexpr auto NoRequestData = WINHTTP_NO_REQUEST_DATA;
	constexpr auto QueryStatusCode = WINHTTP_QUERY_STATUS_CODE;
	constexpr auto QueryFlagNumber = WINHTTP_QUERY_FLAG_NUMBER;
	constexpr auto HeaderNameByIndex = WINHTTP_HEADER_NAME_BY_INDEX;
	constexpr auto NoHeaderIndex = WINHTTP_NO_HEADER_INDEX;
	constexpr auto AccessTypeAutomaticProxy = WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY;
	constexpr auto AccessTypeNoProxy = WINHTTP_ACCESS_TYPE_NO_PROXY;
	constexpr auto SecurityFlagIgnoreAllCertErrors = SECURITY_FLAG_IGNORE_ALL_CERT_ERRORS;
	constexpr Constant<WINHTTP_INVALID_STATUS_CALLBACK> InvalidStatusCallback;
	constexpr auto CallbackFlagAllNotifications = WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS;
	constexpr auto FlagAsync = WINHTTP_FLAG_ASYNC;
	constexpr auto CallbackStatusRequestSent = WINHTTP_CALLBACK_STATUS_REQUEST_SENT;
	constexpr auto CallbackStatusHeadersAvailable = WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE;
	constexpr auto CallbackStatusReadComplete = WINHTTP_CALLBACK_STATUS_READ_COMPLETE;
	constexpr auto CallbackStatusWriteComplete = WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE;
	constexpr auto CallbackStatusRequestError = WINHTTP_CALLBACK_STATUS_REQUEST_ERROR;
	constexpr auto ApiReceiveResponse = API_RECEIVE_RESPONSE;
	constexpr auto ApiQueryDataAvailable = API_QUERY_DATA_AVAILABLE;
	constexpr auto ApiReadData = API_READ_DATA;
	constexpr auto ApiWriteData = API_WRITE_DATA;
	constexpr auto ApiSendRequest = API_SEND_REQUEST;
	constexpr auto CallbackStatusCloseComplete = WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE;
}

export namespace Boring32::Win32::Shell
{
	using WellKnownFolder = ::GUID;
	// https://learn.microsoft.com/en-us/windows/win32/shell/knownfolderid
	namespace WellKnownFolders
	{
		using
			::FOLDERID_AccountPictures,
			::FOLDERID_AddNewPrograms,
			::FOLDERID_AdminTools,
			::FOLDERID_AppDataDesktop,
			::FOLDERID_AppDataDocuments,
			::FOLDERID_AppDataFavorites,
			::FOLDERID_AppDataProgramData,
			::FOLDERID_ApplicationShortcuts,
			::FOLDERID_AppsFolder,
			::FOLDERID_AppUpdates,
			::FOLDERID_CameraRoll,
			::FOLDERID_CDBurning,
			::FOLDERID_ChangeRemovePrograms,
			::FOLDERID_CommonAdminTools,
			::FOLDERID_CommonOEMLinks,
			::FOLDERID_CommonPrograms,
			::FOLDERID_CommonStartMenu,
			::FOLDERID_CommonStartup,
			::FOLDERID_CommonTemplates,
			::FOLDERID_ComputerFolder,
			::FOLDERID_ConflictFolder,
			::FOLDERID_ConnectionsFolder,
			::FOLDERID_Contacts,
			::FOLDERID_ControlPanelFolder,
			::FOLDERID_Cookies,
			::FOLDERID_Desktop,
			::FOLDERID_DeviceMetadataStore,
			::FOLDERID_Documents,
			::FOLDERID_DocumentsLibrary,
			::FOLDERID_Downloads,
			::FOLDERID_Favorites,
			::FOLDERID_Fonts,
			::FOLDERID_Games,
			::FOLDERID_GameTasks,
			::FOLDERID_History,
			::FOLDERID_HomeGroup,
			::FOLDERID_HomeGroupCurrentUser,
			::FOLDERID_ImplicitAppShortcuts,
			::FOLDERID_InternetCache,
			::FOLDERID_InternetFolder,
			::FOLDERID_Libraries,
			::FOLDERID_Links,
			::FOLDERID_LocalAppData,
			::FOLDERID_LocalAppDataLow,
			::FOLDERID_LocalizedResourcesDir,
			::FOLDERID_Music,
			::FOLDERID_MusicLibrary,
			::FOLDERID_NetHood,
			::FOLDERID_NetworkFolder,
			::FOLDERID_Objects3D,
			::FOLDERID_OriginalImages,
			::FOLDERID_PhotoAlbums,
			::FOLDERID_PicturesLibrary,
			::FOLDERID_Playlists,
			::FOLDERID_PrintersFolder,
			::FOLDERID_PrintHood,
			::FOLDERID_Profile,
			::FOLDERID_ProgramData,
			::FOLDERID_ProgramFiles,
			::FOLDERID_ProgramFilesX64,
			::FOLDERID_ProgramFilesX86,
			::FOLDERID_ProgramFilesCommon,
			::FOLDERID_ProgramFilesCommonX64,
			::FOLDERID_ProgramFilesCommonX86,
			::FOLDERID_Programs,
			::FOLDERID_Public,
			::FOLDERID_PublicDesktop,
			::FOLDERID_PublicDocuments,
			::FOLDERID_PublicDownloads,
			::FOLDERID_PublicGameTasks,
			::FOLDERID_PublicLibraries,
			::FOLDERID_PublicMusic,
			::FOLDERID_PublicPictures,
			::FOLDERID_PublicRingtones,
			::FOLDERID_PublicUserTiles,
			::FOLDERID_PublicVideos,
			::FOLDERID_QuickLaunch,
			::FOLDERID_Recent,
			//::FOLDERID_RecordedTV, // Not used. This value is undefined as of Windows 7.
			::FOLDERID_RecordedTVLibrary,
			::FOLDERID_RecycleBinFolder,
			::FOLDERID_ResourceDir,
			::FOLDERID_Ringtones,
			::FOLDERID_RoamingAppData,
			::FOLDERID_RoamedTileImages,
			::FOLDERID_RoamingTiles,
			::FOLDERID_SampleMusic,
			::FOLDERID_SamplePictures,
			::FOLDERID_SamplePlaylists,
			::FOLDERID_SampleVideos,
			::FOLDERID_SavedGames,
			::FOLDERID_SavedPictures,
			::FOLDERID_SavedPicturesLibrary,
			::FOLDERID_SavedSearches,
			::FOLDERID_Screenshots,
			::FOLDERID_SEARCH_CSC,
			::FOLDERID_SearchHistory,
			::FOLDERID_SearchHome,
			::FOLDERID_SEARCH_MAPI,
			::FOLDERID_SearchTemplates,
			::FOLDERID_SendTo,
			::FOLDERID_SidebarDefaultParts,
			::FOLDERID_SidebarParts,
			::FOLDERID_SkyDrive,
			::FOLDERID_SkyDriveCameraRoll,
			::FOLDERID_SkyDriveDocuments,
			::FOLDERID_SkyDrivePictures,
			::FOLDERID_StartMenu,
			::FOLDERID_Startup,
			::FOLDERID_SyncManagerFolder,
			::FOLDERID_SyncResultsFolder,
			::FOLDERID_SyncSetupFolder,
			::FOLDERID_System,
			::FOLDERID_SystemX86,
			::FOLDERID_Templates,
			//::FOLDERID_TreeProperties, // Not used in Windows Vista. Unsupported as of Windows 7.
			::FOLDERID_UserPinned,
			::FOLDERID_UserProfiles,
			::FOLDERID_UserProgramFiles,
			::FOLDERID_UserProgramFilesCommon,
			::FOLDERID_UsersFiles,
			::FOLDERID_UsersLibraries,
			::FOLDERID_Videos,
			::FOLDERID_VideosLibrary,
			::FOLDERID_Windows;
	}

	namespace KnownFolderFlags
	{
		using ::KNOWN_FOLDER_FLAG;
	}

	using ::IKnownFolder;
	using ::IKnownFolderManager;

	using ::SHGetKnownFolderPath;
}


export namespace Boring32::Win32::Winreg
{
	using ::HKEY;
	using ::HKEY__;

	namespace Keys
	{
		template<HKEY VKey>
		struct Key
		{
			constexpr operator HKEY() const noexcept { return VKey; }
		};

		constexpr Key<HKEY_CLASSES_ROOT> HKCR;
		constexpr Key<HKEY_CURRENT_CONFIG> HKCC;
		constexpr Key<HKEY_CURRENT_USER> HKCU;
		constexpr Key<HKEY_LOCAL_MACHINE> HKLM;
		constexpr Key<HKEY_USERS> HKU;
	}

	constexpr auto _RRF_RT_REG_SZ = RRF_RT_REG_SZ;
	constexpr auto _RRF_RT_REG_DWORD = RRF_RT_REG_DWORD;
	constexpr auto _RRF_RT_REG_QWORD = RRF_RT_REG_QWORD;
	constexpr auto _REG_NOTIFY_CHANGE_LAST_SET = REG_NOTIFY_CHANGE_LAST_SET;

	constexpr auto _REG_LATEST_FORMAT = REG_LATEST_FORMAT;
	constexpr auto _KEY_ALL_ACCESS = KEY_ALL_ACCESS;
	constexpr auto _KEY_SET_VALUE = KEY_SET_VALUE;

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

	using
		::RegGetValueW,
		::RegSetValueExW,
		::RegNotifyChangeKeyValue,
		::RegDeleteTreeW,
		::RegOpenKeyExW,
		::RegQueryInfoKeyW,
		::RegEnumValueW,
		::RegSaveKeyExW,
		::RegDeleteValueW,
		::RegCloseKey,
		::RegCreateKeyExW,
		::RegOpenCurrentUser
		;
}
