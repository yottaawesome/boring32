module boring32.async:mutev;
import boring32.error;
import <stdexcept>;
import <format>;
import <string>;
import <win32.hpp>;

namespace Boring32::Async
{
	Mutev::~Mutev()
	{
		Close();
	}

	Mutev::Mutev()
	:	m_first(false, true, false, L""),
		m_second(false, true, false, L""),
		m_currentActive(Active::None)
	{ }
	
	Mutev::Mutev(const Active currentActive)
	:	m_first(false, true, false, L""),
		m_second(false, true, false, L""),
		m_currentActive(currentActive)
	{
		Set(m_currentActive);
	}

	Mutev::Mutev(const Mutev& other)
	{
		Copy(other);
	}
	
	Mutev& Mutev::operator=(const Mutev& other)
	{
		return Copy(other);
	}

	Mutev& Mutev::Copy(const Mutev& other)
	{
		if (this == &other)
			return *this;

		m_first = other.m_first;
		m_second = other.m_first;
		m_currentActive = other.m_currentActive;

		return *this;
	}

	Mutev::Mutev(Mutev&& other) noexcept
	{
		Move(other);
	}

	Mutev& Mutev::operator=(Mutev&& other) noexcept
	{
		return Move(other);
	}

	Mutev& Mutev::Move(Mutev& other) noexcept
	{
		m_first = std::move(other.m_first);
		m_second = std::move(other.m_first);
		m_currentActive = other.m_currentActive;

		return *this;
	}

	void Mutev::Close()
	{
		m_first.Close();
		m_second.Close();
		m_currentActive = Active::None;
	}

	void Mutev::Set(const Active currentActive)
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

	Mutev::Active Mutev::GetCurrentActive() const noexcept
	{
		return m_currentActive;
	}

	Mutev::Active Mutev::Switch()
	{
		if (m_currentActive == Active::None)
			Set(Active::First);
		if (m_currentActive == Active::First)
			Set(Active::Second);
		if (m_currentActive == Active::Second)
			Set(Active::First);
		return m_currentActive;
	}
}