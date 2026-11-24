#pragma comment(lib, "Cryptui.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Crypt32.lib")

export module Boring32:Crypto;
export import :Crypto.AESEncryption;
export import :Crypto.Certificate;
export import :Crypto.CertificateChain;
export import :Crypto.ChainingMode;
export import :Crypto.CertStore;
export import :Crypto.CryptoKey;
export import :Crypto.Functions;
export import :Crypto.SecureString;
export import :Crypto.TempCertImport;
