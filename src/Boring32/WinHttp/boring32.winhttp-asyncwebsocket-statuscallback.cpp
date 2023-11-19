module boring32.winhttp:asyncwebsocket;
import boring32.strings;
import boring32.error;
import boring32.win32;
import <future>;
import <string>;
import <algorithm>;
import <source_location>;
import <map>;
import <iostream>;

namespace Boring32::WinHttp::WebSockets
{
	void Print(const DWORD dwInternetStatus)
	{
		//static const std::map<DWORD, std::wstring> InternetStatus{
		//{WINHTTP_CALLBACK_STATUS_RESOLVING_NAME, L"WINHTTP_CALLBACK_STATUS_RESOLVING_NAME"},
		//{WINHTTP_CALLBACK_STATUS_NAME_RESOLVED, L"WINHTTP_CALLBACK_STATUS_NAME_RESOLVED" },
		//{WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER, L"WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER"},
		//{WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER, L"WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER"},
		//{WINHTTP_CALLBACK_STATUS_SENDING_REQUEST, L"WINHTTP_CALLBACK_STATUS_SENDING_REQUEST"},
		//{WINHTTP_CALLBACK_STATUS_REQUEST_SENT, L"WINHTTP_CALLBACK_STATUS_REQUEST_SENT"},
		//{WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE, L"WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE"},
		//{WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED, L"WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED"},
		//{WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION, L"WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION"},
		//{WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED, L"WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED"},
		//{WINHTTP_CALLBACK_STATUS_HANDLE_CREATED, L"WINHTTP_CALLBACK_STATUS_HANDLE_CREATED"},
		//{WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING, L"WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING"},
		//{WINHTTP_CALLBACK_STATUS_DETECTING_PROXY, L"WINHTTP_CALLBACK_STATUS_DETECTING_PROXY"},
		//{WINHTTP_CALLBACK_STATUS_REDIRECT, L"WINHTTP_CALLBACK_STATUS_REDIRECT"},
		//{WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE, L"WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE"},
		//{WINHTTP_CALLBACK_STATUS_SECURE_FAILURE, L"WINHTTP_CALLBACK_STATUS_SECURE_FAILURE"},
		//{WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE, L"WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE"},
		//{WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE, L"WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE"},
		//{WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE, L"WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE"},
		//{WINHTTP_CALLBACK_STATUS_READ_COMPLETE, L"WINHTTP_CALLBACK_STATUS_READ_COMPLETE"},
		//{WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE, L"WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE"},
		//{WINHTTP_CALLBACK_STATUS_REQUEST_ERROR, L"WINHTTP_CALLBACK_STATUS_REQUEST_ERROR"},
		//{WINHTTP_CALLBACK_STATUS_GETPROXYFORURL_COMPLETE, L"WINHTTP_CALLBACK_STATUS_GETPROXYFORURL_COMPLETE"},
		//{WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE, L"WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE"},
		//{WINHTTP_CALLBACK_STATUS_SHUTDOWN_COMPLETE, L"WINHTTP_CALLBACK_STATUS_SHUTDOWN_COMPLETE"},
		//{WINHTTP_CALLBACK_STATUS_SETTINGS_WRITE_COMPLETE, L"WINHTTP_CALLBACK_STATUS_SETTINGS_WRITE_COMPLETE"},
		//{WINHTTP_CALLBACK_STATUS_SETTINGS_READ_COMPLETE, L"WINHTTP_CALLBACK_STATUS_SETTINGS_READ_COMPLETE"}
		//};

		//// Not sure why this fails
		//static bool firstTime = true;
		//static CRITICAL_SECTION cs;
		//if (firstTime)
		//{
		//	InitializeCriticalSection(&cs);
		//	firstTime = false;
		//}
		//Async::CriticalSectionLock csl(cs);
		//std::wcout << dwInternetStatus << std::endl;
		//std::wcout << InternetStatus.at(dwInternetStatus) << std::endl;
	}

