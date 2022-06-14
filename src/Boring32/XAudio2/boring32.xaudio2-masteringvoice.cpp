module;

#include <stdexcept>
#include <source_location>
#include <Windows.h>
#include <xaudio2.h>
#include <wrl/client.h>

module boring32.xaudio2:masteringvoice;
import boring32.error;
import :xaudio2error;

namespace Boring32::XAudio2
{
	MasteringVoice::~MasteringVoice()
	{
		Close();
	}

	MasteringVoice::MasteringVoice(MasteringVoice&&) noexcept = default;

	MasteringVoice::MasteringVoice(IXAudio2MasteringVoice* voice)
		: m_voice(voice)
	{
		if (!voice) 
			throw XAudio2Error("voice cannot be nullptr");
	}

	MasteringVoice& MasteringVoice::operator=(MasteringVoice&&) noexcept = default;

	void MasteringVoice::Close()
	{
		if (m_voice)
		{
			m_voice->DestroyVoice();
			m_voice = nullptr;
		}
	}

	IXAudio2MasteringVoice* MasteringVoice::Get() const noexcept
	{
		return m_voice;
	}
}