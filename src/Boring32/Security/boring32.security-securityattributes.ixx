export module boring32.security:securityattributes;
import boring32.win32;

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
