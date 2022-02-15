module;

#include <Windows.h>
#include <xaudio2.h>

export module boring32.xaudio2.masteringvoice;
import boring32.xaudio2.voice;

export namespace Boring32::XAudio2
{
	// Not reference counted
	// https://docs.microsoft.com/en-us/windows/win32/api/xaudio2/nn-xaudio2-ixaudio2masteringvoice
	class MasteringVoice : public Voice
	{
		public:
			virtual ~MasteringVoice();
			MasteringVoice(IXAudio2MasteringVoice* voice);

		public:
			virtual void Close() override;

		protected:
			IXAudio2MasteringVoice* m_masteringVoice;
	};
}