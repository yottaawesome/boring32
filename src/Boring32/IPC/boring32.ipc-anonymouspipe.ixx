export module boring32.ipc:anonymouspipe;
import boring32.raii;
import <string>;
import <vector>;
import <win32.hpp>;

export namespace Boring32::IPC
{
	class AnonymousPipe
	{
		// Constructors
		public:
			virtual ~AnonymousPipe();
			AnonymousPipe();
			AnonymousPipe(const AnonymousPipe& other);
			AnonymousPipe(AnonymousPipe&& other) noexcept;
			AnonymousPipe(
				const bool inheritable,
				const DWORD size,
				const std::wstring& delimiter
			);
			AnonymousPipe(
				const DWORD size,
				const std::wstring& delimiter,
				const HANDLE readHandle,
				const HANDLE writeHandle
			);

		public:
			virtual AnonymousPipe& operator=(const AnonymousPipe& other);
			virtual AnonymousPipe& operator=(AnonymousPipe&& other) noexcept;

			// API
		public:
			virtual void Write(const std::wstring& msg);
			virtual void DelimitedWrite(const std::wstring& msg);
			virtual std::wstring Read();
			virtual std::vector<std::wstring> DelimitedRead();
			virtual void CloseRead();
			virtual void CloseWrite();
			virtual void SetMode(const DWORD mode);
			virtual HANDLE GetRead() const noexcept;
			virtual HANDLE GetWrite() const noexcept;
			virtual std::wstring GetDelimiter() const;
			virtual DWORD GetSize() const;
			virtual DWORD GetUsedSize() const;
			virtual DWORD GetRemainingSize() const;
			virtual void Close();

			// Internal variables
		protected:
			std::wstring m_delimiter;
			DWORD m_size = 0;
			DWORD m_mode = 0;
			RAII::Win32Handle m_readHandle;
			RAII::Win32Handle m_writeHandle;
	};
}