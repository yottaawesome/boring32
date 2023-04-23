module boring32.xaudio2:masteringvoice;
import boring32.error;
import :xaudio2error;
import <stdexcept>;

namespace Boring32::XAudio2
{
	MasteringVoice::MasteringVoice(IXAudio2MasteringVoice* voice)
		: m_voice(voice)
	{
		if (!voice) 
			throw XAudio2Error("voice cannot be nullptr");
	}

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