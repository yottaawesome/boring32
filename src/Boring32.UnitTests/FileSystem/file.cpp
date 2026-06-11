#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32;

namespace FileSystem
{
	TEST_CLASS(File)
	{
		TEST_METHOD(TestConstructor)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt", Boring32::Win32::GenericRead };
		}

		TEST_METHOD(TestGetHandle)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt", Boring32::Win32::GenericRead };
			auto handle = file.GetHandle();
			Assert::IsTrue(handle != nullptr);
		}

		TEST_METHOD(TestGetFileType)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt", Boring32::Win32::GenericRead };
			auto type = file.GetType();
			Assert::IsTrue(type == Boring32::Win32::FileType::Disk);
		}

		TEST_METHOD(TestGetSize)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt", Boring32::Win32::GenericRead };
			auto size = file.GetSize();
			Assert::IsTrue(size == 0);
		}

		TEST_METHOD(TestClose)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt", Boring32::Win32::GenericRead };
			file.Close();
			Assert::IsTrue(file.GetHandle() == nullptr);
		}

		TEST_METHOD(TestGetFilePath)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt", Boring32::Win32::GenericRead };
			auto path = file.GetFilePath();
			Assert::IsTrue(path.ends_with(L"testfile.txt"));
		}

		TEST_METHOD(TestReadWriteClearFile)
		{
			auto file = Boring32::FileSystem::File{ L"testfile.txt",  Boring32::Win32::GenericWrite | Boring32::Win32::GenericRead };

			constexpr auto stringToWrite = std::string_view{ "Hello, world!" };

			auto dataToWrite = stringToWrite 
				| std::ranges::views::transform([](char c) { return static_cast<std::byte>(c); }) 
				| std::ranges::to<std::vector<std::byte>>();

			file.WriteFile(dataToWrite);
			file.Flush();

			file.SetFilePointer(0, Boring32::Win32::FilePointerMoveMethod::Begin);
			auto size = file.GetSize();
			auto dataReadBack = file.ReadFile(size);

			auto stringReadBack = dataReadBack 
				| std::ranges::views::transform([](std::byte b) { return static_cast<char>(b); }) 
				| std::ranges::to<std::string>();

			file.Clear();
			Assert::IsTrue(stringReadBack == stringToWrite);
			Assert::IsTrue(size == dataToWrite.size());
			Assert::IsTrue(file.GetSize() == 0);
		}
	};
}