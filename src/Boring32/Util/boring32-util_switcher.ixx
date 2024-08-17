export module boring32:util_switcher;

export namespace Boring32::Util
{
	class Switcher final // TODO: What is the purpose of this class? I don't remember.
	{
		public:
			~Switcher()
			{
				m_valueToSwitch = !m_valueToSwitch;
			}

			Switcher(bool& valueToSwitch)
				: m_valueToSwitch(valueToSwitch)
			{
				m_valueToSwitch = !m_valueToSwitch;
			};

		private:
			bool m_valueToSwitch;
	};
}