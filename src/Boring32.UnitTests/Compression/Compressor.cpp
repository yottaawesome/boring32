#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.compression;

namespace Compression
{
	TEST_CLASS(Compressor)
	{
		private: 
			const std::string m_compressionString = "Hello world! This buffer will be compressed";

		public:
			TEST_METHOD(TestCompressorConstructor)
			{
				Boring32::Compression::Compressor compressor(Boring32::Compression::CompressionType::MSZIP);
				Assert::IsNotNull(compressor.GetHandle());
				Assert::IsTrue(compressor.GetType() == Boring32::Compression::CompressionType::MSZIP);
			}

			TEST_METHOD(TestCompressorGetCompressedBytes)
			{
				Boring32::Compression::Compressor compressor(Boring32::Compression::CompressionType::MSZIP);
				const std::byte* buffer = (std::byte*)&m_compressionString[0];
				const size_t compressedBytesSize = compressor.GetCompressedSize({
						buffer,
						buffer + m_compressionString.size()
					}
				);
				Assert::IsTrue(compressedBytesSize > 0);
			}

			TEST_METHOD(TestCompressorCompression)
			{
				Boring32::Compression::Compressor compressor(Boring32::Compression::CompressionType::MSZIP);
				const std::byte* buffer = (std::byte*)&m_compressionString[0];
				std::vector<std::byte> compressed = compressor.CompressBuffer({ 
						buffer, 
						buffer + m_compressionString.size() 
					}
				);
				Assert::IsTrue(compressed.size() > 0);
			}
	};
}