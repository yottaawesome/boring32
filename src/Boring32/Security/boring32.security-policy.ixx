module;

#include <memory>
#include <Windows.h>
#include <Ntsecapi.h>

export module boring32.security:policy;

namespace Boring32::Security
{
	struct LSAHandleDeleter final
	{
		void operator()(LSA_HANDLE handle) const noexcept
		{
			LsaClose(handle);
		}
	};
	using LSAHandleUniquePtr = std::unique_ptr<std::remove_pointer<LSA_HANDLE>::type, LSAHandleDeleter>;
	using LSAHandleSharedPtr = std::shared_ptr<std::remove_pointer<LSA_HANDLE>::type>;
	LSAHandleSharedPtr CreateLSASharedPtr(LSA_HANDLE handle)
	{
		return LSAHandleSharedPtr(handle, LsaClose);
	}

	export class Policy
	{
		public:
			virtual ~Policy();
			Policy(
				ACCESS_MASK desiredAccess
			);

		public:
			virtual void Close();

		private:
			LSAHandleUniquePtr m_handle;
	};
}
