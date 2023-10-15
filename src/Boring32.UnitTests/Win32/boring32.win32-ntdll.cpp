#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.native;

namespace Win32
{
	TEST_CLASS(NTDLL)
	{
		public:
			TEST_METHOD(TestLoad)
			{
				Boring32::Native::NTDLL ntdll;
			}
	};
}