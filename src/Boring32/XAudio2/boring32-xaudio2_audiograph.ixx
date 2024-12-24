export module boring32:xaudio2_audiograph;

export namespace Boring32::XAudio2
{
	struct AudioGraph
	{
		~AudioGraph() 
		{
			Close();
		}

		void Close() {}
	};
}