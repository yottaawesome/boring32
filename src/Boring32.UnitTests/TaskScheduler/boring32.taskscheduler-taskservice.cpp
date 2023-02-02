#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.taskscheduler;

namespace TaskScheduler
{
	TEST_CLASS(TaskService)
	{
		public:
			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::TaskScheduler::TaskService service;
			}
	};
}