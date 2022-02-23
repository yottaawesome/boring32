module;

#include <string>
#include <stdexcept>
#include <source_location>
#include <Windows.h>

export module boring32.error.ntstatuserror;
import boring32.error.boring32error;

export namespace Boring32::Error
{
	class NtStatusError : public Boring32Error
	{
		public:
			virtual ~NtStatusError();
			NtStatusError(const NtStatusError& other);
			NtStatusError(NtStatusError&& other) noexcept;
			NtStatusError(
				const std::source_location& location, 
				const std::string& msg
			);
			NtStatusError(
				const std::source_location& location, 
				const std::string& msg, 
				const LONG errorCode
			);

		public:
			virtual NtStatusError& operator=(const NtStatusError& other);
			virtual NtStatusError& operator=(NtStatusError&& other) noexcept;

		public:
			[[nodiscard]] virtual LONG GetErrorCode() const noexcept;
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			) override;

		protected:
			LONG m_errorCode;
	};
}