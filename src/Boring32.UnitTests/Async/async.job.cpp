#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import std;
import boring32;

namespace Async
{
	TEST_CLASS(JobTests)
	{
		TEST_METHOD(TestDefaultConstructor)
		{
			auto job = Boring32::Async::Job{};
			Assert::IsFalse(job);
		}
		TEST_METHOD(TestCreateAnonymousJob)
		{
			auto job = Boring32::Async::Job(true);
			Assert::IsTrue(job);
		}
		TEST_METHOD(TestCreateNamedJob)
		{
			auto job = Boring32::Async::Job(true, L"TestJob");
			Assert::IsTrue(job);
		}
		TEST_METHOD(TestOpenExistingNamedJob)
		{
			auto job1 = Boring32::Async::Job(true, L"TestJob2");
			auto job2 = Boring32::Async::Job(true, L"TestJob2", Boring32::Win32::GenericRead);
			Assert::IsTrue(job1);
			Assert::IsTrue(job2);
		}
	};
}
