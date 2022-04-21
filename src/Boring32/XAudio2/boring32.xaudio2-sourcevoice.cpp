module;

#include <stdexcept>
#include <source_location>
#include <Windows.h>
#include <xaudio2.h>

module boring32.xaudio2:sourcevoice;
import boring32.error;

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
		if (!voice) throw Error::ErrorBase<std::runtime_error>(
			std::source_location::current(),
			"voice cannot be nullptr"
		);
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