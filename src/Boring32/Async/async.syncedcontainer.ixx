export module boring32:async.syncedcontainer;
import std;
import :win32;
import :error;
import :async.criticalsection;

export namespace Boring32::Async
{
	template<typename T>
	class SyncedContainer final
	{
	public:
		SyncedContainer()
			requires std::is_trivially_constructible<T>::value = default;

		template<typename...Args>
		SyncedContainer(Args... args)
			: m_protected(args...)
		{ }

		auto operator()(std::invocable<T> auto&& func)
		{
			auto cs = CriticalSectionLock(m_cs);

			// This checks if the passed in lambda corresponds to a signature of
			// void(T::reference) and calls the func for each element (effectively
			// a foreach). If the signature does not correspond to this, func
			// will be called with the list as the argument.
			// https://stackoverflow.com/questions/60791193/c-requires-expression-for-checking-function-signature-does-not-work-for-lambda
			// https://stackoverflow.com/questions/21657627/what-is-the-type-signature-of-a-c11-1y-lambda-function
			// https://stackoverflow.com/questions/24975147/check-if-class-has-function-with-signature
			// https://stackoverflow.com/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature
			// https://subscription.packtpub.com/book/application-development/9781787120495/1/ch01lvl1sec14/simplifying-compile-time-decisions-with-constexpr-if
			// https://stackoverflow.com/questions/257288/templated-check-for-the-existence-of-a-class-member-function
			// https://stackoverflow.com/questions/56910387/invoke-result-with-member-operator-function
			// https://stackoverflow.com/questions/52049841/how-to-use-stdinvoke-result-t-to-get-return-type-of-a-function
			// https://stackoverflow.com/questions/30756392/determining-return-type-of-stdfunction
			constexpr bool IsElementSignature = requires(typename T::reference element)
			{
				// All the below work
				//requires std::is_same_v<decltype(func(a)), void>;
				//std::is_same_v<decltype(func(a)), void>;
				//std::is_same_v<std::invoke_result<decltype(func), typename T::reference>::type, void>;
				{ func(element) }->std::same_as<void>;
			};
			if constexpr (IsElementSignature)
				for (auto& element : m_protected)
					func(element);
			else 
				return func(m_protected);
		}

		/*virtual void operator()(const std::function<void(typename T::reference)>& func)
		{
			CriticalSectionLock cs(m_cs);
			for(auto x : m_protected)
				func(x);
		}*/

		auto operator()(const size_t index, const auto func)
		{
			auto cs = CriticalSectionLock(m_cs);
			if (index >= m_protected.size())
				throw Error::Boring32Error("Invalid index");
			return func(m_protected[index]);
		}

		auto operator=(const T& other) -> SyncedContainer<T> requires std::is_copy_assignable<T>::value
		{
			auto cs = CriticalSectionLock(m_cs);
			m_protected = other;
			return *this;
		}

		auto operator=(T&& other) noexcept -> SyncedContainer<T> 
			requires std::is_move_assignable<T>::value
		{
			auto cs = CriticalSectionLock(m_cs);
			m_protected = other;
			return *this;
		}

		auto operator[](const size_t index) -> typename T::value_type 
			requires (std::copyable<typename T::value_type>)
		{
			auto cs = CriticalSectionLock(m_cs);
			if (index >= m_protected.size())
				throw Error::Boring32Error("Invalid index");
			return m_protected[index];
		}

		auto ForEach(std::invocable<const typename T::reference> auto&& func) -> SyncedContainer<T>
		{
			constexpr auto IsBool = std::is_same_v<std::invoke_result_t<decltype(func), const typename T::reference>, bool>;

			auto cs = CriticalSectionLock(m_cs);
			for (auto& x : m_protected)
			{
				if constexpr (IsBool)
				{
					if (not func(x))
						return *this;
				}
				else
				{
					func(x);
				}
			}
			return *this;
		}

		void PopBack()
		{
			auto cs = CriticalSectionLock(m_cs);
			if (m_protected.empty())
				return;
			m_protected.pop_back();
		}

		void PushBack(typename T::const_reference newValue)
		{
			auto cs = CriticalSectionLock(m_cs);
			m_protected.push_back(newValue);
		}

		void Clear()
		{
			auto cs = CriticalSectionLock(m_cs);
			m_protected.clear();
		}

		auto Size() -> size_t
		{
			auto cs = CriticalSectionLock(m_cs);
			return m_protected.size();
		}

		auto Empty() -> bool
		{
			auto cs = CriticalSectionLock(m_cs);
			return m_protected.empty();
		}

		void Delete(const size_t index)
		{
			auto cs = CriticalSectionLock(m_cs);
			if (index >= m_protected.size())
				return;
			m_protected.erase(m_protected.begin() + index);
		}

		[[nodiscard]]
		auto Remove(const size_t index) -> typename T::value_type requires (std::copyable<typename T::value_type>)
		{
			auto cs = CriticalSectionLock(m_cs);
			if (index >= m_protected.size())
				throw Error::Boring32Error("Invalid index");
			auto returnVal = m_protected[index];
			m_protected.erase(m_protected.begin() + index);
			return returnVal;
		}
			
	private:
		T m_protected;
		CriticalSection m_cs;
	};
}