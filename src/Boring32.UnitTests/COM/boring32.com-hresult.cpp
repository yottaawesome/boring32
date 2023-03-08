#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.com;

namespace COM
{
	constexpr HRESULT TestHr = CS_E_PACKAGE_NOTFOUND;

	TEST_CLASS(HResult)
	{
		void TestCopyOrMove(Boring32::COM::HResult& from, Boring32::COM::HResult to)
		{
			Assert::IsTrue(from.Get() == TestHr);
			Assert::IsTrue(to.Get() == TestHr);
		}

		public:
			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::COM::HResult hr;
			}

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::COM::HResult hr1(TestHr);
				Boring32::COM::HResult hr2(hr1);
				TestCopyOrMove(hr1, hr2);
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::COM::HResult hr1(TestHr);
				Boring32::COM::HResult hr2(std::move(hr1));
				TestCopyOrMove(hr1, hr2);
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::COM::HResult hr1(TestHr);
				Boring32::COM::HResult hr2 = std::move(hr1);
				TestCopyOrMove(hr1, hr2);
			}
	};
}