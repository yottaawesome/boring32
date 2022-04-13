export module boring32.util:switcher;

export namespace Boring32::Util
{
	class Switcher
	{
		public:
			virtual ~Switcher();
			Switcher(bool& valueToSwitch);

		protected:
			bool m_valueToSwitch;
	};
}