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
			virtual ImpersonationContext&
				operator=(const ImpersonationContext&) = delete;

			ImpersonationContext(ImpersonationContext&& other) noexcept;
			virtual ImpersonationContext& 
				operator=(ImpersonationContext&&) noexcept = delete;

		public:
			virtual bool Close();
			virtual HKEY GetUserRegistry();

		protected:
			virtual ImpersonationContext& 
				Move(ImpersonationContext& other) noexcept;

		protected:
			HKEY m_registryHive;
	};
}