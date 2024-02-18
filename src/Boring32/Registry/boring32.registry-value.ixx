export module boring32.registry:value;
import boring32.shared;
import boring32.error;
import boring32.strings;

export namespace Boring32::Registry
{
	// https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-value-types
	using ValueTypes = Win32::Winreg::ValueTypes;

	template<ValueTypes V, typename T>
	struct KeyValuePair
	{
		ValueTypes V;
		using LanguageType = T;
	};
	template<>
	struct KeyValuePair<ValueTypes::String, std::string> { void Blah() {} };

	

	template<typename T>
	class Value
	{
		public:
			virtual ~Value() { }
			Value(const Value& other) = default;
			Value(Value&& other) noexcept = default;
			Value(
				const std::wstring& path,
				const std::wstring& valueName,
				const T value
			)
				: m_path(path),
				m_valueName(valueName),
				m_value(std::move(value))
			{
			}
			
		public:
			virtual Value& operator=(const Value& other) = default;
			virtual Value& operator=(Value&& other) noexcept = default;

		public:
			virtual const std::wstring& GetPath() const noexcept { return m_path; }
			virtual const std::wstring& GetValueName() const noexcept { return m_valueName; }
			virtual const T& GetValue() const noexcept { return m_value; }

		protected:
			std::wstring m_path;
			std::wstring m_valueName;
			T m_value;
	};

	
}