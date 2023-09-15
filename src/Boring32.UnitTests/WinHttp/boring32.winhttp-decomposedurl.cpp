#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.winhttp;

namespace WinHttp
{
	TEST_CLASS(DecomposedURL)
	{
		std::wstring Url1 = L"https://www.boring32.com:80/some/path";
		std::wstring Url2 = L"https://www.32boring.com";

		public:
			TEST_METHOD(TestConstructor)
			{
				Boring32::WinHttp::DecomposedURL url(Url1);
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::WinHttp::DecomposedURL url1(Url1);
				Boring32::WinHttp::DecomposedURL url2(Url2);
				url2 = url1;
				Assert::IsTrue(url1.Url() == url2.Url());
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::WinHttp::DecomposedURL url1(Url1);
				Boring32::WinHttp::DecomposedURL url2(Url2);
				url2 = std::move(url1);
				Assert::IsTrue(url1.Url().empty()); 
				Assert::IsTrue(url2.Url() == Url1);
			}

			TEST_METHOD(TestGetScheme)
			{
				Boring32::WinHttp::DecomposedURL url1(Url1);
				Assert::IsTrue(url1.Scheme() == L"https");

			}

			TEST_METHOD(TestGetHost)
			{
				Boring32::WinHttp::DecomposedURL url1(Url1);
				Assert::IsTrue(url1.Host() == L"www.boring32.com");

			}

			TEST_METHOD(TestGetPath)
			{
				Boring32::WinHttp::DecomposedURL url1(Url1);
				Assert::IsTrue(url1.Path() == L"some/path");
			}

			TEST_METHOD(TestGetPort)
			{
				Boring32::WinHttp::DecomposedURL url1(Url1);
				Assert::IsTrue(url1.Port() == 80);
			}
	};
}