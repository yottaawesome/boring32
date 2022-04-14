module;

#include <string>
#include <tuple>
#include <Windows.h>

export module boring32.registry:value;

export namespace Boring32::Registry
{
	// https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-value-types
	enum class ValueTypes
	{
		Binary = REG_BINARY,
		DWord = REG_DWORD,
		DWordLittleEndian = REG_DWORD_LITTLE_ENDIAN,
		DWordBigEndian = REG_DWORD_BIG_ENDIAN,
		ExpandableString = REG_EXPAND_SZ,
		Link = REG_LINK,
		MultiString = REG_MULTI_SZ,
		None = REG_NONE,
		QWord = REG_QWORD,
		QWordLittleEndian = REG_QWORD_LITTLE_ENDIAN,
		String = REG_SZ,
	};

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