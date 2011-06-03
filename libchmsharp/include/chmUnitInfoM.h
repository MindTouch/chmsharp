
#pragma once

#include <chm_lib.h>

using namespace System;

namespace CHMsharp
{
	public ref struct chmUnitInfoM
	{
		UInt64 start;
		UInt64 length;
		int space;
		int flags;
		String ^path;
	};
}
