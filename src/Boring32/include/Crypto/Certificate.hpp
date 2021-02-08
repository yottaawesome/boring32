#pragma once
#include <Windows.h>
#include <wincrypt.h>

namespace Boring32::Crypto
{
	class Certificate
	{
		public:
			virtual ~Certificate();
			Certificate();
			Certificate(PCCERT_CONTEXT m_certContext);

			Certificate(const Certificate& other);
			virtual Certificate& operator=(const Certificate& other);

			Certificate(Certificate&& other) noexcept;
			virtual Certificate& operator=(Certificate&& other) noexcept;

			virtual bool operator==(PCCERT_CONTEXT const other) const noexcept;

		public:
			virtual void Close() noexcept;
			virtual PCCERT_CONTEXT GetCert() const noexcept;

		protected:
			virtual Certificate& Copy(const Certificate& other);
			virtual Certificate& Move(Certificate& other) noexcept;

		protected:
			PCCERT_CONTEXT m_certContext;
	};
}
