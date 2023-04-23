module boring32.xaudio2:sourcevoice;
import boring32.error;
import :xaudio2error;
import <stdexcept>;

namespace Boring32::XAudio2
{
	SourceVoice::~SourceVoice()
	{
		Close();
	}
	
	SourceVoice::SourceVoice(SourceVoice&&) noexcept = default;
	SourceVoice& SourceVoice::operator=(SourceVoice&&) noexcept = default;

	SourceVoice::SourceVoice(IXAudio2SourceVoice* voice)
		: m_voice(voice)
	{
		if (!voice) 
			throw XAudio2Error("voice cannot be nullptr");
	}

	void SourceVoice::Close()
	{
		if (m_voice)
		{
			m_voice->DestroyVoice();
			m_voice = nullptr;
		}
	}
}