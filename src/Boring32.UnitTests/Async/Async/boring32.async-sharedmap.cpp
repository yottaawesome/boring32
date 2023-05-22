#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.async;

namespace Async
{
	TEST_CLASS(SharedMap)
	{
		public:
			TEST_METHOD(TestDefaultConstructor)
			{
				::Boring32::Async::SharedMap<std::wstring, int> map;
			}

			TEST_METHOD(TestSetValue)
			{
				::Boring32::Async::SharedMap<std::wstring, int> map;
				map.SetValue(L"Blah", 1);
			}

			TEST_METHOD(TestGetValue)
			{
				::Boring32::Async::SharedMap<std::wstring, int> map;
				map.SetValue(L"Blah", 1);
				Assert::IsTrue(map.GetValueCopy(L"Blah") == 1);
			}

			TEST_METHOD(TestExecuteOnValue)
			{
				::Boring32::Async::SharedMap<std::wstring, int> map;
				std::wstring key = L"Blah";
				map.SetValue(key, 1);
				map.ExecuteOnValue(
					key, 
					[](const int& value)
					{
						Assert::IsTrue(value == 1);
					});
			}
	};
}