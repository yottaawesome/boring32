export module boring32:xaudio2_masteringvoice;
import boring32.win32;
import :error;
import :xaudio2_xaudio2error;
import :xaudio2_voice;

export namespace Boring32::XAudio2
{
	// Not reference counted
	// https://docs.microsoft.com/en-us/windows/win32/api/xaudio2/nn-xaudio2-ixaudio2masteringvoice
	struct MasteringVoice final : Voice
	{
		MasteringVoice(const MasteringVoice&) = delete;
		MasteringVoice(MasteringVoice&&) noexcept = default;
		MasteringVoice(Win32::IXAudio2MasteringVoice* voice)
			: m_voice(voice)
		{
			if (!voice)
				throw XAudio2Error("voice cannot be nullptr");
		}

		MasteringVoice& operator=(const MasteringVoice&) = delete;
		MasteringVoice& operator=(MasteringVoice&&) noexcept = default;

		void Close() override
		{
			if (m_voice)
			{
				m_voice->DestroyVoice();
				m_voice = nullptr;
			}
		}

		Win32::IXAudio2MasteringVoice* Get() const noexcept
		{
			return m_voice;
		}

		private:
		Win32::IXAudio2MasteringVoice* m_voice;
	};
}