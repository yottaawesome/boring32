#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Registry/RegistryKey.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Registry
{
	TEST_CLASS(RegKey)
	{
		public:
			TEST_METHOD(TestGetString)
			{
				Boring32::Registry::RegistryKey reg(
					HKEY_LOCAL_MACHINE,
					L"SOFTWARE\\Microsoft\\.NETFramework",
					KEY_READ
				);
				std::wstring out;
				reg.GetValue(L"InstallRoot", out);
				Assert::IsTrue(out == L"C:\\Windows\\Microsoft.NET\\Framework64\\");
			}
	};
}