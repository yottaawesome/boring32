#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace TaskScheduler
{
	TEST_CLASS(TaskService)
	{
		public:
			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::TaskScheduler::TaskService service;
				Assert::IsFalse(service);
			}

			TEST_METHOD(TestConnect)
			{
				Boring32::TaskScheduler::TaskService service;
				service.Connect();
				Assert::IsTrue(service);
			}

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::TaskScheduler::TaskService service1;
				service1.Connect();
				Boring32::TaskScheduler::TaskService service2(service1);
				Assert::IsTrue(service1);
				Assert::IsTrue(service1 == service2);
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::TaskScheduler::TaskService service1;
				service1.Connect();
				Boring32::TaskScheduler::TaskService service2 = service1;
				Assert::IsTrue(service1);
				Assert::IsTrue(service1 == service2);
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::TaskScheduler::TaskService service1;
				service1.Connect();
				Boring32::TaskScheduler::TaskService service2(std::move(service1));
				Assert::IsFalse(service1);
				Assert::IsTrue(service2);
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::TaskScheduler::TaskService service1;
				service1.Connect();
				Boring32::TaskScheduler::TaskService service2 = std::move(service1);
				Assert::IsFalse(service1);
				Assert::IsTrue(service2);
			}

			TEST_METHOD(TestClose)
			{
				Boring32::TaskScheduler::TaskService service;
				service.Connect();
				Assert::IsTrue(service);
				service.Close();
				Assert::IsFalse(service);
			}

			TEST_METHOD(TestGetRootFolder)
			{
				Boring32::TaskScheduler::TaskService service;
				service.Connect();
				Boring32::TaskScheduler::TaskFolder rootFolder = service.GetRootFolder();
			}
	};
}