module boring32.security:securityattributes;

namespace Boring32::Security
{
	SecurityAttributes::~SecurityAttributes()
	{
		Close();
	}

	void SecurityAttributes::Close() noexcept
	{

	}
}
