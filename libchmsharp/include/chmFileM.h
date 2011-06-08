
#pragma once

#include <chm_lib.h>
#include <chmUnitInfoM.h>

using namespace System;

namespace CHMsharp {

	public ref class chmFileM
	{
	public:
		enum class Level
		{
			Normal = 1,
			Meta = 2,
			Special = 4,
			Files = 8,
			Dirs = 16,
			All = 31
		};
		enum class Status
		{
			Failure = 0,
			Continue = 1,
			Success = 2
		};
		delegate Status chmEnumerator(chmFileM ^file, chmUnitInfoM ^ui, Object ^context);

	internal:
		ref struct EnumContext
		{
			chmFileM ^file;
			chmEnumerator ^cb;
			Object ^realctx;
		};

	private:
		String ^_filename;
		struct chmFile *_file;

	public:
		property String ^FileName
		{
			String ^get()
			{
				return _filename;
			}
		}

	private:
		chmFileM(String ^filename, struct chmFile *file);
		~chmFileM();

	public:
		void Close(void);
		static chmFileM ^Open(String ^file);
		bool Enumerate(Level what, chmEnumerator ^e, Object ^context);
		bool EnumerateDirectory(String ^prefix, Level what, chmEnumerator ^e, Object ^context);
		UInt64 RetrieveObject(chmUnitInfoM ^ui, cli::array<Byte> ^%buf, UInt64 addr, UInt64 len);
		bool ResolveObject(String ^objpath, chmUnitInfoM ^%ui);
	};
}
