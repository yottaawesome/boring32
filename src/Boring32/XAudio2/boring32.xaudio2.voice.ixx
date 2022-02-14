export module boring32.xaudio2.voice;

export namespace Boring32::XAudio2
{
	class Voice
	{
		public:
			virtual ~Voice();

		public:
			virtual void Close();
	};
}