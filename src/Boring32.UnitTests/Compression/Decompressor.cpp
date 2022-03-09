#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.compression;

namespace Compression
{
	TEST_CLASS(Decompressor)
	{
		private:
			const std::string m_compressionString = "Hello world! This buffer will be compressed";

		public:
			TEST_METHOD(TestDecompressorConstructor)
			{
				Boring32::Compression::Decompressor decompressor(Boring32::Compression::CompressionType::MSZIP);
				Assert::IsNotNull(decompressor.GetHandle());
				Assert::IsTrue(decompressor.GetType() == Boring32::Compression::CompressionType::MSZIP);
			}

			TEST_METHOD(TestInvalidDecompression)
			{
				Assert::ExpectException<Boring32::Compression::CompressionError>(
					[]() 
					{ 
						Boring32::Compression::Decompressor decompressor(Boring32::Compression::CompressionType::MSZIP);
						auto x = decompressor.DecompressBuffer({ std::byte(0x1), std::byte(0x2) }); 
					}
				);
			}

			TEST_METHOD(TestDecompressorGetDecompressedSizeMSZIP)
			{
				const auto type = Boring32::Compression::CompressionType::MSZIP;

				Boring32::Compression::Compressor compressor(type);
				const std::byte* buffer = (std::byte*)&m_compressionString[0];
				const auto compressedData = compressor.CompressBuffer({
						buffer,
						buffer + m_compressionString.size()
					}
				);

				Boring32::Compression::Decompressor decompressor(type);

				Assert::IsTrue(decompressor.GetDecompressedSize(compressedData) > 0);
			}

			TEST_METHOD(TestDecompressorGetDecompressedSizeXPRESS)
			{
				const auto type = Boring32::Compression::CompressionType::XPRESS;

				Boring32::Compression::Compressor compressor(type);
				const std::byte* buffer = (std::byte*)&m_compressionString[0];
				const auto compressedData = compressor.CompressBuffer({
						buffer,
						buffer + m_compressionString.size()
					}
				);

				Boring32::Compression::Decompressor decompressor(type);

				Assert::IsTrue(decompressor.GetDecompressedSize(compressedData) > 0);
			}

			TEST_METHOD(TestDecompressorGetDecompressedSizeXPRESSHuffman)
			{
				const auto type = Boring32::Compression::CompressionType::XPRESSHuffman;

				Boring32::Compression::Compressor compressor(type);
				const std::byte* buffer = (std::byte*)&m_compressionString[0];
				const auto compressedData = compressor.CompressBuffer({
						buffer,
						buffer + m_compressionString.size()
					}
				);

				Boring32::Compression::Decompressor decompressor(type);

				Assert::IsTrue(decompressor.GetDecompressedSize(compressedData) > 0);
			}

			TEST_METHOD(TestDecompressorGetDecompressedSizeLZMS)
			{
				const auto type = Boring32::Compression::CompressionType::LZMS;

				Boring32::Compression::Compressor compressor(type);
				const std::byte* buffer = (std::byte*)&m_compressionString[0];
				const auto compressedData = compressor.CompressBuffer({
						buffer,
						buffer + m_compressionString.size()
					}
				);

				Boring32::Compression::Decompressor decompressor(type);

				Assert::IsTrue(decompressor.GetDecompressedSize(compressedData) > 0);
			}
	};
}