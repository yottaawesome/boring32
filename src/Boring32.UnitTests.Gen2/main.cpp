import std;
import boring32.unit_tests;

#pragma comment(lib, "wlanapi.lib")

int main()
{
    unit_tests::testing::run(
        unit_tests::registry::registry_value_tests{}
    );
    unit_tests::results::print_results();

    return 0;
}
