#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Async
{
	using TestType = Boring32::Async::ThreadSafeVector<int, Boring32::Async::CriticalSection>;

	TEST_CLASS(ThreadSafeVector)
	{
		TEST_METHOD(TestDefaultConstructor)
		{
			TestType vec;
			Assert::AreEqual(size_t(0), vec.Size());
			Assert::IsNotNull(vec.GetWaitableHandle());
		}

		TEST_METHOD(TestVariadicConstructor)
		{
			TestType vec{ 1,2,3 };
			auto actual = vec.ToVector();
			bool allMatch = std::ranges::all_of(
				actual,
				[](int v) { return v == 1 or v == 2 or v == 3; });
			Assert::IsTrue(allMatch and actual.size() == 3);
		}

		TEST_METHOD(TestAdd)
		{
			TestType vec;
			vec.Add(1);
			vec.Add(2);
			Assert::AreEqual(size_t(2), vec.Size());
			Assert::IsTrue(vec.IsSignalled());
		}

		TEST_METHOD(TestDeleteOne)
		{
			TestType vec;
			vec.Add(1);
			vec.Add(2);
			Assert::IsTrue(vec.DeleteOne([](int v) { return v == 1; }));
			Assert::AreEqual(size_t(1), vec.Size());
			Assert::IsTrue(vec.IsSignalled());
		}

		TEST_METHOD(TestDeleteNoneFound)
		{
			TestType vec;
			vec.Add(1);
			vec.Add(2);
			Assert::IsFalse(vec.DeleteOne([](int v) { return v == 8; }));
			Assert::AreEqual(size_t(2), vec.Size());
			Assert::IsTrue(vec.IsSignalled());
		}

		TEST_METHOD(TestIsSignalledHasElements)
		{
			TestType vec;
			vec.Add(1);
			Assert::IsTrue(vec.IsSignalled());
		}

		TEST_METHOD(TestIsSignalledHasNoElements)
		{
			TestType vec;
			vec.Add(1);
			Assert::IsTrue(vec.IsSignalled());
			vec.DeleteAt(0);
			Assert::IsFalse(vec.IsSignalled());
		}

		TEST_METHOD(ExtractOne)
		{
			TestType vec;
			vec.Add(1);
			vec.Add(2);
			auto i = vec.ExtractOne([](int v) { return v == 1; });
			Assert::IsTrue(i.has_value());
			Assert::AreEqual(*i, 1);
			Assert::IsTrue(vec.IsSignalled());
		}

		TEST_METHOD(TestDeleteWhere)
		{
			TestType vec;
			vec.Add(1);
			vec.Add(1);
			vec.Add(2);
			auto removed = vec.DeleteWhere([](int v) { return v == 1; });
			Assert::AreEqual(removed, 2ull);
			Assert::AreEqual(vec.Size(), 1ull);
			Assert::IsTrue(vec.IsSignalled());
		}

		TEST_METHOD(TestToVector)
		{
			TestType vec{ 1,2,3 };
			auto actual = vec.ToVector();
			bool allMatch = std::ranges::all_of(
				actual, 
				[](int v) { return v == 1 or v == 2 or v == 3; });
			Assert::IsTrue(allMatch and actual.size() == 3);
		}
		
		TEST_METHOD(TestClear)
		{
			TestType vec{ 1,2,3 };
			Assert::AreEqual(size_t(3), vec.Size());
			vec.Clear();
			Assert::AreEqual(size_t(0), vec.Size());
			Assert::IsFalse(vec.IsSignalled());
		}
		
		TEST_METHOD(TestAt)
		{
			TestType vec{ 1,3,2 };
			Assert::AreEqual(1, vec.At(0));
			Assert::AreEqual(3, vec.At(1));
			Assert::AreEqual(2, vec.At(2));
		}

		TEST_METHOD(TestAtIndexOutOfBounds)
		{
			TestType vec{ 1,3,2 };
			try
			{
				vec.At(3);
				throw std::exception("Expected exception was not thrown.");
			}
			catch (const std::runtime_error&)
			{
			}
		}

		TEST_METHOD(TestDoWithLock)
		{
			TestType vec{ 1,3,2 };
			vec.DoWithLock([](std::vector<int>& v) { v.push_back(4); });
			Assert::AreEqual(size_t(4), vec.Size());
			auto actual = vec.ToVector();
			bool allMatch = std::ranges::all_of(
				actual,
				[](int v) { return v == 1 or v == 2 or v == 3 or v == 4; });
			Assert::AreEqual(vec.At(3), 4);
			Assert::IsTrue(allMatch and actual.size() == 4);
		}

		TEST_METHOD(TestDeleteAt)
		{
			TestType vec{ 1,3,2 };
			vec.DeleteAt(1);
			Assert::AreEqual(size_t(2), vec.Size());
			Assert::AreEqual(vec.At(1), 2);
			Assert::IsTrue(vec.IsSignalled());
		}

		TEST_METHOD(TestForEach)
		{
			TestType vec{ 1,3,2 };
			int sum = 0;
			vec.ForEach([&sum](int v) { sum += v; return true; });
			Assert::AreEqual(6, sum);
		}

		TEST_METHOD(TestForEachThenClear)
		{
			TestType vec{ 1,3,2 };
			int sum = 0;
			vec.ForEachThenClear([&sum](int v) { sum += v; });
			Assert::AreEqual(6, sum);
			Assert::AreEqual(size_t(0), vec.Size());
		}

		TEST_METHOD(TestGetSize)
		{
			TestType vec{ 1,3,2 };
			Assert::AreEqual(size_t(3), vec.Size());
		}

		TEST_METHOD(TestGetWaitableHandle)
		{
			TestType vec{ 1,3,2 };
			auto handle = vec.GetWaitableHandle();
			Assert::IsNotNull(handle);
		}

		TEST_METHOD(TestIndexOf)
		{
			TestType vec{ 1,3,2 };
			auto index = vec.IndexOf([](int v) { return v == 3; });
			Assert::IsTrue(index.has_value());
			Assert::AreEqual(1ull, *index);
		}
	};
}
