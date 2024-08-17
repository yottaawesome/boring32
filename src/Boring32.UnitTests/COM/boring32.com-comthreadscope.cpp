#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace COM
{
	TEST_CLASS(COMThreadScope)
	{
		public:
			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::COM::COMThreadScope threadScope;
			}

			TEST_METHOD(TestInitConstructor)
			{
				Boring32::COM::COMThreadScope threadScope(COINIT::COINIT_APARTMENTTHREADED);
			}

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::COM::COMThreadScope threadScope1(COINIT::COINIT_APARTMENTTHREADED);
				Boring32::COM::COMThreadScope threadScope2(threadScope1);
			}

			TEST_METHOD(TestGetApartmentThreadingMode)
			{
				Boring32::COM::COMThreadScope threadScope(COINIT::COINIT_APARTMENTTHREADED);
				Assert::IsTrue(threadScope.GetApartmentThreadingMode() == COINIT::COINIT_APARTMENTTHREADED);
			}
	};
}