module;

#include <source_location>
#include <Windows.h>
#include <xaudio2.h>
#include <wrl/client.h>

module boring32.xaudio2.engine;
import boring32.error.comerror;

namespace Boring32::XAudio2
{
	Engine::~Engine()
	{
		Close();
	}

	void Engine::Close() 
	{
		m_engine = nullptr;
	}

	void Engine::Initialise()
	{
		const HRESULT hr = XAudio2Create(&m_engine, 0, XAUDIO2_DEFAULT_PROCESSOR);
		if (FAILED(hr))
			throw Error::ComError(
				std::source_location::current(), 
				"Failed to create XAudio2 engine", 
				hr
			);
	}
}