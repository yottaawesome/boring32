#pragma once
#include "Event.hpp"

namespace Boring32::Async
{
	class OverlappedOp
	{
		public:
			virtual ~OverlappedOp();
			OverlappedOp();
			OverlappedOp(
				const bool createOrOpen,
				const bool isInheritable,
				const bool manualReset,
				const bool isSignaled,
				const std::wstring name
			);

			OverlappedOp(const OverlappedOp& other);
			virtual void operator=(const OverlappedOp& other);

			OverlappedOp(OverlappedOp&& other) noexcept;
			virtual void operator=(OverlappedOp&& other) noexcept;

		public:
			Event IoEvent;
			OVERLAPPED IoOverlapped;
			bool CallReturnValue;
			DWORD LastErrorValue;

		protected:
			virtual void Copy(const OverlappedOp& other);
			virtual void Move(OverlappedOp& other) noexcept;
	};
}