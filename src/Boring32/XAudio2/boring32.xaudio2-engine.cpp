module boring32.xaudio2:engine;
import :xaudio2error;
import boring32.error;
import <stdexcept>;

namespace Boring32::XAudio2
{
	Engine::~Engine()
	{
		Close();
	}

	Engine::Engine() = default;
	Engine::Engine(Engine&&) noexcept = default;
	Engine& Engine::operator=(Engine&&) noexcept = default;

	void Engine::Close() 
	{
		m_engine = nullptr;
	}

	void Engine::Initialise()
	{
		const HRESULT hr = XAudio2Create(&m_engine, 0, XAUDIO2_DEFAULT_PROCESSOR);
		if (FAILED(hr)) Error::ThrowNested(
			Error::COMError("Failed to create XAudio2 engine", hr),
			XAudio2Error("An error occurred when initialising the XAudio2 engine")
		);
	}
}