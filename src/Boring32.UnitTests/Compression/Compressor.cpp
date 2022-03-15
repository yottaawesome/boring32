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
			TEST_METHOD(TestCompressorDefaultConstructor)
			{
				Boring32::Compression::Compressor compressor;
			}

			TEST_METHOD(TestCompressorConstructor)
			{
				Boring32::Compression::Compressor compressor(Boring32::Compression::CompressionType::MSZIP);
				Assert::IsNotNull(compressor.GetHandle());
				Assert::IsTrue(compressor.GetType() == Boring32::Compression::CompressionType::MSZIP);
			}

			TEST_METHOD(TestCompressorCopyConstructor)
			{
				Boring32::Compression::Compressor compressor1(Boring32::Compression::CompressionType::MSZIP);
				Boring32::Compression::Compressor compressor2(compressor1);
				Assert::IsNotNull(compressor1.GetHandle());
				Assert::IsTrue(compressor1.GetType() == Boring32::Compression::CompressionType::MSZIP);
				Assert::IsNotNull(compressor2.GetHandle());
				Assert::IsTrue(compressor2.GetType() == Boring32::Compression::CompressionType::MSZIP);
				Assert::IsTrue(compressor1.GetHandle() != compressor2.GetHandle());
			}

			TEST_METHOD(TestCompressorCopyAssignment)
			{
				Boring32::Compression::Compressor compressor1(Boring32::Compression::CompressionType::MSZIP);
				Boring32::Compression::Compressor compressor2 = compressor1;
				Assert::IsNotNull(compressor1.GetHandle());
				Assert::IsTrue(compressor1.GetType() == Boring32::Compression::CompressionType::MSZIP);
				Assert::IsNotNull(compressor2.GetHandle());
				Assert::IsTrue(compressor2.GetType() == Boring32::Compression::CompressionType::MSZIP);
				Assert::IsTrue(compressor1.GetHandle() != compressor2.GetHandle());
			}

			TEST_METHOD(TestCompressorGetCompressedBytesMSZIP)
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

			TEST_METHOD(TestCompressorGetCompressedBytesLZMS)
			{
				Boring32::Compression::Compressor compressor(Boring32::Compression::CompressionType::LZMS);
				const std::byte* buffer = (std::byte*)&m_compressionString[0];
				const size_t compressedBytesSize = compressor.GetCompressedSize({
						buffer,
						buffer + m_compressionString.size()
					}
				);
				Assert::IsTrue(compressedBytesSize > 0);
			}

			TEST_METHOD(TestCompressorGetCompressedBytesXPRESS)
			{
				Boring32::Compression::Compressor compressor(Boring32::Compression::CompressionType::XPRESS);
				const std::byte* buffer = (std::byte*)&m_compressionString[0];
				const size_t compressedBytesSize = compressor.GetCompressedSize({
						buffer,
						buffer + m_compressionString.size()
					}
				);
				Assert::IsTrue(compressedBytesSize > 0);
			}

			TEST_METHOD(TestCompressorGetCompressedBytesXPRESSHuffman)
			{
				Boring32::Compression::Compressor compressor(Boring32::Compression::CompressionType::XPRESSHuffman);
				const std::byte* buffer = (std::byte*)&m_compressionString[0];
				const size_t compressedBytesSize = compressor.GetCompressedSize({
						buffer,
						buffer + m_compressionString.size()
					}
				);
				Assert::IsTrue(compressedBytesSize > 0);
			}

			TEST_METHOD(TestCompressorCompressionMSZIP)
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

			TEST_METHOD(TestCompressorCompressionLZMS)
			{
				Boring32::Compression::Compressor compressor(Boring32::Compression::CompressionType::LZMS);
				const std::byte* buffer = (std::byte*)&m_compressionString[0];
				std::vector<std::byte> compressed = compressor.CompressBuffer({
						buffer,
						buffer + m_compressionString.size()
					}
				);
				Assert::IsTrue(compressed.size() > 0);
			}

			TEST_METHOD(TestCompressorCompressionXPRESS)
			{
				Boring32::Compression::Compressor compressor(Boring32::Compression::CompressionType::XPRESS);
				const std::byte* buffer = (std::byte*)&m_compressionString[0];
				std::vector<std::byte> compressed = compressor.CompressBuffer({
						buffer,
						buffer + m_compressionString.size()
					}
				);
				Assert::IsTrue(compressed.size() > 0);
			}

			TEST_METHOD(TestCompressorCompressionXPRESSHuffman)
			{
				Boring32::Compression::Compressor compressor(Boring32::Compression::CompressionType::XPRESSHuffman);
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