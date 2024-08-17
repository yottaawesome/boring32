export module boring32:async_syncedcontainer;
import std;
import std.compat;
import boring32.win32;
import :error;
import :async_criticalsectionlock;

export namespace Boring32::Async
{
	template<typename T>
	class SyncedContainer
	{
		public:
			virtual ~SyncedContainer()
			{
				Win32::DeleteCriticalSection(&m_cs);
			}

			SyncedContainer()
				requires std::is_trivially_constructible<T>::value
			{
				Win32::InitializeCriticalSection(&m_cs);
			}

			template<typename...Args>
			SyncedContainer(Args... args)
				: m_protected(args...)
			{
				Win32::InitializeCriticalSection(&m_cs);
			}

		public:
			auto operator()(const auto func)
			{
				CriticalSectionLock cs(m_cs);

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
				else return func(m_protected);
			}

			/*virtual void operator()(const std::function<void(typename T::reference)>& func)
			{
				CriticalSectionLock cs(m_cs);
				for(auto x : m_protected)
					func(x);
			}*/

			auto operator()(const size_t index, const auto func)
			{
				CriticalSectionLock cs(m_cs);
				if (index >= m_protected.size())
					throw Error::Boring32Error("Invalid index");
				return func(m_protected[index]);
			}

			SyncedContainer<T> operator=(const T& other)
			requires std::is_copy_assignable<T>::value
			{
				CriticalSectionLock cs(m_cs);
				m_protected = other;
				return *this;
			}

			SyncedContainer<T> operator=(T&& other) noexcept
			requires std::is_move_assignable<T>::value
			{
				CriticalSectionLock cs(m_cs);
				m_protected = other;
				return *this;
			}

			typename T::value_type operator[](const size_t index)
			requires (
				std::is_copy_constructible<typename T::value_type>::value 
				|| std::is_copy_assignable<typename T::value_type>::value
			)
			{
				CriticalSectionLock cs(m_cs);
				if (index >= m_protected.size())
					throw Error::Boring32Error("Invalid index");
				return m_protected[index];
			}

		public:
			virtual SyncedContainer<T> ForEach(
				const std::function<void(typename T::reference)>& func
			)
			{
				CriticalSectionLock cs(m_cs);
				for (auto& x : m_protected)
					func(x);
				return *this;
			}

			virtual SyncedContainer<T> ForEach(
				const std::function<bool(typename T::reference)>& func
			)
			{
				CriticalSectionLock cs(m_cs);
				for (auto& x : m_protected)
					if(!func(x))
						return *this;
				return *this;
			}

			virtual void PopBack()
			{
				CriticalSectionLock cs(m_cs);
				if (m_protected.empty())
					return;
				m_protected.pop_back();
			}

			virtual void PushBack(typename T::const_reference newValue)
			{
				CriticalSectionLock cs(m_cs);
				m_protected.push_back(newValue);
			}

			virtual void Clear()
			{
				CriticalSectionLock cs(m_cs);
				m_protected.clear();
			}

			virtual size_t Size()
			{
				CriticalSectionLock cs(m_cs);
				return m_protected.size();
			}

			virtual bool Empty()
			{
				CriticalSectionLock cs(m_cs);
				return m_protected.empty();
			}

			virtual void Delete(const size_t index)
			{
				CriticalSectionLock cs(m_cs);
				if (index >= m_protected.size())
					return;
				m_protected.erase(m_protected.begin() + index);
			}

			[[nodiscard]]
			typename T::value_type Remove(const size_t index)
			requires (
				std::is_copy_constructible<typename T::value_type>::value
				|| std::is_copy_assignable<typename T::value_type>::value
			)
			{
				CriticalSectionLock cs(m_cs);
				if (index >= m_protected.size())
					throw Error::Boring32Error("Invalid index");
				auto returnVal = m_protected[index];
				m_protected.erase(m_protected.begin() + index);
				return returnVal;
			}
			
		protected:
			T m_protected;
			Win32::CRITICAL_SECTION m_cs;
	};
}