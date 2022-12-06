#include "pch.h"
#include "CppUnitTest.h"

import boring32.services;
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
	};
}