#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Async
{
	using TestType = Boring32::Async::ThreadSafeVector<int, Boring32::Async::CriticalSection>;

	TEST_CLASS(ThreadSafeVector)
	{
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
		}

		TEST_METHOD(TestEraseOne)
		{
			TestType vec;
			vec.Add(1);
			vec.Add(2);
			vec.EraseOne([](int v) { return v == 1; });
			Assert::AreEqual(size_t(1), vec.Size());
		}

		TEST_METHOD(TestEraseMultiple)
		{
			TestType vec;
			vec.Add(1);
			vec.Add(1);
			vec.EraseMultiple([](int v) { return v == 1; });
			Assert::AreEqual(size_t(0), vec.Size());
		}

		TEST_METHOD(ExtractOne)
		{
			TestType vec;
			vec.Add(1);
			vec.Add(2);
			auto i = vec.ExtractOne([](int v) { return v == 1; });
			Assert::IsTrue(i.has_value());
			Assert::AreEqual(*i, 1);
		}

		TEST_METHOD(TestDeleteWhere)
		{
			TestType vec;
			vec.Add(1);
			vec.Add(1);
			vec.Add(2);
			auto [removed, original] = vec.DeleteWhere([](int v) { return v == 1; });
			Assert::AreEqual(removed, 2ull);
			Assert::AreEqual(original, 3ull);
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
	};
}