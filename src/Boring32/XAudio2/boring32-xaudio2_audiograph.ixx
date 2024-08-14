export module boring32:xaudio2_audiograph;

export namespace Boring32::XAudio2
{
	class AudioGraph
	{
		public:
			virtual ~AudioGraph() 
			{
				Close();
			}

		public:
			virtual void Close() {}
	};
}