export module boring32.security:constants;
import std;


export namespace Boring32::Security::Constants
{
	enum class GroupIntegrity
	{
		Low,
		Medium,
		MediumPlus,
		High,
		System,
		ProtectedProcess,
		SecureProcess
	};

	const std::wstring& GetIntegrity(const GroupIntegrity integrity)
	{
		//https://docs.microsoft.com/en-US/troubleshoot/windows-server/identity/security-identifiers-in-windows
		//https://docs.microsoft.com/en-us/windows/win32/secauthz/well-known-sids
		static const std::map<GroupIntegrity, std::wstring> Integrities{
			{GroupIntegrity::Low, L"S-1-16-4096"},
			{GroupIntegrity::Medium, L"S-1-16-8192"},
			{GroupIntegrity::MediumPlus, L"S-1-16-8448"},
			{GroupIntegrity::High, L"S-1-16-12288"},
			{GroupIntegrity::System, L"S-1-16-16384"},
			{GroupIntegrity::ProtectedProcess, L"S-1-16-20480"},
			{GroupIntegrity::SecureProcess, L"S-1-16-28672"}
		};

		return Integrities.at(integrity);
	}
}