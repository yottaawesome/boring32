#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32;

namespace WinHttp
{
	TEST_CLASS(Session)
	{
		private:
			const std::wstring UserAgent = L"TestUserAgent";

			void TestEquivalence(const Boring32::WinHttp::Session& session1, Boring32::WinHttp::Session& session2)
			{
				Assert::IsTrue(session2.GetUserAgent() == session1.GetUserAgent());
				Assert::IsTrue(session2.GetProxyBypass() == session1.GetProxyBypass());
				Assert::IsTrue(session2.GetNamedProxy() == session1.GetNamedProxy());
				Assert::IsTrue(session2.GetProxyType() == session1.GetProxyType());
				Assert::IsTrue(session2.GetSession() == session1.GetSession());
				Assert::IsNotNull(session1.GetSession());
				Assert::IsNotNull(session2.GetSession());
			}

			void TestMove(const Boring32::WinHttp::Session& session1, Boring32::WinHttp::Session& session2)
			{
				Assert::IsTrue(session1.GetUserAgent().empty());
				Assert::IsTrue(session1.GetProxyBypass().empty());
				Assert::IsTrue(session1.GetNamedProxy().empty());
				Assert::IsTrue(session1.GetProxyType() == Boring32::WinHttp::ProxyType::NoProxy);
				Assert::IsNull(session1.GetSession());

				Assert::IsTrue(session2.GetUserAgent() == UserAgent);
				Assert::IsTrue(session2.GetProxyBypass().empty());
				Assert::IsTrue(session2.GetNamedProxy().empty());
				Assert::IsTrue(session2.GetProxyType() == Boring32::WinHttp::ProxyType::AutoProxy);
				Assert::IsNotNull(session2.GetSession());
			}

		// Can't really test the named proxy type case
		public:
			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::WinHttp::Session session;
				Assert::IsTrue(session.GetUserAgent().empty());
				Assert::IsTrue(session.GetProxyBypass().empty());
				Assert::IsTrue(session.GetNamedProxy().empty());
				Assert::IsTrue(session.GetProxyType() == Boring32::WinHttp::ProxyType::AutoProxy);
			}

			TEST_METHOD(TestConstructorUserAgent)
			{
				Boring32::WinHttp::Session session(UserAgent);
				Assert::IsTrue(session.GetUserAgent() == UserAgent);
				Assert::IsTrue(session.GetProxyBypass().empty());
				Assert::IsTrue(session.GetNamedProxy().empty());
				Assert::IsTrue(session.GetProxyType() == Boring32::WinHttp::ProxyType::AutoProxy);
				Assert::IsNotNull(session.GetSession());
			}

			TEST_METHOD(TestConstructorUserAgentNoProxyType)
			{
				Boring32::WinHttp::Session session(UserAgent, Boring32::WinHttp::ProxyType::NoProxy);
				Assert::IsTrue(session.GetUserAgent() == UserAgent);
				Assert::IsTrue(session.GetProxyBypass().empty());
				Assert::IsTrue(session.GetNamedProxy().empty());
				Assert::IsTrue(session.GetProxyType() == Boring32::WinHttp::ProxyType::NoProxy);
				Assert::IsNotNull(session.GetSession());
			}

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::WinHttp::Session session1(UserAgent, Boring32::WinHttp::ProxyType::NoProxy);
				Boring32::WinHttp::Session session2(session1);
				TestEquivalence(session1, session2);
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::WinHttp::Session session1(UserAgent, Boring32::WinHttp::ProxyType::NoProxy);
				Boring32::WinHttp::Session session2 = session1;
				TestEquivalence(session1, session2);
			}
			
			

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::WinHttp::Session session1(UserAgent, Boring32::WinHttp::ProxyType::AutoProxy);
				Boring32::WinHttp::Session session2(std::move(session1));
				TestMove(session1, session2);
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::WinHttp::Session session1(UserAgent, Boring32::WinHttp::ProxyType::AutoProxy);
				Boring32::WinHttp::Session session2 = std::move(session1);
				TestMove(session1, session2);
			}

			TEST_METHOD(TestConstructorUserAgentAutoProxyType)
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