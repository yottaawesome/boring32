export module boring32:xaudio2_engine;
import boring32.win32;
import :error;
import :xaudio2_xaudio2error;

export namespace Boring32::XAudio2
{
	struct Engine final
	{
		Engine() = default;
		Engine(const Engine&) = delete;
		Engine(Engine&&) noexcept = default;

		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) noexcept = default;

		void Close()
		{
			m_engine = nullptr;
		}

		void Initialise()
		{
			Win32::HRESULT hr = Win32::XAudio2Create(&m_engine, 0, Win32::XAudio2DefaultProcessor);
			if (Win32::HrFailed(hr)) Error::ThrowNested(
				Error::COMError(hr, "Failed to create XAudio2 engine"),
				XAudio2Error("An error occurred when initialising the XAudio2 engine")
			);
		}

	private:
		Win32::ComPtr<Win32::IXAudio2> m_engine;
	};
}
