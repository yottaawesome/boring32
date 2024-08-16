#include "pch.h"

import boring32;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Services
{
	TEST_CLASS(ServiceControlManager)
	{
		public:
			TEST_METHOD(TestConstructor)
			{
				Boring32::Services::ServiceControlManager scm(SC_MANAGER_CONNECT);
				Assert::IsNotNull(scm.GetHandle());
			}

			TEST_METHOD(TestClose)
			{
				Boring32::Services::ServiceControlManager scm(SC_MANAGER_CONNECT);
				scm.Close();
				Assert::IsNull(scm.GetHandle());
			}

			TEST_METHOD(TestOperatorBool)
			{
				Boring32::Services::ServiceControlManager scm(SC_MANAGER_CONNECT);
				Assert::IsTrue(scm);
			}

			TEST_METHOD(TestGetService)
			{
				Boring32::Services::ServiceControlManager scm(SC_MANAGER_CONNECT);
				Boring32::Services::Service s = scm.AccessService(
					L"DcomLaunch",
					SERVICE_QUERY_STATUS
				);
				Assert::IsNotNull(s.GetHandle());
			}

			TEST_METHOD(TestMoveConstruction)
			{
				Boring32::Services::ServiceControlManager scm1(
					SC_MANAGER_CONNECT
				);
				Boring32::Services::ServiceControlManager scm2(std::move(scm1));
				Assert::IsNull(scm1.GetHandle());
				Assert::IsNotNull(scm2.GetHandle());
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::Services::ServiceControlManager scm1(
					SC_MANAGER_CONNECT
				);
				Boring32::Services::ServiceControlManager scm2
					= std::move(scm1);
				Assert::IsNull(scm1.GetHandle());
				Assert::IsNotNull(scm2.GetHandle());
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::Services::ServiceControlManager scm1(
					SC_MANAGER_CONNECT
				);
				Boring32::Services::ServiceControlManager scm2 = scm1;
				Assert::IsNotNull(scm1.GetHandle());
				Assert::IsNotNull(scm2.GetHandle());
				Assert::IsTrue(scm1.GetHandle() == scm2.GetHandle());
			}

			TEST_METHOD(TestCopyConstruction)
			{
				Boring32::Services::ServiceControlManager scm1(
					SC_MANAGER_CONNECT
				);
				Boring32::Services::ServiceControlManager scm2(scm1);
				Assert::IsNotNull(scm1.GetHandle());
				Assert::IsNotNull(scm2.GetHandle());
				Assert::IsTrue(scm1.GetHandle() == scm2.GetHandle());
			}
	};
}