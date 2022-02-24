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
			Boring32::Compression::Decompressor compressor(Boring32::Compression::CompressionType::MSZIP);
			Assert::IsNotNull(compressor.GetHandle());
			Assert::IsTrue(compressor.GetType() == Boring32::Compression::CompressionType::MSZIP);
		}
	};
}