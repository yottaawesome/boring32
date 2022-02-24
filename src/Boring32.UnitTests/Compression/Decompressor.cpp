#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.compression;

namespace Compression
{
	TEST_CLASS(Decompressor)
	{
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
	};
}