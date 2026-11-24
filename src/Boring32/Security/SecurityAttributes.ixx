export module boring32:security.securityattributes;
import :win32;

export namespace Boring32::Security
{
	// TODO: not really clear if this is needed.
	class SecurityAttributes final
	{
	public:
		~SecurityAttributes()
		{
			Close();
		}

		void Close() noexcept
		{
		}

	private:
		Win32::SECURITY_ATTRIBUTES m_sa{0};
	};
}
