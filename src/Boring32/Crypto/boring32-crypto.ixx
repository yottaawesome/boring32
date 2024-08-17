#pragma comment(lib, "Cryptui.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Crypt32.lib")

export module boring32:crypto;
export import :crypto_aesencryption;
export import :crypto_certificate;
export import :crypto_certificatechain;
export import :crypto_chainingmode;
export import :crypto_certstore;
export import :crypto_cryptokey;
export import :crypto_functions;
export import :crypto_securestring;
export import :crypto_tempcertimport;
