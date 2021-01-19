#pragma once
#include <deque>
#include <algorithm>

namespace Boring32::DataStructures
{
	template<typename T>
	class CappedStack
	{
		public:
			virtual ~CappedStack() {}

			CappedStack(const size_t maxSize)
			:	m_maxSize(maxSize)
			{
				if (m_maxSize == 0)
					throw std::invalid_argument(__FUNCSIG__ ": maxSize is 0");
			}

			virtual CappedStack<T>& Push(const T value)
			{
				if (m_stack.size() == m_maxSize)
				{
					m_stack.pop_front();
				}
				m_stack.push_back(value);
				return *this;
			}

			virtual T Pop()
			{
				if (m_stack.empty())
					throw std::runtime_error(__FUNCSIG__ ": Cannot pop empty stack");
				T value = m_stack.back();
				m_stack.pop_back();
				return value;
			}

			virtual T GetFirst()
			{
				if (m_stack.empty())
					throw std::runtime_error(__FUNCSIG__ ": Cannot get from empty stack");
				return m_stack.front();
			}

			virtual T GetCurrent()
			{
				if (m_stack.empty())
					throw std::runtime_error(__FUNCSIG__ ": Cannot get from empty stack");
				return m_stack.back();
			}

			virtual const std::deque<T>& GetContainer() const noexcept
			{
				return m_stack;
			}

			virtual bool operator==(const T val) const
			{
				return m_stack.back() == val;
			}

			virtual CappedStack<T>& operator=(const T val)
			{
				return Push(val);
			}

		protected:
			size_t m_maxSize;
			std::deque<T> m_stack;
	};
}
