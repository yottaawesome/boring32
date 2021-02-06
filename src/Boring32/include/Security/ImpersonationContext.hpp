#pragma once
#include <Windows.h>

namespace Boring32::Security
{
	class ImpersonationContext
	{
		public:
			virtual ~ImpersonationContext();
			ImpersonationContext(HANDLE const token);

			ImpersonationContext(const ImpersonationContext&) = delete;
			virtual ImpersonationContext& operator=(const ImpersonationContext&) = delete;

			ImpersonationContext(ImpersonationContext&&) = delete;
			virtual ImpersonationContext& operator=(ImpersonationContext&&) = delete;

		public:
			virtual bool Close();
	};
}