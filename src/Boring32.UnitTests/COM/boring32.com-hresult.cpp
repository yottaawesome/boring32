#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.com;

namespace COM
{
	TEST_CLASS(HResult)
	{
		public:
			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::COM::HResult hr;
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::COM::HResult hr1(CS_E_PACKAGE_NOTFOUND);
				Boring32::COM::HResult hr2(std::move(hr1));
				Assert::IsTrue(hr1.Get() == CS_E_PACKAGE_NOTFOUND);
				Assert::IsTrue(hr2.Get() == CS_E_PACKAGE_NOTFOUND);
			}
	};
}