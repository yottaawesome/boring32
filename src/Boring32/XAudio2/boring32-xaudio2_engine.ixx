export module boring32:xaudio2_engine;
import boring32.shared;
import :error;
import :xaudio2_xaudio2error;

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
				const Win32::HRESULT hr = Win32::XAudio2Create(&m_engine, 0, Win32::XAudio2DefaultProcessor);
				if (Win32::HrFailed(hr)) Error::ThrowNested(
					Error::COMError("Failed to create XAudio2 engine", hr),
					XAudio2Error("An error occurred when initialising the XAudio2 engine")
				);
			}

		protected:
			Win32::ComPtr<Win32::IXAudio2> m_engine;
	};
}