	void AsyncWebSocket::StatusCallback(
		Win32::WinHttp::HINTERNET hInternet,
		Win32::DWORD_PTR dwContext,
		Win32::DWORD dwInternetStatus,
		Win32::LPVOID lpvStatusInformation,
		Win32::DWORD dwStatusInformationLength
	)
	{
		//std::wcout << L"AA " << GetCurrentThreadId() << std::endl;
		Print(dwInternetStatus);
		// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsetstatuscallback
		// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nc-winhttp-winhttp_status_callback
		switch (dwInternetStatus)
		{
			case Win32::WinHttp::_WINHTTP_CALLBACK_STATUS_REQUEST_SENT:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_REQUEST_SENT" << std::endl;
				
				auto socket = reinterpret_cast<AsyncWebSocket*>(dwContext);
				if(socket)
					std::wcout << L"WINHTTP_CALLBACK_STATUS_REQUEST_SENT: socket available" << std::endl;

				if (!Win32::WinHttp::WinHttpReceiveResponse(hInternet, 0))
				{
					const auto lastError = Win32::GetLastError();
					Error::Win32Error ex(
						"WinHttpReceiveResponse() failed on initial connection",
						lastError
					);

					std::wcout << ex.what() << std::endl;
				}

				break;
			}

			case Win32::WinHttp::_WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE" << std::endl;
				
				AsyncWebSocket* socket = (AsyncWebSocket*)dwContext;
				try
				{
					Win32::DWORD statusCode = 0;
					Win32::DWORD statusCodeSize = sizeof(statusCode);
					// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpqueryheaders
					const bool success = Win32::WinHttp::WinHttpQueryHeaders(
						hInternet,
						Win32::WinHttp::_WINHTTP_QUERY_STATUS_CODE | Win32::WinHttp::_WINHTTP_QUERY_FLAG_NUMBER,
						(Win32::LPCWSTR)Win32::WinHttp::_WINHTTP_HEADER_NAME_BY_INDEX,
						&statusCode,
						&statusCodeSize,
						(Win32::LPDWORD)Win32::WinHttp::_WINHTTP_NO_HEADER_INDEX
					);
					if (!success)
					{
						const auto lastError = Win32::GetLastError();
						throw Error::Win32Error("WinHttpQueryHeaders() failed", lastError);
					}

					switch (statusCode)
					{
						case 101: // Switching protocol
							break;

						case 301: // Redirect responses 
						case 302: 
						case 303:
						case 307:
						case 308:
							std::wcout << "Redirected" << std::endl;
							return;

						default: // Unexpected responses
							throw Error::Boring32Error(
								"Received unexpected HTTP response code while upgrading to websocket: "
								+ std::to_string(statusCode)
							);
					}

					socket->CompleteUpgrade();
				}
				catch (const std::exception& ex)
				{
					std::wcout << ex.what() << std::endl;
					if (socket)
					{
						socket->m_status = WebSocketStatus::Error;
						socket->m_connectionResult.IsConnected = false;
						socket->m_connectionResult.Complete.Signal(std::nothrow);
					}
				}
				break;
			}

			case Win32::WinHttp::_WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_READ_COMPLETE" << std::endl;
				AsyncWebSocket* socket = reinterpret_cast<AsyncWebSocket*>(dwContext);
				if (socket == nullptr)
				{
					std::wcerr << L"No socket found\n";
					return;
				}
				AsyncReadResult& read = socket->m_readResult;

