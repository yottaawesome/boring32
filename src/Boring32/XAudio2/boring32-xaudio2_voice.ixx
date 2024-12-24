export module boring32:xaudio2_voice;

export namespace Boring32::XAudio2
{
	struct Voice
	{
		virtual ~Voice()
		{
			Close();
		}

		virtual void Close() = 0;
	};
}