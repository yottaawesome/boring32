#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32;

namespace WinHttp
{
	TEST_CLASS(HttpWebClient)
	{
		const std::wstring UserAgent = L"TestUserAgent";


		TEST_METHOD(TestDefaultConstructor)
		{
			Boring32::WinHttp::HttpWebClient client(L"someuseragent", L"google.com", L"", 443, false);
			client.Connect();
			Boring32::WinHttp::HttpRequestResult result = client.Get(L"");
			Assert::IsTrue(result.StatusCode == 200);
			Assert::IsFalse(result.ResponseBody.empty());
		}

	};
}