				try
				{
					if (read.Status != ReadResultStatus::Initiated 
						&& read.Status != ReadResultStatus::PartialRead)
					{
						std::wcerr << L"read in an unexpected status" << std::endl;
						return;
					}

					auto status = static_cast<Win32::WinHttp::WINHTTP_WEB_SOCKET_STATUS*>(lpvStatusInformation);
					read.TotalBytesRead += status->dwBytesTransferred;
					switch (status->eBufferType)
					{
						case Win32::WinHttp::WINHTTP_WEB_SOCKET_BUFFER_TYPE::WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE:
						case Win32::WinHttp::WINHTTP_WEB_SOCKET_BUFFER_TYPE::WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE:
							read.Status = ReadResultStatus::PartialRead;
							socket->Receive(read);
							break;

						case Win32::WinHttp::WINHTTP_WEB_SOCKET_BUFFER_TYPE::WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE:
						case Win32::WinHttp::WINHTTP_WEB_SOCKET_BUFFER_TYPE::WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE:
							read.Status = ReadResultStatus::Finished;
							read.Data.resize(read.TotalBytesRead);
							read.Complete.Signal();
							break;

						case Win32::WinHttp::WINHTTP_WEB_SOCKET_BUFFER_TYPE::WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE:
							read.Status = ReadResultStatus::Finished;
							socket->m_status = WebSocketStatus::Closed;
							read.Complete.Signal();
							break;

						default:
							throw Error::Boring32Error("Unknown eBufferType");
					}
				}
				catch (const std::exception& ex)
				{
					socket->m_status = WebSocketStatus::Error;
					read.Complete.Signal(std::nothrow);
					std::wcerr << ex.what() << std::endl;
				}

				break;
			}

			case Win32::WinHttp::_WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE" << std::endl;
				AsyncWebSocket* socket = reinterpret_cast<AsyncWebSocket*>(dwContext);
				socket->m_writeResult.Status = WriteResultStatus::Finished;

				break;
			}

			case Win32::WinHttp::_WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_REQUEST_ERROR" << std::endl;
				
				Win32::WinHttp::WINHTTP_ASYNC_RESULT* requestError = (Win32::WinHttp::WINHTTP_ASYNC_RESULT*)lpvStatusInformation;
				
				AsyncWebSocket* socket = (AsyncWebSocket*)dwContext;
				socket->m_status = WebSocketStatus::Error;
				Error::Win32Error err("Error occurred in async socket callback", (DWORD)requestError->dwError);
				switch (requestError->dwResult)
				{
					case Win32::WinHttp::_API_RECEIVE_RESPONSE:
						std::wcerr << "The error occurred during a call to WinHttpReceiveResponse: " << err.what() << std::endl;
						break;

					case Win32::WinHttp::_API_QUERY_DATA_AVAILABLE:
						std::wcerr << "The error occurred during a call to WinHttpQueryDataAvailable: " << err.what() << std::endl;
						break;

					case Win32::WinHttp::_API_READ_DATA:
						std::wcerr << "The error occurred during a call to WinHttpReadData: " << err.what() << std::endl;
						break;

					case Win32::WinHttp::_API_WRITE_DATA:
						std::wcerr << "The error occurred during a call to WinHttpWriteData: " << err.what() << std::endl;
						break;

					case Win32::WinHttp::_API_SEND_REQUEST:
						std::wcerr << "The error occurred during a call to WinHttpSendRequest: " << err.what() << std::endl;
						break;

					default:
						std::wcerr << "Unknown error: " << requestError->dwResult << " " << err.what() << std::endl;
						break;
				}
				// Might be better to consolidate this into a single error event on the websocket level
				socket->m_connectionResult.IsConnected = false;
				socket->m_readResult.Status = ReadResultStatus::Error;
				socket->m_writeResult.Status = WriteResultStatus::Error;
				socket->m_connectionResult.Complete.Signal();
				socket->m_readResult.Complete.Signal();
				socket->m_writeResult.Complete.Signal();
				break;
			}

			case Win32::WinHttp::_WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE" << std::endl;
				AsyncWebSocket* socket = (AsyncWebSocket*)dwContext;
				socket->Release();
				socket->m_status = WebSocketStatus::Closed;

				break;
			}
		}
	}
}