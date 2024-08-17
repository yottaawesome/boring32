export module boring32.unit_tests:strings_tests;
import boring32;
import boring32.win32;
import :framework;

using namespace Boring32;

export namespace unit_tests::strings
{
	struct strings_tests
	{
		auto tests() -> testing::testables auto
		{
			return unit_tests::testing::make_tests(
				unit_tests::testing::test{
					"Test wchar_t* to std::string",
					[] {
						const wchar_t* test = L"Test";
						auto value = Boring32::Strings::To<std::string>(test);
						assert::is_true(std::same_as<decltype(value), std::string>);
						assert::is_true(value == "Test");
					}
				},
				unit_tests::testing::test{
					"Test std::wstring_view to std::string",
					[] {
						std::wstring_view test = L"Test";
						auto value = Boring32::Strings::To<std::string>(test);
						assert::is_true(std::same_as<decltype(value), std::string>);
						assert::is_true(value == "Test");
					}
				},
				unit_tests::testing::test{
					"Test std::wstring to std::string",
					[] {
						std::wstring test{L"Test"};
						auto value = Boring32::Strings::To<std::string>(test);
						assert::is_true(std::same_as<decltype(value), std::string>);
						assert::is_true(value == "Test");
					}
				},

				unit_tests::testing::test{
					"Test char* to std::wstring",
					[] {
						const char* test = "Test";
						auto value = Boring32::Strings::To<std::wstring>(test);
						assert::is_true(std::same_as<decltype(value), std::wstring>);
						assert::is_true(value == L"Test");
					}
				},
				unit_tests::testing::test{
					"Test string_view to std::wstring",
					[] {
						std::string_view test = "Test";
						auto value = Boring32::Strings::To<std::wstring>(test);
						assert::is_true(std::same_as<decltype(value), std::wstring>);
						assert::is_true(value == L"Test");
					}
				},
				unit_tests::testing::test{
					"Test std::string to std::wstring",
					[] {
						std::string test = "Test";
						auto value = Boring32::Strings::To<std::wstring>(test);
						assert::is_true(std::same_as<decltype(value), std::wstring>);
						assert::is_true(value == L"Test");
					}
				},
				unit_tests::testing::test{
					"Test AutoAnsi",
					[] {
						constexpr auto f = [](const Boring32::Strings::AutoAnsi& s)
						{
							return s.Value;
						};

						assert::is_true(f(L"aaaa") == "aaaa");
					}
				},
				unit_tests::testing::test{
					"Test AutoWide",
					[] {
						constexpr auto f = [](const Boring32::Strings::AutoWide& s)
						{
							return s.Value;
						};
						Boring32::Strings::AutoWide s;
						assert::is_true(f("aaaa") == L"aaaa");
					}
				}
			);
		}
	};
}