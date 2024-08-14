export module boring32:xaudio2_voice;

export namespace Boring32::XAudio2
{
	class Voice
	{
		public:
			virtual ~Voice()
			{
				Close();
			}

		public:
			virtual void Close() = 0;
	};
}