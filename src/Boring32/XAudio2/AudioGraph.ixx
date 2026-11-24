export module Boring32:XAudio2.AudioGraph;

export namespace Boring32::XAudio2
{
	struct AudioGraph
	{
		~AudioGraph() 
		{
			Close();
		}

		void Close() {}
	};
}