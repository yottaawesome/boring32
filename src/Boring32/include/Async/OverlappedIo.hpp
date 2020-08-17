#pragma once
#include "OverlappedOp.hpp"

namespace Boring32::Async
{
	class OverlappedIo : public OverlappedOp
	{
		public:
			virtual ~OverlappedIo();
			OverlappedIo();
			OverlappedIo(
				const bool isInheritable,
				const bool manualReset,
				const bool isSignaled, 
				const std::wstring name
			);

			OverlappedIo(const OverlappedIo& other);
			virtual void operator=(const OverlappedIo& other);

			OverlappedIo(OverlappedIo&& other) noexcept;
			virtual void operator=(OverlappedIo&& other) noexcept;

			virtual bool GetBytesTransferred(const bool wait, DWORD& outBytes);

		public:
			std::wstring IoBuffer;
			Raii::Win32Handle IoHandle;

		protected:
			virtual void Copy(const OverlappedIo& other);
			virtual void Move(OverlappedIo& other) noexcept;
	};
}
