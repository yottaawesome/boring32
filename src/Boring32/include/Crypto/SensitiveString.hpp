#pragma once
#include <string>

namespace Boring32::Crypto
{
	class SensitiveString
	{
		public:
			virtual ~SensitiveString();
			SensitiveString();
			SensitiveString(std::wstring&& value);

		public:
			virtual void Clear();

		public:
			std::wstring Value;
	};
}