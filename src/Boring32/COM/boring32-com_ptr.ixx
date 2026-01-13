export module boring32:com_ptr;
import std;
import boring32.win32;

export namespace Boring32::COM
{
	template<typename T>
	struct Ptr
	{
		static constexpr Win32::GUID Uuid = __uuidof(T);

		using pointer = T*;

		constexpr ~Ptr() noexcept
		{
			reset();
		}

		constexpr Ptr() = default;

		constexpr Ptr(T* typePtr) noexcept
			: ptr(typePtr)
		{ }

		// Copyable
		constexpr Ptr(const Ptr& typePtr) noexcept
			: ptr(typePtr.ptr)
		{
			if (ptr)
				ptr->AddRef();
		}
		constexpr auto operator=(this Ptr& self, const Ptr& other) noexcept -> Ptr&
		{
			self.reset();
			self.ptr = other.ptr;
			self.ptr->AddRef();
			return self;
		}


		// Movable
		constexpr Ptr(Ptr&& other) noexcept
			: ptr(other.ptr)
		{
			other.ptr = nullptr;
		}
		constexpr auto operator=(this Ptr& self, Ptr&& other) noexcept -> Ptr&
		{
			self.reset();
			std::swap(self, other);
			self.swap(other);
			return self;
		}

		constexpr operator Win32::GUID(this const Ptr& self) noexcept
		{
			return self.Uuid;
		}

		constexpr operator bool(this const Ptr& self) noexcept
		{
			return self.ptr != nullptr;
		}

		constexpr auto operator==(this const Ptr& self, const Ptr& other) noexcept -> bool
		{
			return self.ptr == other.ptr;
		}

		constexpr auto operator*(this auto&& self) noexcept -> T*
		{
			return *self.ptr;
		}

		constexpr auto operator->(this auto&& self) noexcept -> T*
		{
			return self.ptr;
		}

		constexpr auto reset(this Ptr& self) noexcept -> Ptr&
		{
			if (self.ptr)
			{
				self.ptr->Release();
				self.ptr = nullptr;
			}
			return self;
		}

		constexpr auto detach(this Ptr& self) noexcept -> T*
		{
			T* temp = self.ptr;
			self.ptr = nullptr;
			return temp;
		}

		constexpr auto get(this const Ptr& self) noexcept -> T*
		{
			return self.ptr;
		}

		constexpr auto swap(this Ptr& self, Ptr& other) noexcept -> void
		{
			std::swap(self.ptr, other.ptr);
		}

		constexpr auto AddressOf(this Ptr& self) noexcept -> void**
		{
			return (void**)&self.ptr;
		}

		T* ptr = nullptr;
	};

	void F()
	{

	}
}
