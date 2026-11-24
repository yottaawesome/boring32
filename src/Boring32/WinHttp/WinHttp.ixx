#pragma comment(lib, "winhttp.lib")

export module Boring32:WinHttp;
export import :WinHttp.HttpWebClient;
export import :WinHttp.ProxyInfo;
export import :WinHttp.Enums;
export import :WinHttp.RequestResult;
export import :WinHttp.Session;
export import :WinHttp.AsyncWebSocket;
export import :WinHttp.WebSocket;
export import :WinHttp.AsyncWebSocketSettings;
export import :WinHttp.WebSocketSettings;
export import :WinHttp.WinHttpError;
export import :WinHttp.WinHttpHandle;
export import :WinHttp.Handles;
export import :WinHttp.DecomposedURL;
