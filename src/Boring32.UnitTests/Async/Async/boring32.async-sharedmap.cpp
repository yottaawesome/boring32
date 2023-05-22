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
	};
}