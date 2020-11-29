#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Error/Error.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Boring32::Error::UnitTests
{
	TEST_CLASS(Error)
	{
		public:
			TEST_METHOD(TestGetErrorCodeStringWinHttp)
			{
				// https://docs.microsoft.com/en-us/windows/win32/winhttp/error-messages
				std::wstring error = Boring32::Error::CreateErrorStringFromCode(L"", 12103);
				size_t pos = error.find_first_of(L"method cannot be called after the Open method has been called", 0);
				Assert::IsTrue(pos > 0);
			}
	};
}