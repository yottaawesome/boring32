#pragma once

#include "targetver.hpp"

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN             

#include <iostream>
#include <sstream>
#include <Windows.h>
#include <process.h>
#include <pathcch.h>
#include <comdef.h>
#include <compressapi.h>
#include <Bcrypt.h>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "Pathcch.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "Cabinet.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Bcrypt.lib")
