module;

#include <string>
#include <stdexcept>
#include <source_location>
#include <Windows.h>

export module boring32.error.ntstatuserror;

export namespace Boring32::Error
{
	class NtStatusError : public std::runtime_error
	{
		public:
			virtual ~NtStatusError();
			NtStatusError(const char* msg, const LONG errorCode);
			NtStatusError(const std::string& msg, const LONG errorCode);
			NtStatusError(const std::source_location& location, const std::string& msg, const LONG errorCode);

		public:
			virtual LONG GetErrorCode() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			LONG m_errorCode;
			std::string m_errorString;
	};
}