#pragma once
// See https://learn.microsoft.com/en-us/cpp/build/walkthrough-header-units
// https://learn.microsoft.com/en-us/cpp/build/walkthrough-import-stl-header-units
#pragma warning(disable:4005)
#pragma warning(disable:5106)
// The above need to be disabled because of
// https://developercommunity.visualstudio.com/t/warning-C4005:-Outptr:-macro-redefinit/1546919
// No idea when this will get fixed, MS seems to be taking their time with it.
#include <Windows.h>
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
