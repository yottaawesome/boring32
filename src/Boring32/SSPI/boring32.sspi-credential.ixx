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
	using CredentialUniquePtr = std::unique_ptr<std::remove_pointer<PCredHandle>::type, CredentialDeleter>;

	export class Credential
	{
		public:
			virtual ~Credential() = default;
			Credential();
			Credential(const Credential&) = delete;
			Credential(Credential&&) noexcept = default;

		public:
			virtual Credential& operator=(const Credential&) = delete;
			virtual Credential& operator=(Credential&&) noexcept = default;

		protected:
			void Create();

		protected:
			CredentialUniquePtr m_credHandle;

	};
}