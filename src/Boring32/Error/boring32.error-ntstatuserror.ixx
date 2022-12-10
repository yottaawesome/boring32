module;

#include <source_location>

export module boring32.error:ntstatuserror;
import :boring32error;
import <string>;
import <stdexcept>;
import <win32.hpp>;

export namespace Boring32::Error
{
	class NtStatusError : public Boring32Error
	{
		public:
			virtual ~NtStatusError();
			NtStatusError(const NtStatusError& other);
			NtStatusError(NtStatusError&& other) noexcept;
			NtStatusError(
				const std::string& msg,
				const std::source_location location = std::source_location::current()
			);
			NtStatusError(
				const std::string& msg,
				const LONG errorCode,
				const std::source_location location = std::source_location::current()
			);

		public:
			virtual NtStatusError& operator=(const NtStatusError& other);
			virtual NtStatusError& operator=(NtStatusError&& other) noexcept;

		public:
			[[nodiscard]] virtual LONG GetErrorCode() const noexcept;

		protected:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			) override;

		protected:
			LONG m_errorCode;
	};
}