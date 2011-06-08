
#include "stdafx.h"
#include "chmFileM.h"

#include <malloc.h>

using namespace System::Runtime::InteropServices;

using namespace CHMsharp;

static int _chm_enum_cb(struct chmFile *h, struct chmUnitInfo *ui, void *context)
{
	chmUnitInfoM ^uim = gcnew chmUnitInfoM();
	uim->start = ui->start;
	uim->length = ui->length;
	uim->space = ui->space;
	uim->flags = ui->flags;
	uim->path = gcnew String(ui->path);

	GCHandle gchptr = GCHandle::FromIntPtr(IntPtr(context));
	GCHandle gchctx = GCHandle::FromIntPtr((IntPtr)gchptr.Target);
	chmFileM::EnumContext ^enumctx = (chmFileM::EnumContext^)gchctx.Target;

	return (int)enumctx->cb(enumctx->file, uim, enumctx->realctx);
}

chmFileM::chmFileM(String ^filename, struct chmFile *file)
{
	_filename = filename;
	_file = file;
}

chmFileM::~chmFileM()
{
	if (_file != NULL)
		this->Close();
}

void chmFileM::Close()
{
	chm_close(_file);
	_file = NULL;
}

bool chmFileM::Enumerate(Level what, chmEnumerator ^e, Object ^context)
{
	return this->EnumerateDirectory(nullptr, what, e, context);
}

bool chmFileM::EnumerateDirectory(String ^prefix, Level what, chmEnumerator ^e, Object ^context)
{
	if (_file == NULL)
		throw gcnew InvalidOperationException("The file is closed!");

	EnumContext ^ctx = gcnew EnumContext();
	ctx->file = this;
	ctx->cb = e;
	ctx->realctx = context;

	GCHandle gchctx = GCHandle::Alloc(ctx, GCHandleType::Normal);

	IntPtr ctxptr = GCHandle::ToIntPtr(gchctx);
	GCHandle gchptr = GCHandle::Alloc(ctxptr, GCHandleType::Pinned);

	int result;
	
	if (String::IsNullOrEmpty(prefix)) {
		result = chm_enumerate(
			_file, 
			(int)what, 
			&_chm_enum_cb, 
			GCHandle::ToIntPtr(gchptr).ToPointer());
	} else {
		IntPtr prefixptr = Marshal::StringToHGlobalAnsi(prefix);
		result = chm_enumerate_dir(
			_file, 
			(char*)prefixptr.ToPointer(), 
			(int)what, 
			&_chm_enum_cb, 
			GCHandle::ToIntPtr(gchptr).ToPointer());
		Marshal::FreeHGlobal(prefixptr);
	}

	gchptr.Free();
	gchctx.Free();

	return (bool)result;
}

chmFileM ^chmFileM::Open(String ^file)
{
	IntPtr fileptr = Marshal::StringToHGlobalAnsi(file);
	struct chmFile *result = chm_open((char*)fileptr.ToPointer());
	Marshal::FreeHGlobal(fileptr);

	return gcnew chmFileM(file, result);
}

UInt64 chmFileM::RetrieveObject(chmUnitInfoM ^ui, cli::array<Byte> ^%buf, UInt64 addr, UInt64 len)
{
	if (_file == NULL)
		throw gcnew InvalidOperationException("The file is closed!");

	if (ui->length == 0)
		return 0;

	chmUnitInfo uin;
	uin.start = ui->start;
	uin.length = ui->length;
	uin.space = ui->space;
	uin.flags = ui->flags;

	IntPtr pathptr = Marshal::StringToHGlobalAnsi(ui->path);
	strcpy_s(uin.path, CHM_MAX_PATHLEN, (char*)pathptr.ToPointer());

	unsigned char *bufn = (unsigned char*)_alloca(ui->length);
	UInt64 result = chm_retrieve_object(_file, &uin, bufn, addr, len);

	Marshal::Copy((IntPtr)bufn, buf, 0, len);
	Marshal::FreeHGlobal(pathptr);

	return result;
}

bool chmFileM::ResolveObject(String ^objpath, chmUnitInfoM ^%ui)
{
	IntPtr objpathptr = Marshal::StringToHGlobalAnsi(objpath);
	chmUnitInfo uin;

	int result = chm_resolve_object(_file, (char*)objpathptr.ToPointer(), &uin);
	Marshal::FreeHGlobal(objpathptr);

	if (result == CHM_RESOLVE_FAILURE)
		return false;

	ui->start = uin.start;
	ui->length = uin.length;
	ui->space = uin.space;
	ui->flags = uin.flags;
	ui->path = gcnew String(uin.path);

	return true;
}
