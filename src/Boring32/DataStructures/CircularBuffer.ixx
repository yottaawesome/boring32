export module boring32:datastructures.cappedstack;
import std;

export namespace Util
{
	template<typename T, unsigned N>
	struct CircularBuffer
	{
		static_assert(N > 0, "CircularBuffer capacity must be greater than zero.");

		// Once full, adding a value replaces the oldest entry.
		auto Add(const T& value) -> void
		{
			if (IsFull())
			{
				*m_data[m_front] = value;
				m_front = GetIndex(1);
			}
			else
			{
				m_data[GetIndex(m_size)].emplace(value);
				++m_size;
			}
		}

		auto PopFront() -> std::optional<T>
		{
			if (m_size == 0)
				return std::nullopt;

			auto value = std::move(m_data[m_front]);
			m_data[m_front].reset();
			m_front = GetIndex(1);
			--m_size;
			return value;
		}

		auto PopBack() -> std::optional<T>
		{
			if (m_size == 0)
				return std::nullopt;

			auto& back = m_data[GetIndex(m_size - 1)];
			auto value = std::move(back);
			back.reset();
			--m_size;
			return value;
		}

		auto IsFull() const noexcept -> bool
		{
			return m_size == N;
		}

		auto GetAll() const -> std::vector<T>
		{
			auto result = std::vector<T>{};
			result.reserve(m_size);
			for (std::size_t i = 0; i < m_size; ++i)
				result.push_back(*m_data[GetIndex(i)]);
			return result;
		}

		auto Clear() noexcept -> void
		{
			for (std::size_t i = 0; i < m_size; ++i)
				m_data[GetIndex(i)].reset();
			m_front = 0;
			m_size = 0;
		}

	private:
		auto GetIndex(std::size_t offset) const noexcept -> std::size_t
		{
			return (m_front + offset) % N;
		}

		std::array<std::optional<T>, N> m_data{};
		std::size_t m_front = 0;
		std::size_t m_size = 0;
	};
}
