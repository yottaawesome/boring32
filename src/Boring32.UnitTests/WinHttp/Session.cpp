#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.winhttp;

namespace WinHttp
{
	TEST_CLASS(Session)
	{
		const std::wstring UserAgent = L"TestUserAgent";

		// Can't really test the named proxy type case
		public:
			TEST_METHOD(TestSessionDefaultConstructor)
			{
				Boring32::WinHttp::Session session;
				Assert::IsTrue(session.GetUserAgent().empty());
				Assert::IsTrue(session.GetProxyBypass().empty());
				Assert::IsTrue(session.GetNamedProxy().empty());
				Assert::IsTrue(session.GetProxyType() == Boring32::WinHttp::ProxyType::AutoProxy);
			}

			TEST_METHOD(TestSessionConstructorUserAgent)
			{
				Boring32::WinHttp::Session session(UserAgent);
				Assert::IsTrue(session.GetUserAgent() == UserAgent);
				Assert::IsTrue(session.GetProxyBypass().empty());
				Assert::IsTrue(session.GetNamedProxy().empty());
				Assert::IsTrue(session.GetProxyType() == Boring32::WinHttp::ProxyType::AutoProxy);
				Assert::IsNotNull(session.GetSession());
			}

			TEST_METHOD(TestSessionConstructorUserAgentNoProxyType)
			{
				Boring32::WinHttp::Session session(UserAgent, Boring32::WinHttp::ProxyType::NoProxy);
				Assert::IsTrue(session.GetUserAgent() == UserAgent);
				Assert::IsTrue(session.GetProxyBypass().empty());
				Assert::IsTrue(session.GetNamedProxy().empty());
				Assert::IsTrue(session.GetProxyType() == Boring32::WinHttp::ProxyType::NoProxy);
				Assert::IsNotNull(session.GetSession());
			}

			TEST_METHOD(TestSessionConstructorUserAgentAutoProxyType)
			{
				Boring32::WinHttp::Session session(UserAgent, Boring32::WinHttp::ProxyType::AutoProxy);
				Assert::IsTrue(session.GetUserAgent() == UserAgent);
				Assert::IsTrue(session.GetProxyBypass().empty());
				Assert::IsTrue(session.GetNamedProxy().empty());
				Assert::IsTrue(session.GetProxyType() == Boring32::WinHttp::ProxyType::AutoProxy);
				Assert::IsNotNull(session.GetSession());
			}

			TEST_METHOD(TestClose)
			{
				Boring32::WinHttp::Session session(UserAgent, Boring32::WinHttp::ProxyType::NoProxy);
				session.Close();
				Assert::IsTrue(session.GetUserAgent().empty());
				Assert::IsTrue(session.GetProxyBypass().empty());
				Assert::IsTrue(session.GetNamedProxy().empty());
				Assert::IsTrue(session.GetProxyType() == Boring32::WinHttp::ProxyType::NoProxy);
				Assert::IsNull(session.GetSession());
			}
	};
}