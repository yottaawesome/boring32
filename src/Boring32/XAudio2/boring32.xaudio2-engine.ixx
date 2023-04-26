export module boring32.xaudio2:engine;
import <win32.hpp>;
import :xaudio2error;
import boring32.error;
import <stdexcept>;

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
			virtual void Close()
			{
				m_engine = nullptr;
			}

			virtual void Initialise()
			{
				const HRESULT hr = XAudio2Create(&m_engine, 0, XAUDIO2_DEFAULT_PROCESSOR);
				if (FAILED(hr)) Error::ThrowNested(
					Error::COMError("Failed to create XAudio2 engine", hr),
					XAudio2Error("An error occurred when initialising the XAudio2 engine")
				);
			}

		protected:
			Microsoft::WRL::ComPtr<IXAudio2> m_engine;
	};
}