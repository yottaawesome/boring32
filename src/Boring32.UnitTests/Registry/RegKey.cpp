#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Registry/RegKey.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Registry
{
	TEST_CLASS(RegKey)
	{
		public:
			TEST_METHOD(TestGetString)
			{
				Boring32::Registry::RegKey reg(
					HKEY_LOCAL_MACHINE,
					L"SOFTWARE\\Microsoft\\.NETFramework"
				);
				Assert::IsTrue(
					reg.GetString(L"InstallRoot") == L"C:\\Windows\\Microsoft.NET\\Framework64\\"
				);
			}
	};
}