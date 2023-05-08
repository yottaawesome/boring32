#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.error;
import boring32.async;

namespace Error
{
	TEST_CLASS(Functions)
	{
		public:
			TEST_METHOD(TestGetErrorCodeStringWinHttp)
			{
				// https://docs.microsoft.com/en-us/windows/win32/winhttp/error-messages
				std::wstring error = Boring32::Error::TranslateErrorCode<std::wstring>(12103, L"winhttp.dll");
				size_t pos = error.find_first_of(L"method cannot be called after the Open method has been called", 0);
				Assert::IsTrue(pos > 0);
			}

			TEST_METHOD(TestPrintExceptionToStringStream)
			{
				std::stringstream ss;
				try
				{
					throw std::logic_error("first");
				}
				catch (const std::exception& ex)
				{
					try
					{
						std::throw_with_nested(std::logic_error("second"));
					}
					catch (const std::exception& ex)
					{
						try
						{
							std::throw_with_nested(std::logic_error("third"));
						}
						catch (const std::exception& ex)
						{
							Boring32::Error::PrintExceptionToStringStream(ex, ss);
							Assert::IsFalse(ss.str().empty());
						}
					}
				}
			}

			TEST_METHOD(TestPrintExceptionToWStringStream)
			{
				std::wstringstream ss;
				try
				{
					throw std::logic_error("first");
				}
				catch (const std::exception& ex)
				{
					try
					{
						std::throw_with_nested(std::logic_error("second"));
					}
					catch (const std::exception& ex)
					{
						try
						{
							std::throw_with_nested(std::logic_error("third"));
						}
						catch (const std::exception& ex)
						{
							Boring32::Error::PrintExceptionToStringStream(ex, ss);
							Assert::IsFalse(ss.str().empty());
						}
					}
				}
			}
	};
}