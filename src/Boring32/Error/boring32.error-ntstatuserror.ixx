export module boring32.error:ntstatuserror;
import :boring32error;
import <string>;
import <stdexcept>;
import <source_location>;

export namespace Boring32::Error
{
	class NtStatusError : public Boring32Error
	{
		public:
			virtual ~NtStatusError() = default;
			NtStatusError(const NtStatusError& other) = default;
			NtStatusError(NtStatusError&& other) noexcept = default;
			NtStatusError(
				const std::string& msg,
				const std::source_location location = std::source_location::current()
			);
			NtStatusError(
				const std::string& msg,
				const long errorCode,
				const std::source_location location = std::source_location::current()
			);

		public:
			virtual NtStatusError& operator=(const NtStatusError& other) = default;
			virtual NtStatusError& operator=(NtStatusError&& other) noexcept = default;

		public:
			[[nodiscard]] virtual long GetErrorCode() const noexcept;

		protected:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			) override;

		protected:
			long m_errorCode = 0;
	};
}