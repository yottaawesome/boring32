export module boring32.async:mutev;
import <format>;
import <string>;
import boring32.error;
import :event;

export namespace Boring32::Async
{
	// TODO: what is this even supposed to do again?
	class Mutev
	{
		public:
			enum class Active : unsigned
			{
				None = 0,
				First = 1,
				Second = 2
			};

		public:
			virtual ~Mutev()
			{
				Close();
			}

			Mutev() = default;

			Mutev(const Active currentActive)
				: m_currentActive(currentActive)
			{
				Set(m_currentActive);
			}

			Mutev(const Mutev& other)
			{
				Copy(other);
			}

			virtual Mutev& operator=(const Mutev& other)
			{
				return Copy(other);
			}

			Mutev(Mutev&& other) noexcept
			{
				Move(other);
			}

			virtual Mutev& operator=(Mutev&& other) noexcept
			{
				return Move(other);
			}

		public:
			virtual void Close()
			{
				m_first.Close();
				m_second.Close();
				m_currentActive = Active::None;
			}

			virtual void Set(const Active currentActive)
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

			virtual Active GetCurrentActive() const noexcept
			{
				return m_currentActive;
			}

			virtual Active Switch()
			{
				if (m_currentActive == Active::None)
					Set(Active::First);
				if (m_currentActive == Active::First)
					Set(Active::Second);
				if (m_currentActive == Active::Second)
					Set(Active::First);
				return m_currentActive;
			}

		protected:
			virtual Mutev& Copy(const Mutev& other)
			{
				if (this == &other)
					return *this;

				m_first = other.m_first;
				m_second = other.m_first;
				m_currentActive = other.m_currentActive;

				return *this;
			}

			virtual Mutev& Move(Mutev& other) noexcept
			{
				m_first = std::move(other.m_first);
				m_second = std::move(other.m_first);
				m_currentActive = other.m_currentActive;

				return *this;
			}

		protected:
			Event m_first{ false, true, false };
			Event m_second{ false, true, false };
			Active m_currentActive = Active::None;
	};
}
