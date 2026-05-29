export module boring32:registry.value;
import std;
import :win32;
import :error;
import :strings;

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
	class Value final
	{
	public:
		Value(const Value& other) = default;
		Value(Value&& other) noexcept = default;
		Value(
			const std::wstring& path,
			const std::wstring& valueName,
			const T value
		) : m_path(path),
			m_valueName(valueName),
			m_value(std::move(value))
		{ }
			
		auto operator=(const Value& other) -> Value& = default;
		auto operator=(Value&& other) noexcept -> Value& = default;

		auto GetPath() const noexcept -> std::wstring { return m_path; }
		auto GetValueName() const noexcept -> std::wstring { return m_valueName; }
		auto GetValue() const noexcept -> const T& { return m_value; }

	protected:
		std::wstring m_path;
		std::wstring m_valueName;
		T m_value;
	};
}
