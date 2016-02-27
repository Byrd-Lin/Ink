#include <stdio.h>
#include <string>
#include "core/general.h"
#include "core/debug.h"
#include "core/native/native.h"
#include "core/interface/engine.h"
#include "includes/switches.h"
#include "io.h"

using namespace ink;
using namespace std;

extern InkMod_ModuleID ink_native_file_mod_id;

Ink_TypeTag getFilePointerType(Ink_InterpreteEngine *engine)
{
	return engine->getEngineComAs<Ink_TypeTag>(ink_native_file_mod_id)[0];
}

inline Ink_NumericValue getNumVal(Ink_Object *num)
{
	if (num->type == INK_NUMERIC) {
		return as<Ink_Numeric>(num)->value;
	}
	return 0;
}

Ink_Object *InkNative_File_Exist(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	if (!checkArgument(engine, argc, argv, 1, INK_STRING)) {
		return NULL_OBJ;
	}
	string tmp = getStringVal(engine, context, argv[0])->getValue();

	return new Ink_Numeric(engine, !access(tmp.c_str(), 0));
}

Ink_Object *InkNative_File_Remove(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	if (!checkArgument(engine, argc, argv, 1, INK_STRING)) {
		return NULL_OBJ;
	}
	string tmp = getStringVal(engine, context, argv[0])->getValue();

	return new Ink_Numeric(engine, !remove(tmp.c_str()));
}

Ink_Object *InkNative_File_Constructor(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *ret;
	FILE *fp = NULL;
	string path, mode;

	if (checkArgument(false, argc, argv, 2, INK_STRING, INK_STRING)) {
		path = getStringVal(engine, context, argv[0])->getValue();
		mode = getStringVal(engine, context, argv[1])->getValue();

		fp = fopen(path.c_str(), mode.c_str());
		if (!fp) {
			InkWarn_Failed_Open_File(engine, path.c_str());
		}
	} else if (checkArgument(false, argc, argv, 1, INK_STRING)) {
		path = getStringVal(engine, context, argv[0])->getValue();

		fp = fopen(path.c_str(), "r");
		if (!fp) {
			InkWarn_Failed_Open_File(engine, path.c_str());
		}
	} else if (checkArgument(false, argc, argv, 1, FILE_POINTER_TYPE)) {
		fp = as<Ink_FilePointer>(argv[0])->fp;
	}

	context->getLocal()->context->setSlot_c("this", ret = new Ink_FilePointer(engine, fp));
	
	return ret;
}

Ink_Object *InkNative_File_Close(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");
	FILE *tmp;

	ASSUME_BASE_TYPE(engine, FILE_POINTER_TYPE);

	tmp = as<Ink_FilePointer>(base)->fp;
	if (tmp) {
		if (NOT_STDIO(tmp))
			as<Ink_FilePointer>(base)->fp = NULL;
		fclose(tmp);
	} else {
		InkWarn_IO_Uninitialized_File_Pointer(engine);
	}

	return NULL_OBJ;
}

Ink_Object *InkNative_File_PutString(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");
	FILE *tmp;

	ASSUME_BASE_TYPE(engine, FILE_POINTER_TYPE);
	if (!checkArgument(engine, argc, argv, 1, INK_STRING)) {
		return NULL_OBJ;
	}

	tmp = as<Ink_FilePointer>(base)->fp;
	string tmp_str = getStringVal(engine, context, argv[0])->getValue();
	if (tmp) fputs(tmp_str.c_str(), tmp);
	else {
		InkWarn_IO_Uninitialized_File_Pointer(engine);
	}

	return NULL_OBJ;
}

Ink_Object *InkNative_File_PutC(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");
	FILE *tmp;
	char ch;
	string tmp_str;

	ASSUME_BASE_TYPE(engine, FILE_POINTER_TYPE);
	if (checkArgument(false, argc, argv, 1, INK_STRING)) {
		tmp_str = getStringVal(engine, context, argv[0])->getValue();
		ch = tmp_str.c_str()[0];
	} else if (checkArgument(engine, argc, argv, 1, INK_NUMERIC)) {
		ch = getNumVal(argv[0]);
	} else {
		return NULL_OBJ;
	}

	tmp = as<Ink_FilePointer>(base)->fp;
	if (tmp) fputc(ch, tmp);
	else {
		InkWarn_IO_Uninitialized_File_Pointer(engine);
	}

	return NULL_OBJ;
}

Ink_Object *InkNative_File_GetString(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");
	FILE *tmp;
	char buffer[FILE_GETS_BUFFER_SIZE] = { '\0' };

	ASSUME_BASE_TYPE(engine, FILE_POINTER_TYPE);

	tmp = as<Ink_FilePointer>(base)->fp;
	if (tmp) {
		return new Ink_String(engine, string(fgets(buffer, FILE_GETS_BUFFER_SIZE, tmp)));
	}

	InkWarn_IO_Uninitialized_File_Pointer(engine);

	return NULL_OBJ;
}

