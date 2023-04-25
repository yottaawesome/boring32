export module boring32.util:switcher;

export namespace Boring32::Util
{
	class Switcher
	{
		public:
			virtual ~Switcher()
			{
				m_valueToSwitch = !m_valueToSwitch;
			}

			Switcher(bool& valueToSwitch)
				: m_valueToSwitch(valueToSwitch)
			{
				m_valueToSwitch = !m_valueToSwitch;
			};

		protected:
			bool m_valueToSwitch;
	};
}