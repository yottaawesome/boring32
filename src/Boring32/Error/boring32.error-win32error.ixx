export module boring32.error:win32error;
import <string>;
import <stdexcept>;
import <source_location>;
import :boring32error;

export namespace Boring32::Error
{
	class Win32Error : public Boring32Error
	{
		public:
			virtual ~Win32Error() = default;
			Win32Error(const Win32Error& other) = default;
			Win32Error(Win32Error&& other) noexcept = default;
			Win32Error(
				const std::string& msg,
				const std::source_location location = std::source_location::current()
			);
			Win32Error(
				const std::string& msg,
				const unsigned long errorCode,
				const std::source_location location = std::source_location::current()
			);
			Win32Error(
				const std::string& msg, 
				const unsigned long errorCode,
				const std::wstring& moduleName,
				const std::source_location location = std::source_location::current()
			);

		public:
			virtual Win32Error& operator=(const Win32Error& other) = default;
			virtual Win32Error& operator=(Win32Error&& other) noexcept = default;

		public:
			virtual unsigned long GetErrorCode() const noexcept;

		protected:
			unsigned long m_errorCode = 0;
	};
}