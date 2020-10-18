#pragma once
#ifdef ONYX32IMPORT
#define ONYXHEADER __declspec(dllimport)
#elif ONYX32EXPORT
#define ONYXHEADER __declspec(dllexport)
#else
#define ONYXHEADER
#endif