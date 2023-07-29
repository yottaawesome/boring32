#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.msi;

namespace MSI
{
	TEST_CLASS(Database)
	{
		public:
			TEST_METHOD(TestConstructor)
			{
				Boring32::MSI::Database(
					LR"(C:\Temp\test.msi)"
				);
			}
	};
}
