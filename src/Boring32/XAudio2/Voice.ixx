export module Boring32:XAudio2.Voice;

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