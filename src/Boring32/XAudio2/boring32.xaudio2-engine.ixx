export module boring32.xaudio2:engine;
import <win32.hpp>;

export namespace Boring32::XAudio2
{
	class Engine
	{
		public:
			virtual ~Engine() = default;
			Engine() = default;
			Engine(const Engine&) = delete;
			Engine(Engine&&) noexcept = default;

		public:
			virtual Engine& operator=(const Engine&) = delete;
			virtual Engine& operator=(Engine&&) noexcept = default;

		public:
			virtual void Close();
			virtual void Initialise();

		protected:
			Microsoft::WRL::ComPtr<IXAudio2> m_engine;
	};
}