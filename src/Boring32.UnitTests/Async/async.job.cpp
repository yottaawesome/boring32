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
			Assert::IsFalse(job.HasJob());
		}
		TEST_METHOD(TestCreateAnonymousJob)
		{
			auto job = Boring32::Async::Job(true);
			Assert::IsTrue(job.HasJob());
		}
		TEST_METHOD(TestCreateNamedJob)
		{
			auto job = Boring32::Async::Job(true, L"TestJob");
			Assert::IsTrue(job.HasJob());
		}
		TEST_METHOD(TestOpenExistingNamedJob)
		{
			auto job1 = Boring32::Async::Job(true, L"TestJob2");
			auto job2 = Boring32::Async::Job(true, L"TestJob2", Boring32::Win32::GenericRead);
			Assert::IsTrue(job1.HasJob());
			Assert::IsTrue(job2.HasJob());
		}
		TEST_METHOD(TestMoveConstructor)
		{
			auto job1 = Boring32::Async::Job(true, L"TestJob3");
			auto job2 = std::move(job1);
			Assert::IsFalse(job1.HasJob());
			Assert::IsTrue(job2.HasJob());
		}
		TEST_METHOD(TestMoveAssignment)
		{
			auto job1 = Boring32::Async::Job(true, L"TestJob4");
			auto job2 = Boring32::Async::Job{};
			job2 = std::move(job1);
			Assert::IsFalse(job1.HasJob());
			Assert::IsTrue(job2.HasJob());
		}
		TEST_METHOD(TestSetInformation)
		{
			auto job = Boring32::Async::Job(true, L"TestJob5");
			auto jeli = Boring32::Win32::JOBOBJECT_EXTENDED_LIMIT_INFORMATION{};
			jeli.BasicLimitInformation.LimitFlags = Boring32::Win32::JobObjectLimits::KillOnJobClose;
			job.SetInformation(jeli);
		}
		TEST_METHOD(TestClose)
		{
			auto job = Boring32::Async::Job(true, L"TestJob6");
			job.Close();
			Assert::IsFalse(job.HasJob());
		}
		TEST_METHOD(TestGetName)
		{
			auto job = Boring32::Async::Job(true, L"TestJob7");
			Assert::AreEqual(L"TestJob7", job.GetName().c_str());
		}
	};
}
