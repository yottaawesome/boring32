export module boring32.security:securityattributes;

export namespace Boring32::Security
{
	class SecurityAttributes
	{
		public:
			virtual ~SecurityAttributes();

		public:
			virtual void Close() noexcept;
	};
}
