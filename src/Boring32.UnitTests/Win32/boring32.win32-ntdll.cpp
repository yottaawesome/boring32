#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.win32;

namespace Win32
{
	TEST_CLASS(NTDLL)
	{
		public:
			TEST_METHOD(TestLoad)
			{
				Boring32::Win32::NTDLL ntdll;
			}
	};
}