#pragma comment(lib, "Cryptui.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Crypt32.lib")

export module boring32:crypto;
export import :crypto.aesencryption;
export import :crypto.certificate;
export import :crypto.certificatechain;
export import :crypto.chainingmode;
export import :crypto.certstore;
export import :crypto.cryptokey;
export import :crypto.functions;
export import :crypto.securestring;
export import :crypto.tempcertimport;
