export module boring32:datastructures_cappedstack;
import std;
import :error;

export namespace Boring32::DataStructures
{
	template<typename T, typename TContainer = std::deque<T>>
	struct CappedStack final
	{
		constexpr CappedStack(size_t maxSize, bool uniqueOnly)
		:	m_maxSize(maxSize),
			m_uniqueOnly(uniqueOnly)
		{
			if (m_maxSize == 0)
				throw Error::Boring32Error("maxSize is 0");
		}

		constexpr CappedStack& Push(const T value)
		{
			if (m_stack.empty())
			{
				m_stack.push_back(value);
				return *this;
			}
			if (m_uniqueOnly && value == m_stack.back())
				return *this;
			if (m_stack.size() == m_maxSize)
				PopFront();
				
			m_stack.push_back(value);
				
			return *this;
		}

		constexpr void PopFront()
		{
			if (m_stack.empty())
				return;
			if constexpr (requires (T t) { t.pop_front(); })
				m_stack.pop_front();
			else
				m_stack.erase(m_stack.begin());
		}

		constexpr T Pop()
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Cannot pop empty stack");
			T value = m_stack.back();
			m_stack.pop_back();
			return value;
		}

		constexpr bool Pop(T& value) noexcept
		{
			if (m_stack.empty())
				return false;
			value = Pop();
			return true;
		}

		constexpr bool PopLeaveOne() noexcept
		{
			if (m_stack.size() < 2)
				return false;
			Pop();
			return true;
		}

		constexpr bool PopLeaveOne(T& value) noexcept
		{
			if (m_stack.size() < 2)
				return false;
			return Pop(value);
		}

		constexpr T GetFirst()
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Cannot get from empty stack");
			return m_stack.front();
		}

		constexpr T GetCurrent()
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Cannot get from empty stack");
			return m_stack.back();
		}

		constexpr T GetFromBack(const size_t backIndex)
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Cannot get from empty stack");
			if((backIndex+1) >= m_stack.size())
				throw Error::Boring32Error("Invalid index");
			return m_stack.at(m_stack.size()-1-backIndex);
		}

		constexpr const TContainer& GetContainer() const noexcept
		{
			return m_stack;
		}

		constexpr bool operator==(const T val) const
		{
			if (m_stack.empty())
				return false;
			return m_stack.back() == val;
		}

		constexpr CappedStack& operator=(const T val)
		{
			return Push(val);
		}

		constexpr T operator[](const size_t index) const
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Stack is empty");
			return m_stack[index];
		}

		constexpr size_t GetMaxSize() const noexcept
		{
			return m_maxSize;
		}

		constexpr size_t GetSize() const noexcept
		{
			return m_stack.size();
		}

		constexpr bool IsEmpty() const noexcept
		{
			return m_stack.empty();
		}

		constexpr bool AddsUniqueOnly() const noexcept
		{
			return m_uniqueOnly;
		}

		private:
		size_t m_maxSize = 0;
		TContainer m_stack;
		bool m_uniqueOnly = false;
	};
}

static_assert(
	[]() consteval 
	{
		using TestStack = Boring32::DataStructures::CappedStack<int, std::vector<int>>;
		{
			// Test constructor
			TestStack stack(5, false);
			if (stack.GetMaxSize() != 5)
				throw std::runtime_error("GetMaxSize() expected to be 5");
			if (stack.AddsUniqueOnly())
				throw std::runtime_error("AddsUniqueOnly expected to be true.");
		}

		// Test assign push
		{
			TestStack stack(5, false);
			for (int i = 0; i < 5; i++)
				stack = i;
			if (stack.GetMaxSize() != 5)
				throw std::runtime_error("GetMaxSize() expected to be 5");
			for (int i = 0; i < 5; i++)
				if (stack[i] != i)
					throw std::runtime_error("Stack value did not match expected value.");
		}

		// Test assign unique push
		{
			TestStack stack(5, true);
			for (int i = 0; i < 5; i++)
				stack = 1;
			if (stack.GetSize() != 1)
				throw std::runtime_error("Stack size did not match expected value.");
			if (stack[0] != 1)
				throw std::runtime_error("Stack value [0] did not match expected value.");
		}

		// Test assign pop
		{
			TestStack stack(5, true);
			for (int i = 0; i < 5; i++)
				stack = i;
			if (stack.Pop() != 4)
				throw std::runtime_error("Pop() returned unexpected value");
			if (stack.GetSize() != 4)
				throw std::runtime_error("GetSize() returned unexpected value");
			for (int i = 0; i < 4; i++)
				if (stack[i] != i)
					throw std::runtime_error("stack returned unexpected value.");
		}

		// Test assign pop 2
		{
			TestStack stack(5, true);
			for (int i = 0; i < 5; i++)
				stack = i;
			int i = -1;
			if (not stack.Pop(i))
				throw std::runtime_error("Pop() expected to return true.");
			if (i != 4)
				throw std::runtime_error("i was epxected to be 4.");
			if (stack.GetSize() != 4)
				throw std::runtime_error("GetSize() was epxected to be 4.");

			for (int i = 0; i < 4; i++)
				if(stack[i] != i)
					throw std::runtime_error("stack returned an expected value.");
		}

		// Test assign equality
		{
			TestStack stack(5, true);
			for (int i = 0; i < 5; i++)
				stack = i;
			if (stack != 4)
				throw std::runtime_error("Expected sstack to be 4.");
		}

		return true;
	}()
);