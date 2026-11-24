#pragma comment(lib, "Ws2_32.lib")

// TODO: rework nested exceptions, promote WinSockError to a full error type.

export module Boring32:WinSock;
export import :WinSock.Functions;
export import :WinSock.WinSockError;
export import :WinSock.WinSockInit;
export import :WinSock.TCPSocket;
export import :WinSock.Socket;
export import :WinSock.ResolvedName;
