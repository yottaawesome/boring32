export module boring32:security_securityattributes;
import boring32.shared;

export namespace Boring32::Security
{
	class SecurityAttributes final
	{
		public:
			~SecurityAttributes()
			{
				Close();
			}

		public:
			void Close() noexcept
			{

			}

		protected:
			Win32::SECURITY_ATTRIBUTES m_sa{0};
	};
}
