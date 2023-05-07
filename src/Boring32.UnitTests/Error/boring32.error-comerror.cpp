#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.error;

namespace Error
{
	TEST_CLASS(COMError)
	{
		HRESULT m_errorCode = E_ACCESSDENIED;
		std::string m_errorMessage = "Some error message";

		public:
			TEST_METHOD(TestConstructor)
			{
				Boring32::Error::COMError error(m_errorMessage, m_errorCode);
				Assert::IsTrue(error.GetHResult() == m_errorCode);
				Assert::IsTrue(std::string(error.what()).contains(m_errorMessage));
			}
	};
}