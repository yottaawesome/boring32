export module boring32:com.ptr;
import std;
import :win32;
import :error;

export namespace Boring32::Com
{
	// An alternative to Microsoft::WRL::ComPtr with better constexpr support
	// and compatibility with std::out_ptr.
	template<typename T>
	class Ptr
	{
	public:
		using pointer = T*;
		struct Copy { T* ptr = nullptr; };
		struct Own { T* ptr = nullptr; };

		constexpr ~Ptr() noexcept
		{
			reset();
		}

		constexpr Ptr() = default;

		constexpr Ptr(T* t) noexcept
			: ptr(t)
		{}
		
		constexpr Ptr(Copy copy) noexcept
			: ptr(copy.ptr)
		{
			if (ptr)
				ptr->AddRef();
		}

		constexpr Ptr(Own own) noexcept
			: ptr(own.ptr)
		{ }

		// Copyable
		constexpr Ptr(const Ptr& typePtr) noexcept
			: ptr(typePtr.ptr)
		{
			if (ptr)
				ptr->AddRef();
		}
		constexpr auto operator=(const Ptr& other) noexcept -> Ptr&
		{
			if (this == &other)
				return *this;
			reset();
			ptr = other.ptr;
			if (ptr)
				ptr->AddRef();
			return *this;
		}

		// Movable
		constexpr Ptr(Ptr&& other) noexcept
			: ptr(std::exchange(other.ptr, nullptr))
		{ }
		constexpr auto operator=(Ptr&& other) noexcept -> Ptr&
		{
			if (this == &other)
				return *this;
			reset();
			ptr = std::exchange(other.ptr, nullptr);
			return *this;
		}

		constexpr operator Win32::GUID(this const Ptr& self) noexcept
		{
			return self.GetUuid();
		}

		constexpr operator bool(this const Ptr& self) noexcept
		{
			return self.ptr != nullptr;
		}

		constexpr auto operator=(std::nullptr_t) noexcept -> Ptr&
		{
			reset();
			return *this;
		}

		constexpr auto operator==(this const Ptr& self, const Ptr& other) noexcept -> bool
		{
			return self.ptr == other.ptr;
		}

		constexpr auto operator*(this auto&& self) noexcept -> T*
		{
			return self.ptr;
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

		constexpr void reset(this Ptr& self, T* ptr)
		{
			self.reset();
			self.ptr = ptr;
		}

		constexpr auto detach(this Ptr& self) noexcept -> T*
		{
			return std::exchange(self.ptr, nullptr);
		}

		constexpr auto get(this const Ptr& self) noexcept -> T*
		{
			return self.ptr;
		}

		constexpr void swap(this Ptr& self, Ptr& other) noexcept
		{
			self.ptr = std::exchange(other.ptr, self.ptr);
		}

		constexpr auto AddressOf(this Ptr& self) noexcept -> void**
		{
			return reinterpret_cast<void**>(&self.ptr);
		}

		constexpr auto ReleaseAndGetAddressOf(this Ptr& self) noexcept -> void**
		{
			self.reset();
			return reinterpret_cast<void**>(&self.ptr);
		}

		constexpr auto ReleaseAndGetAddressOfTyped(this Ptr& self) noexcept -> T**
		{
			self.reset();
			return &self.ptr;
		}

		constexpr auto GetUuid(this const Ptr& self) noexcept -> Win32::GUID
		{
			return __uuidof(T);
		}

		template<typename T>
		constexpr auto TryAs(this const Ptr& self) noexcept -> std::expected<Ptr<T>, Win32::HRESULT>
		{
			if (not self.ptr)
				return Ptr<T>{};
			auto result = Ptr<T>{};
			auto hr = self.ptr->QueryInterface(__uuidof(T), reinterpret_cast<void**>(&result.ptr));
			return Win32::HrSucceeded(hr) ? result : std::unexpected{ hr };
		}

		template<typename T>
		constexpr auto As(this const Ptr& self) -> Ptr<T>
		{
			auto result = self.TryAs<T>();
			return result ? *result : throw Error::COMError{ result.error(), "QueryInterface() failed" };
		}

	private:
		T* ptr = nullptr;
	};
}
