module boring32.util.switcher;

namespace Boring32::Util
{
	Switcher::~Switcher()
	{
		m_valueToSwitch = !m_valueToSwitch;
	}

	Switcher::Switcher(bool& valueToSwitch) : m_valueToSwitch(valueToSwitch)
	{
		m_valueToSwitch = !m_valueToSwitch;
	};
}