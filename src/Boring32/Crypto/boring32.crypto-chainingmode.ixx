export module boring32.crypto:chainingmode;
import std;

import <win32.hpp>;

export namespace Boring32::Crypto
{
	// May be better to merge these two into -functions.ixx or -aesencryption.ixx
	enum class ChainingMode
	{
		NotSet,
		CipherBlockChaining,
		CbcMac,
		CipherFeedback,
		ElectronicCodebook,
		GaloisCounterMode
	};

	const std::wstring& ChainingModeToString(const ChainingMode cm)
	{
		static std::map<ChainingMode, std::wstring> modes {
			{ ChainingMode::CipherBlockChaining,	BCRYPT_CHAIN_MODE_CBC },
			{ ChainingMode::CbcMac,					BCRYPT_CHAIN_MODE_CCM },
			{ ChainingMode::CipherFeedback,			BCRYPT_CHAIN_MODE_CFB },
			{ ChainingMode::ElectronicCodebook,		BCRYPT_CHAIN_MODE_ECB },
			{ ChainingMode::GaloisCounterMode,		BCRYPT_CHAIN_MODE_GCM }
		};
		return modes.at(cm);
	};
}