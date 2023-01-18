export module boring32.sspi:credential;
import <memory>;
import <win32.hpp>;

namespace Boring32::SSPI
{
	struct CredentialDeleter final
	{
		void operator()(PCredHandle handle)
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/sspi/nf-sspi-freecredentialshandle
			FreeCredentialsHandle(handle);
		}
	};
	using CredentialUniquePtr = std::unique_ptr<CredHandle, CredentialDeleter>;

	export class Credential
	{
		public:
			virtual ~Credential() = default;
			Credential(const Credential&) = delete;
			Credential(Credential&&) noexcept = default;
			Credential();

		public:
			virtual Credential& operator=(const Credential&) = delete;
			virtual Credential& operator=(Credential&&) noexcept = default;

		protected:
			virtual void Create();
			virtual void Create2();

		protected:
			CredentialUniquePtr m_credHandle;
	};
}