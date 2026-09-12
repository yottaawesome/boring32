#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Com
{
	// Dummy COM interface for testing
	struct IDummy
	{
		unsigned RefCount = 0;
		constexpr unsigned AddRef() { return ++RefCount; };
		constexpr unsigned Release() { return --RefCount; };
	};

	using Ptr = Boring32::Com::Ptr<IDummy>;

	// Default constructor
	static_assert(
		[]
		{
			Boring32::Com::Ptr<IDummy> p1{};
			return p1.get() == nullptr;
		}(),
		"Expected p1.get() to return nullptr.");

	// Constructor from raw pointer
	static_assert(
		[]
		{
			IDummy q{};
			Ptr p1{ Ptr::Own{&q} };
			return p1.get() == &q and q.RefCount == 0;
		}(),
		"Expected p1.get() to return &q and not to increment the existing count.");

	// Copy semantics
	static_assert(
		[]
		{
			IDummy q{};
			Ptr p1{ Ptr::Own{&q} };
			Ptr p2 = p1;
			return p1.get() == &q and p2.get() == &q;
		}(),
		"Expected p2.get() to return &q.");

	static_assert(
		[]
		{
			IDummy q{};
			Ptr p1{ Ptr::Own{&q} };
			Ptr p2{};
			p2 = p1;
			return q.RefCount == 1;
		}(),
		"Expected p2.get() to return &q.");

	// Move semantics
	static_assert(
		[]
		{
			IDummy q{};
			Ptr p1{ Ptr::Own{&q} };
			Ptr p2{ std::move(p1) };
			return p1.get() == nullptr and p2.get() == &q;
		}(),
		"Expected p1.get() to return nullptr and p2.get() to return &q.");
	static_assert(
		[]
		{
			IDummy q{};
			Ptr p1{ Ptr::Own{&q} };
			Ptr p2{};
			p2 = std::move(p1);
			return p1.get() == nullptr and p2.get() == &q;
		}(),
		"Expected p1.get() to return nullptr and p2.get() to return &q.");

	// Miscellaneous
	static_assert(
		[]
		{
			IDummy q{ 1 };
			Ptr p1{ Ptr::Own{&q} };
			p1.reset();
			return q.RefCount == 0;
		}(),
		"Expected RefCount to be 0 after reset() is called.");

	static_assert(
		[]
		{
			IDummy q{ 1 };
			{
				Ptr p{ Ptr::Own{&q} };
			}
			return q.RefCount == 0;
		}(),
		"Expected RefCount to be 0 after Ptr is destroyed.");

	static_assert(
		[]
		{
			IDummy q{ 1 };
			Ptr p1{ Ptr::Own{&q} };
			auto rawPtr = p1.detach();
			return q.RefCount == 1 and p1.get() == nullptr and rawPtr == &q;
		}(),
		"Expected RefCount to be 1, p1.get() to be nullptr, and rawPtr to be &q.");

	static_assert(
		[]
		{
			IDummy q{};
			Ptr p1{ Ptr::Own{&q} };
			return *p1 == &q;
		}(),
		"Expected *p1 to be &q.");
}
