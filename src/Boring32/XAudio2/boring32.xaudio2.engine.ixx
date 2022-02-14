module;

#include <Windows.h>
#include <xaudio2.h>
#include <wrl/client.h>

export module boring32.xaudio2.engine;

export namespace Boring32::XAudio2
{
	class Engine
	{
		public:
			virtual ~Engine();

		public:
			virtual void Close();
			virtual void Initialise();

		protected:
			Microsoft::WRL::ComPtr<IXAudio2> m_engine;
	};
}