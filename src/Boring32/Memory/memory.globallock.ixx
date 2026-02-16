export module boring32:memory.globallock;
import std;
import :win32;
import :error;

export namespace Boring32::Memory
{
	template <typename T>
	class TypedGlobalLock
	{
	public:
		TypedGlobalLock(const TypedGlobalLock&) = delete;
		TypedGlobalLock& operator=(const TypedGlobalLock&) = delete;
		TypedGlobalLock(TypedGlobalLock&&) noexcept = default;
		TypedGlobalLock& operator=(TypedGlobalLock&&) noexcept = default;

		constexpr ~TypedGlobalLock() noexcept
		{
			if (locked)
				if not consteval
				{
					Win32::GlobalUnlock(handle);
				}
			locked = nullptr;
		}

		constexpr TypedGlobalLock(T* handle)
			: handle{ handle }
			, locked{ 
				[](auto handle) constexpr -> T*
				{ 
					if consteval { return handle; }
					else { return reinterpret_cast<T*>(Win32::GlobalLock(handle)); }
				}(handle) 
			}
		{
			if (not locked)
				throw Error::Win32Error{ Win32::GetLastError(), "Failed to lock global memory." };
		}

		constexpr auto GetLocked(this const TypedGlobalLock& self) noexcept -> T*
		{
			return self.locked;
		}

		constexpr auto operator->(this const TypedGlobalLock& self) noexcept -> T*
		{
			return self.GetLocked();
		}

		constexpr auto GetHandle(this const TypedGlobalLock& self) noexcept -> Win32::HGLOBAL
		{
			return self.handle;
		}

	private:
		T* handle = nullptr;
		T* locked = nullptr;
	};

	// TODD: expand, add static_asserts to ensure that the class 
	// is constexpr-friendly, and that it can be used in a 
	// constexpr context.
	static_assert(
		[] -> bool
		{
			int x = 2;
			auto lock = TypedGlobalLock<int>{ &x };
			return lock.GetLocked()==&x;
		}()
	);
}
