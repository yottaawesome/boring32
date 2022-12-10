export module boring32.xaudio2:masteringvoice;
import :voice;
import <win32.hpp>;

export namespace Boring32::XAudio2
{
	// Not reference counted
	// https://docs.microsoft.com/en-us/windows/win32/api/xaudio2/nn-xaudio2-ixaudio2masteringvoice
	class MasteringVoice : public Voice
	{
		public:
			virtual ~MasteringVoice();
			MasteringVoice(const MasteringVoice&) = delete;
			MasteringVoice(MasteringVoice&&) noexcept;
			MasteringVoice(IXAudio2MasteringVoice* voice);

		public:
			virtual MasteringVoice& operator=(const MasteringVoice&) = delete;
			virtual MasteringVoice& operator=(MasteringVoice&&) noexcept;

		public:
			virtual void Close() override;
			virtual IXAudio2MasteringVoice* Get() const noexcept;

		protected:
			IXAudio2MasteringVoice* m_voice;
	};
}