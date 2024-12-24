export module boring32:datastructures_cappedstack;
import boring32.shared;
import :error;

export namespace Boring32::DataStructures
{
	template<typename T>
	struct CappedStack final
	{
		CappedStack(size_t maxSize, bool uniqueOnly)
		:	m_maxSize(maxSize),
			m_uniqueOnly(uniqueOnly)
		{
			if (m_maxSize == 0)
				throw Error::Boring32Error("maxSize is 0");
		}

		CappedStack& Push(const T value)
		{
			if (m_stack.empty())
			{
				m_stack.push_back(value);
				return *this;
			}
			if (m_uniqueOnly && value == m_stack.back())
				return *this;
			if (m_stack.size() == m_maxSize)
				m_stack.pop_front();
				
			m_stack.push_back(value);
				
			return *this;
		}

		T Pop()
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Cannot pop empty stack");
			T value = m_stack.back();
			m_stack.pop_back();
			return value;
		}

		bool Pop(T& value) noexcept
		{
			if (m_stack.empty())
				return false;
			value = Pop();
			return true;
		}

		bool PopLeaveOne() noexcept
		{
			if (m_stack.size() < 2)
				return false;
			Pop();
			return true;
		}

		bool PopLeaveOne(T& value) noexcept
		{
			if (m_stack.size() < 2)
				return false;
			return Pop(value);
		}

		T GetFirst()
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Cannot get from empty stack");
			return m_stack.front();
		}

		T GetCurrent()
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Cannot get from empty stack");
			return m_stack.back();
		}

		T GetFromBack(const size_t backIndex)
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Cannot get from empty stack");
			if((backIndex+1) >= m_stack.size())
				throw Error::Boring32Error("Invalid index");
			return m_stack.at(m_stack.size()-1-backIndex);
		}

		const std::deque<T>& GetContainer() const noexcept
		{
			return m_stack;
		}

		bool operator==(const T val) const
		{
			if (m_stack.empty())
				return false;
			return m_stack.back() == val;
		}

		CappedStack<T>& operator=(const T val)
		{
			return Push(val);
		}

		T operator[](const size_t index) const
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Stack is empty");
			return m_stack[index];
		}

		size_t GetMaxSize() const noexcept
		{
			return m_maxSize;
		}

		size_t GetSize() const noexcept
		{
			return m_stack.size();
		}

		bool IsEmpty() const noexcept
		{
			return m_stack.empty();
		}

		bool AddsUniqueOnly() const noexcept
		{
			return m_uniqueOnly;
		}

		private:
		size_t m_maxSize = 0;
		std::deque<T> m_stack;
		bool m_uniqueOnly = false;
	};
}
