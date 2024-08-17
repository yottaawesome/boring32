export module boring32:crypto_chainingmode;
import boring32.shared;

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
			{ ChainingMode::CipherBlockChaining,	Win32::BCryptChainingMode::CipherBlockChaining },
			{ ChainingMode::CbcMac,					Win32::BCryptChainingMode::CbcMac },
			{ ChainingMode::CipherFeedback,			Win32::BCryptChainingMode::CipherFeedback },
			{ ChainingMode::ElectronicCodebook,		Win32::BCryptChainingMode::ElectronicCodebook },
			{ ChainingMode::GaloisCounterMode,		Win32::BCryptChainingMode::GaloisCounterMode }
		};
		return modes.at(cm);
	};
}