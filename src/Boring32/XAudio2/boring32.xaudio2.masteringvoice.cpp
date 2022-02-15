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

	MasteringVoice::MasteringVoice(IXAudio2MasteringVoice* voice)
		: m_masteringVoice(voice)
	{
		if (!voice) throw Error::ErrorBase<std::runtime_error>(
			std::source_location::current(),
			"voice cannot be nullptr"
		);
	}

	void MasteringVoice::Close()
	{
		m_masteringVoice->DestroyVoice();
		m_masteringVoice = nullptr;
	}
}