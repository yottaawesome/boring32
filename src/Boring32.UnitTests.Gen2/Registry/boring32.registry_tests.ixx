export module boring32.unit_tests:registry_tests;
import boring32;
import boring32.win32;
import :framework;

using namespace Boring32;

export namespace unit_tests::registry
{
	struct registry_value_tests
	{
		auto tests() -> testing::testables auto
		{
			return unit_tests::testing::make_tests(
				unit_tests::testing::test{
					"Test DWORD throw",
					[] {
						try
						{
							using registry_value_t = Registry::RegistryValue<Win32::Winreg::_HKEY_LOCAL_MACHINE, L"A", L"A", Win32::Winreg::ValueTypes::DWord>;
							auto result = registry_value_t::Read();
							assert::is_true(false);
						}
						catch (...)
						{
							assert::is_true(true);
						}
					}
				},

				unit_tests::testing::test{
					"Test DWORD default return",
					[] {
						using registry_value_t = Registry::RegistryValue<Win32::Winreg::_HKEY_LOCAL_MACHINE, L"A", L"A", Win32::Winreg::ValueTypes::DWord>;
						auto result = registry_value_t::Read<false, [] { return 1; }>();
						assert::is_true(result == 1);
					}
				},

				unit_tests::testing::test{
					"Test QWORD throw",
					[] {
						try
						{
							using registry_value_t = Registry::RegistryValue<Win32::Winreg::_HKEY_LOCAL_MACHINE, L"A", L"A", Win32::Winreg::ValueTypes::QWord>;
							auto result = registry_value_t::Read();
							assert::is_true(false);
						}
						catch (...)
						{
							assert::is_true(true);
						}
					}
				},

				unit_tests::testing::test{
					"Test QWORD default return",
					[] {
						using registry_value_t = Registry::RegistryValue<Win32::Winreg::_HKEY_LOCAL_MACHINE, L"A", L"A", Win32::Winreg::ValueTypes::QWord>;
						auto result = registry_value_t::Read<false, [] { return 1; }>();
						assert::is_true(result == 1);
					}
				},

				unit_tests::testing::test{
					"Test wstring throw",
					[] {
						try
						{
							using registry_value_t = Registry::RegistryValue<Win32::Winreg::_HKEY_LOCAL_MACHINE, L"A", L"A", Win32::Winreg::ValueTypes::String>;
							auto result = registry_value_t::Read();
							assert::is_true(false);
						}
						catch (...)
						{
							assert::is_true(true);
						}
					}
				},

				unit_tests::testing::test{
					"Test wstring default return",
					[] {
						using registry_value_t = Registry::RegistryValue<Win32::Winreg::_HKEY_LOCAL_MACHINE, L"A", L"A", Win32::Winreg::ValueTypes::String>;
						auto result = registry_value_t::Read<false, [] { return L""; } >();
						assert::is_true(result == L"");
					}
				}
			);
		}
	};
}