export module boring32.async:mutev;
import boring32.shared;
import boring32.error;
import :event;

export namespace Boring32::Async
{
	// TODO: what is this even supposed to do again?
	class Mutev final
	{
		public:
			enum class Active : unsigned
			{
				None = 0,
				First = 1,
				Second = 2
			};

		public:
			~Mutev()
			{
				Close();
			}

			Mutev() = default;
			Mutev(const Mutev& other) = default;
			Mutev& operator=(const Mutev& other) = default;
			Mutev(Mutev&& other) noexcept = default;
			Mutev& operator=(Mutev&& other) noexcept = default;

			Mutev(const Active currentActive)
				: m_currentActive(currentActive)
			{
				Set(m_currentActive);
			}

		public:
			void Close()
			{
				m_first.Close();
				m_second.Close();
				m_currentActive = Active::None;
			}

			void Set(const Active currentActive)
			{
				switch (currentActive)
				{
					case Active::None:
						m_first.Reset();
						m_second.Reset();
						break;

					case Active::First:
						m_first.Signal();
						m_second.Reset();
						break;

					case Active::Second:
						m_first.Reset();
						m_second.Signal();
						break;

					default:
						throw Error::Boring32Error(
							std::format(
								"Unknown active value {}",
								static_cast<unsigned>(currentActive)
							)
						);
				}

				m_currentActive = currentActive;
			}

			Active GetCurrentActive() const noexcept
			{
				return m_currentActive;
			}

			Active Switch()
			{
				if (m_currentActive == Active::None)
					Set(Active::First);
				if (m_currentActive == Active::First)
					Set(Active::Second);
				if (m_currentActive == Active::Second)
					Set(Active::First);
				return m_currentActive;
			}

		private:
			Event m_first{ false, true, false };
			Event m_second{ false, true, false };
			Active m_currentActive = Active::None;
	};
}
