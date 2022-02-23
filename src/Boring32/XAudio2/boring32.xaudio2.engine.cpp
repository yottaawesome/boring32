module;

#include <stdexcept>
#include <Windows.h>
#include <xaudio2.h>
#include <wrl/client.h>

module boring32.xaudio2.engine;
import boring32.xaudio2.xaudio2error;
import boring32.error.comerror;

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

	void Engine::Initialise() try
	{
		const HRESULT hr = XAudio2Create(&m_engine, 0, XAUDIO2_DEFAULT_PROCESSOR);
		if (FAILED(hr)) throw Error::ComError(
			std::source_location::current(), 
			"Failed to create XAudio2 engine", 
			hr
		);
	}
	catch (...)
	{
		// is there a way for throwing a nested exception without the added try-catch?
		std::throw_with_nested(
			XAudio2Error(
				std::source_location::current(), 
				"An error occurred when initialising the XAudio2 engine"
			)
		);
	}
}