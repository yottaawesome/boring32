#pragma once
#include "OverlappedOp.hpp"

namespace Boring32::Async
{
	class OverlappedIo : public OverlappedOp
	{
		public:
			virtual ~OverlappedIo();
			OverlappedIo();
			OverlappedIo(OverlappedIo&& other) noexcept;
			virtual OverlappedIo& operator=(OverlappedIo&& other) noexcept;

		// Non-copyable
		public:
			OverlappedIo(const OverlappedIo& other) = delete;
			virtual OverlappedIo& operator=(const OverlappedIo& other) = delete;

		public:
			std::wstring IoBuffer;
			virtual void ResizeBuffer();

		protected:
			virtual void Move(OverlappedIo& other) noexcept;
	};
}
