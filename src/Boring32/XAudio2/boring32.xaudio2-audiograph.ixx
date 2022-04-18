export module boring32.xaudio2:audiograph;

export namespace Boring32::XAudio2
{
	class AudioGraph
	{
		public:
			virtual ~AudioGraph();

		public:
			virtual void Close();
	};
}