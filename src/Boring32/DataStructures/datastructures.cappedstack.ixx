export module boring32:datastructures.cappedstack;
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

		constexpr auto Push(const T value) -> CappedStack&
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

		constexpr auto Pop() -> T
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Cannot pop empty stack");
			T value = m_stack.back();
			m_stack.pop_back();
			return value;
		}

		constexpr auto Pop(T& value) noexcept -> bool
		{
			if (m_stack.empty())
				return false;
			value = Pop();
			return true;
		}

		constexpr auto PopLeaveOne() noexcept -> bool
		{
			if (m_stack.size() < 2)
				return false;
			Pop();
			return true;
		}

		constexpr auto PopLeaveOne(T& value) noexcept -> bool
		{
			if (m_stack.size() < 2)
				return false;
			return Pop(value);
		}

		constexpr auto GetFirst() -> T
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Cannot get from empty stack");
			return m_stack.front();
		}

		constexpr auto GetCurrent() -> T
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Cannot get from empty stack");
			return m_stack.back();
		}

		constexpr auto GetFromBack(const size_t backIndex) -> T
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Cannot get from empty stack");
			if((backIndex+1) >= m_stack.size())
				throw Error::Boring32Error("Invalid index");
			return m_stack.at(m_stack.size()-1-backIndex);
		}

		constexpr auto GetContainer() const noexcept -> const TContainer&
		{
			return m_stack;
		}

		constexpr auto operator==(const T val) const -> bool
		{
			if (m_stack.empty())
				return false;
			return m_stack.back() == val;
		}

		constexpr auto operator=(const T val) -> CappedStack&
		{
			return Push(val);
		}

		constexpr auto operator[](const size_t index) const -> T
		{
			if (m_stack.empty())
				throw Error::Boring32Error("Stack is empty");
			return m_stack[index];
		}

		constexpr auto GetMaxSize() const noexcept -> size_t
		{
			return m_maxSize;
		}

		constexpr auto GetSize() const noexcept -> size_t
		{
			return m_stack.size();
		}

		constexpr auto IsEmpty() const noexcept -> bool
		{
			return m_stack.empty();
		}

		constexpr auto AddsUniqueOnly() const noexcept -> bool
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