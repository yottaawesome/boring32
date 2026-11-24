export module Boring32:XAudio2.SourceVoice;
import :Win32;
import :Error;
import :XAudio2.XAudio2Error;
import :XAudio2.Voice;

export namespace Boring32::XAudio2
{
	// https://docs.microsoft.com/en-us/windows/win32/api/xaudio2/nn-xaudio2-ixaudio2sourcevoice
	class SourceVoice final : Voice
	{
	public:
		~SourceVoice()
		{
			Close();
		}

		SourceVoice(const SourceVoice&) = delete;
		SourceVoice(SourceVoice&&) noexcept = default;
		SourceVoice(Win32::IXAudio2SourceVoice* voice)
			: m_voice(voice)
		{
			if (not voice)
				throw XAudio2Error{"voice cannot be nullptr"};
		}

		SourceVoice& operator=(const SourceVoice&) = delete;
		SourceVoice& operator=(SourceVoice&&) noexcept = default;

		void Close()
		{
			if (m_voice)
			{
				m_voice->DestroyVoice();
				m_voice = nullptr;
			}
		}

	private:
		Win32::IXAudio2SourceVoice* m_voice;
	};
}