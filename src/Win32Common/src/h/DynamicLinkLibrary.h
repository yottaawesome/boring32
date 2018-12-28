#pragma once
#include "../../include/Win32Common.h"

namespace Win32Common
{
	class DynamicLinkLibrary : public IDynamicLinkLibrary
	{
		public:
			DynamicLinkLibrary(wstring& path);
			virtual const wstring& GetPath() override;
			virtual const HMODULE GetHandle();
			virtual ~DynamicLinkLibrary();

		protected:
			const wstring path;
			HMODULE libraryHandle;
	};
}
