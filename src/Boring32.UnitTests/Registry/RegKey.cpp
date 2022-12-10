#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.registry;

namespace Registry
{
	TEST_CLASS(RegKey)
	{
		public:
			TEST_METHOD(TestGetString)
			{
				Boring32::Registry::Key reg(
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