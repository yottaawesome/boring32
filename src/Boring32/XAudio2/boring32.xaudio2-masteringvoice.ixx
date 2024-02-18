export module boring32.xaudio2:masteringvoice;
import boring32.shared;
import boring32.error;
import :xaudio2error;
import :voice;

export namespace Boring32::XAudio2
{
	// Not reference counted
	// https://docs.microsoft.com/en-us/windows/win32/api/xaudio2/nn-xaudio2-ixaudio2masteringvoice
	class MasteringVoice : public Voice
	{
		public:
			virtual ~MasteringVoice() = default;
			MasteringVoice(const MasteringVoice&) = delete;
			MasteringVoice(MasteringVoice&&) noexcept = default;
			MasteringVoice(Win32::IXAudio2MasteringVoice* voice)
				: m_voice(voice)
			{
				if (!voice)
					throw XAudio2Error("voice cannot be nullptr");
			}

		public:
			virtual MasteringVoice& operator=(const MasteringVoice&) = delete;
			virtual MasteringVoice& operator=(MasteringVoice&&) noexcept = default;

		public:
			virtual void Close() override
			{
				if (m_voice)
				{
					m_voice->DestroyVoice();
					m_voice = nullptr;
				}
			}

			virtual Win32::IXAudio2MasteringVoice* Get() const noexcept
			{
				return m_voice;
			}

		protected:
			Win32::IXAudio2MasteringVoice* m_voice;
	};
}