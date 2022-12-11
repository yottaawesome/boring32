#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.com;

namespace COM
{
	TEST_CLASS(COMThreadScope)
	{
		public:
			TEST_METHOD(TestCompressorDefaultConstructor)
			{
				Boring32::COM::COMThreadScope threadScope;
			}
	};
}