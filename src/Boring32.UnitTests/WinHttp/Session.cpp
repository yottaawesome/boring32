#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.winhttp;

namespace WinHttp
{
	TEST_CLASS(Session)
	{
		const std::wstring UserAgent = L"TestUserAgent";
		// I don't have an available proxy to test the other types...
		const Boring32::WinHttp::ProxyType ProxyType = Boring32::WinHttp::ProxyType::NoProxy;

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

			TEST_METHOD(TestSessionConstructorUserAgentProxyType)
			{
				Boring32::WinHttp::Session session(UserAgent, ProxyType);
				Assert::IsTrue(session.GetUserAgent() == UserAgent);
				Assert::IsTrue(session.GetProxyBypass().empty());
				Assert::IsTrue(session.GetNamedProxy().empty());
				Assert::IsTrue(session.GetProxyType() == ProxyType);
				Assert::IsNotNull(session.GetSession());
			}

			TEST_METHOD(TestClose)
			{
				Boring32::WinHttp::Session session(UserAgent, ProxyType);
				session.Close();
				Assert::IsTrue(session.GetUserAgent().empty());
				Assert::IsTrue(session.GetProxyBypass().empty());
				Assert::IsTrue(session.GetNamedProxy().empty());
				Assert::IsTrue(session.GetProxyType() == Boring32::WinHttp::ProxyType::NoProxy);
				Assert::IsNull(session.GetSession());
			}
	};
}