module;

#include <map>
#include <string>
#include <Windows.h>
#include <bcrypt.h>

export module boring32.crypto.chainingmode;

export namespace Boring32::Crypto
{
	enum class ChainingMode
	{
		NotSet,
		CipherBlockChaining,
		CbcMac,
		CipherFeedback,
		ElectronicCodebook,
		GaloisCounterMode
	};

	extern const std::map<ChainingMode, std::wstring> ChainingModeString
	{
		{ChainingMode::CipherBlockChaining,	BCRYPT_CHAIN_MODE_CBC},
		{ChainingMode::CbcMac,				BCRYPT_CHAIN_MODE_CCM},
		{ChainingMode::CipherFeedback,		BCRYPT_CHAIN_MODE_CFB},
		{ChainingMode::ElectronicCodebook,	BCRYPT_CHAIN_MODE_ECB},
		{ChainingMode::GaloisCounterMode,	BCRYPT_CHAIN_MODE_GCM}
	};
}