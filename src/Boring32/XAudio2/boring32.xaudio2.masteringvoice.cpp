module;

#include <stdexcept>
#include <source_location>
#include <Windows.h>
#include <xaudio2.h>
#include <wrl/client.h>

module boring32.xaudio2.masteringvoice;
import boring32.error.errorbase;

namespace Boring32::XAudio2
{
	MasteringVoice::~MasteringVoice()
	{
		Close();
	}

	MasteringVoice::MasteringVoice(MasteringVoice&&) noexcept = default;

	MasteringVoice::MasteringVoice(IXAudio2MasteringVoice* voice)
		: m_masteringVoice(voice)
	{
		if (!voice) throw Error::ErrorBase<std::runtime_error>(
			std::source_location::current(),
			"voice cannot be nullptr"
		);
	}

	MasteringVoice& MasteringVoice::operator=(MasteringVoice&&) noexcept = default;

	void MasteringVoice::Close()
	{
		m_masteringVoice->DestroyVoice();
		m_masteringVoice = nullptr;
	}

	IXAudio2MasteringVoice* MasteringVoice::Get() const noexcept
	{
		return m_masteringVoice;
	}
}