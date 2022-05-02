#pragma once

#include "targetver.hpp"

// Exclude rarely-used stuff from Windows headers
//#define WIN32_LEAN_AND_MEAN             

#include <iostream>
#include <sstream>
#include <Windows.h>
#include <process.h>
#include <pathcch.h>
#include <comdef.h>
#include <compressapi.h>
#include <Bcrypt.h>
#include <shlwapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "Pathcch.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "Cabinet.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "Cryptui.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Rpcrt4.lib")
