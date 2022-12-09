// See https://learn.microsoft.com/en-us/cpp/build/walkthrough-header-units
// https://learn.microsoft.com/en-us/cpp/build/walkthrough-import-stl-header-units
#pragma warning(disable:4005)
#pragma warning(disable:5106)
// The above need to be disabled because of
// https://developercommunity.visualstudio.com/t/warning-C4005:-Outptr:-macro-redefinit/1546919
// No idea when this will get fixed, MS seems to be taking their time with it.
import <Windows.h>;
import <Comdef.h>;
import <sddl.h>;
//import <wlanapi.h>; // unfortunately, not importable
import <wtypes.h>;
import <guiddef.h>;
// import <pathcch.h>; // unfortunately, not importable
import <Objbase.h>;
import <netlistmgr.h>;
import <wrl/client.h>;