#pragma once

#include "targetver.hpp"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <iostream>
#include <sstream>
#include <Windows.h>
#include <process.h>
#include <pathcch.h>
#include <comdef.h>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "Pathcch.lib")
#pragma comment(lib, "comsuppw.lib")
