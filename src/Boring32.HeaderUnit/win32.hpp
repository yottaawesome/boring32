#pragma once
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