Ink_Object *InkNative_File_GetC(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");
	FILE *tmp;

	ASSUME_BASE_TYPE(engine, FILE_POINTER_TYPE);

	tmp = as<Ink_FilePointer>(base)->fp;
	if (tmp) {
		return new Ink_Numeric(engine, fgetc(tmp));
	}

	InkWarn_IO_Uninitialized_File_Pointer(engine);

	return NULL_OBJ;
}

#if defined(INK_PLATFORM_LINUX)

Ink_Object *InkNative_File_GetCh(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	char ch;
	ttyMode mode_back = getttyMode(); // back up tty mode

	closettyBuffer();
	// close buffering
	ch = fgetc(stdin);

	// restore tty mode
	setttyMode(mode_back);

	return new Ink_Numeric(engine, ch);
}

#endif

Ink_Object *InkNative_File_ReadAll(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");
	FILE *tmp;
	size_t len;
	char *buffer;
	Ink_Object *ret;

	ASSUME_BASE_TYPE(engine, FILE_POINTER_TYPE);

	tmp = as<Ink_FilePointer>(base)->fp;

	if (tmp) {
		fseek(tmp, 0L, SEEK_END);
		len = ftell(tmp);
		buffer = (char *)malloc(len + 1);

		fseek(tmp, 0L, SEEK_SET);
		if (!fread(buffer, len, 1, tmp)) {
			InkWarn_File_Failed_Read_File(engine);
			return NULL_OBJ;
		}
		buffer[len] = '\0';

		ret = new Ink_String(engine, string(buffer));
		free(buffer);

		return ret;
	}

	InkWarn_IO_Uninitialized_File_Pointer(engine);

	return NULL_OBJ;
}

Ink_Object *InkNative_File_Flush(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");
	FILE *tmp;

	ASSUME_BASE_TYPE(engine, FILE_POINTER_TYPE);

	tmp = as<Ink_FilePointer>(base)->fp;
	if (tmp)
		fflush(tmp);
	else
		InkWarn_IO_Uninitialized_File_Pointer(engine);

	return NULL_OBJ;
}

Ink_Object *InkNative_File_Reopen(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");
	FILE *tmp = NULL;
	string path, mode;

	if (!checkArgument(engine, argc, argv, 2, INK_STRING)) {
		return NULL_OBJ;
	}

	ASSUME_BASE_TYPE(engine, FILE_POINTER_TYPE);

	tmp = as<Ink_FilePointer>(base)->fp;
	if (tmp) {
		path = as<Ink_String>(argv[0])->getValue();
		mode = as<Ink_String>(argv[1])->getValue();
		tmp = freopen(path.c_str(), mode.c_str(), tmp);
	} else
		InkWarn_IO_Uninitialized_File_Pointer(engine);

	return new Ink_Numeric(engine, tmp != NULL);
}

void Ink_FilePointer::Ink_FilePointerMethodInit(Ink_InterpreteEngine *engine)
{
	setSlot_c("close", new Ink_FunctionObject(engine, InkNative_File_Close));
	setSlot_c("puts", new Ink_FunctionObject(engine, InkNative_File_PutString));
	setSlot_c("putc", new Ink_FunctionObject(engine, InkNative_File_PutC));
	setSlot_c("gets", new Ink_FunctionObject(engine, InkNative_File_GetString));
	setSlot_c("getc", new Ink_FunctionObject(engine, InkNative_File_GetC));
	setSlot_c("read", new Ink_FunctionObject(engine, InkNative_File_ReadAll));
	setSlot_c("flush", new Ink_FunctionObject(engine, InkNative_File_Flush));
	setSlot_c("reopen", new Ink_FunctionObject(engine, InkNative_File_Reopen));

	return;
}

void InkMod_File_bondTo(Ink_InterpreteEngine *engine, Ink_Object *bondee)
{
	bondee->setSlot_c("File", new Ink_FunctionObject(engine, InkNative_File_Constructor));
	bondee->setSlot_c("file_exist", new Ink_FunctionObject(engine, InkNative_File_Exist));
	bondee->setSlot_c("file_remove", new Ink_FunctionObject(engine, InkNative_File_Remove));
#if defined(INK_PLATFORM_LINUX)
	bondee->setSlot_c("getch", new Ink_FunctionObject(engine, InkNative_File_GetCh)); // no buffering getc
#endif

	return;
}

Ink_Object *InkMod_File_Loader(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	if (!checkArgument(engine, argc, 2)) {
		return NULL_OBJ;
	}

	Ink_Object *apply_to = argv[1];

	InkMod_File_bondTo(engine, apply_to);

	return NULL_OBJ;
}