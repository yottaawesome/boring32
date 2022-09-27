module boring32.security:policy;

namespace Boring32::Security
{
	Policy::~Policy() {}

	void Policy::Close()
	{
		m_handle.reset();
	}
}