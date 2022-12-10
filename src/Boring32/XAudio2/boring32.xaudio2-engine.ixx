export module boring32.xaudio2:engine;
import <win32.hpp>;

export namespace Boring32::XAudio2
{
	class Engine
	{
		public:
			virtual ~Engine();
			Engine();
			Engine(const Engine&) = delete;
			Engine(Engine&&) noexcept;

		public:
			virtual Engine& operator=(const Engine&) = delete;
			virtual Engine& operator=(Engine&&) noexcept;

		public:
			virtual void Close();
			virtual void Initialise();

		protected:
			Microsoft::WRL::ComPtr<IXAudio2> m_engine;
	};
}