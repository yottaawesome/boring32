#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.winhttp;

namespace Tests::WinHttp
{
	TEST_CLASS(Session)
	{
		public:
			TEST_METHOD(TestSessionDefaultConstructor)
			{
				Boring32::WinHttp::Session session;
			}
	};
}