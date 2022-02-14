export module boring32.xaudio2.engine;

export namespace Boring32::XAudio2
{
	class Engine
	{
		public:
			virtual ~Engine();

		public:
			virtual void Close();
	};
}