#pragma comment(lib, "Ws2_32.lib")

export module boring32:winsock;
export import :winsock.functions;
export import :winsock.winsockerror;
export import :winsock.winsockinit;
export import :winsock.tcpsocket;
export import :winsock.socket;
export import :winsock.resolvedname;
