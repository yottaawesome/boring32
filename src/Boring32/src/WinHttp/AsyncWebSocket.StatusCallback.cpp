#include "pch.hpp"
#include <future>
#include <algorithm>
#include "include/Strings/Strings.hpp"
#include "include/Error/Error.hpp"
#include "include/WinHttp/WebSockets/AsyncWebSocket.hpp"

namespace Boring32::WinHttp::WebSockets
{
	void AsyncWebSocket::StatusCallback(
		HINTERNET hInternet,
		DWORD_PTR dwContext,
		DWORD dwInternetStatus,
		LPVOID lpvStatusInformation,
		DWORD dwStatusInformationLength
	)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsetstatuscallback
		switch (dwInternetStatus)
		{
			case WINHTTP_CALLBACK_STATUS_RESOLVING_NAME:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_RESOLVING_NAME" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_NAME_RESOLVED:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_NAME_RESOLVED" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_SENDING_REQUEST:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_SENDING_REQUEST" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_REQUEST_SENT:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_REQUEST_SENT" << std::endl;
				
				auto socket = reinterpret_cast<AsyncWebSocket*>(dwContext);
				if(socket)
					std::wcout << L"WINHTTP_CALLBACK_STATUS_REQUEST_SENT: socket available" << std::endl;


				const bool success = WinHttpReceiveResponse(hInternet, 0);
				if (success == false)
				{
					Error::Win32Error ex(
						__FUNCSIG__ ": WinHttpReceiveResponse() failed on initial connection",
						GetLastError()
					);

					std::wcout << ex.what() << std::endl;
				}

				break;
			}

			case WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED" << std::endl;
				

				break;
			}

			case WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_HANDLE_CREATED:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_HANDLE_CREATED" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_DETECTING_PROXY:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_DETECTING_PROXY" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_REDIRECT:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_REDIRECT" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_SECURE_FAILURE" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE" << std::endl;
				
				try
				{
					DWORD statusCode = 0;
					DWORD statusCodeSize = sizeof(statusCode);
					// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpqueryheaders
					bool success = WinHttpQueryHeaders(
						hInternet,
						WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
						WINHTTP_HEADER_NAME_BY_INDEX,
						&statusCode,
						&statusCodeSize,
						WINHTTP_NO_HEADER_INDEX
					);
					if (success == false)
						throw Error::Win32Error(__FUNCSIG__ ": WinHttpQueryHeaders() failed", GetLastError());

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
							throw std::runtime_error(
								__FUNCSIG__
								": Received unexpected HTTP response code while upgrading to websocket: "
								+ std::to_string(statusCode)
							);
					}

					AsyncWebSocket* socket = (AsyncWebSocket*)dwContext;
					socket->m_winHttpWebSocket = WinHttpWebSocketCompleteUpgrade(
						socket->m_requestHandle.Get(),
						0
					);
					if (socket->m_winHttpWebSocket == nullptr)
						throw Error::Win32Error(
							__FUNCSIG__ ": WinHttpWebSocketCompleteUpgrade() failed",
							GetLastError()
						);

					bool succeeded = WinHttpSetOption(
						socket->m_winHttpWebSocket.Get(),
						WINHTTP_OPTION_CONTEXT_VALUE,
						(void*)&dwContext,
						sizeof(DWORD_PTR)
					);
					if (succeeded == false)
						throw Error::Win32Error(
							__FUNCSIG__ ": WinHttpSetOption() failed when setting context value",
							GetLastError()
						);

					socket->m_status = WebSocketStatus::Connected;
					socket->m_requestHandle = nullptr;
				}
				catch (const std::exception& ex)
				{
					std::wcout << ex.what() << std::endl;
				}
				break;
			}

			case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_READ_COMPLETE" << std::endl;
				
				try
				{
					AsyncWebSocket* socket = reinterpret_cast<AsyncWebSocket*>(dwContext);

					WebSocketReadResult& read = socket->m_currentResult;
					if (read.Status != WebSocketReadResultStatus::Initiated 
						&& read.Status != WebSocketReadResultStatus::PartialRead)
					{
						std::wcerr << L"read in an unexpected status" << std::endl;
						return;
					}

					auto status = static_cast<WINHTTP_WEB_SOCKET_STATUS*>(lpvStatusInformation);
					read.TotalBytesRead += status->dwBytesTransferred;
					switch (status->eBufferType)
					{
						case WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE:
						case WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE:
							read.Status = WebSocketReadResultStatus::PartialRead;
							socket->Receive(read);
							break;

						case WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE:
						case WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE:
							read.Status = WebSocketReadResultStatus::Finished;
							read.Data.resize(read.TotalBytesRead);
							read.Complete.Signal();
							break;

						case WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE:
							read.Status = WebSocketReadResultStatus::Finished;
							socket->m_status = WebSocketStatus::Closed;
							read.Complete.Signal();
							break;

						default:
							throw std::runtime_error("Unknown eBufferType");
					}
				}
				catch (const std::exception& ex)
				{
					std::wcerr << ex.what() << std::endl;
				}

				break;
			}

			case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE" << std::endl;
				
				break;
			}

			case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_REQUEST_ERROR" << std::endl;
				
				WINHTTP_ASYNC_RESULT* requestError = (WINHTTP_ASYNC_RESULT*)lpvStatusInformation;
				
				AsyncWebSocket* socket = (AsyncWebSocket*)dwContext;
				socket->m_status = WebSocketStatus::Error;
				Error::Win32Error err("", (DWORD)requestError->dwError);
				switch (requestError->dwResult)
				{
					case API_RECEIVE_RESPONSE:
						std::wcerr << "The error occurred during a call to WinHttpReceiveResponse: " << err.what() << std::endl;
						break;

					case API_QUERY_DATA_AVAILABLE:
						std::wcerr << "The error occurred during a call to WinHttpQueryDataAvailable: " << err.what() << std::endl;
						break;

					case API_READ_DATA:
						std::wcerr << "The error occurred during a call to WinHttpReadData: " << err.what() << std::endl;
						break;

					case API_WRITE_DATA:
						std::wcerr << "The error occurred during a call to WinHttpWriteData: " << err.what() << std::endl;
						break;

					case API_SEND_REQUEST:
						std::wcerr << "The error occurred during a call to WinHttpSendRequest: " << err.what() << std::endl;
						break;

					default:
						std::wcerr << "Unknown error: " << err.what() << std::endl;
						break;
				}
				socket->m_connectionResult.Complete.Signal();
				break;
			}

			case WINHTTP_CALLBACK_STATUS_GETPROXYFORURL_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_GETPROXYFORURL_COMPLETE" << std::endl;
				
				break;
			}
			case WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE:
			{
				
				std::wcout << L"WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE" << std::endl;
				AsyncWebSocket* socket = (AsyncWebSocket*)dwContext;
				socket->Release();
				socket->m_status = WebSocketStatus::Closed;

				break;
			}
			case WINHTTP_CALLBACK_STATUS_SHUTDOWN_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_SHUTDOWN_COMPLETE" << std::endl;
				

				break;
			}
			case WINHTTP_CALLBACK_STATUS_SETTINGS_WRITE_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_SETTINGS_WRITE_COMPLETE" << std::endl;
				
				break;
			}
			case WINHTTP_CALLBACK_STATUS_SETTINGS_READ_COMPLETE:
			{
				std::wcout << L"WINHTTP_CALLBACK_STATUS_SETTINGS_READ_COMPLETE" << std::endl;
				
				break;
			}

			default:
			{
				std::wcout << L"Default" << std::endl;
			}
		}
	}

}