#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import Boring32;

namespace Win32
{
	TEST_CLASS(NTDLL)
	{
		TEST_METHOD(TestLoad)
		{
			Boring32::Native::NTDLL ntdll;
		}
	};
}