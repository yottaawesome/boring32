﻿#include "pch.h"
#include <string>
#include "CppUnitTest.h"

import boring32.services;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Services
{
	TEST_CLASS(ServiceControlManager)
	{
		public:
			TEST_METHOD(TestTokeniseString1)
			{
				Boring32::Services::ServiceControlManager scm(SC_MANAGER_CONNECT);
			}
	};
}