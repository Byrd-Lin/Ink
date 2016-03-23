#include <sstream>
#include "../object.h"
#include "../context.h"
#include "native.h"

namespace ink {

using namespace std;

inline wstring::size_type getRealIndex(Ink_SInt64 index, wstring::size_type size)
{
	while (index < 0) index += size;
	return index;
}

inline wstring::size_type getRealIndex(Ink_NumericValue val, wstring::size_type size)
{
	Ink_SInt64 index = getInt(val);
	while (index < 0) index += size;
	return index;
}

Ink_Object *InkNative_String_Add(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");
	Ink_String *tmp;

	ASSUME_BASE_TYPE(engine, INK_STRING);

	if (!checkArgument(engine, argc, 1)) {
		return NULL_OBJ;
	}

	if ((tmp = getStringVal(engine, context, argv[0])) != NULL) {
		return new Ink_String(engine, as<Ink_String>(base)->getWValue() + tmp->getWValue());
	}

	InkWarn_Invalid_Argument_For_String_Add(engine, argv[0]->type);
	return NULL_OBJ;
}

Ink_Object *InkNative_String_Index(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");
	wstring base_str;
	wstring::size_type index;

	ASSUME_BASE_TYPE(engine, INK_STRING);

	if (!checkArgument(engine, argc, argv, 1, INK_NUMERIC)) {
		InkNote_Method_Fallthrough(engine, "[]", INK_STRING, INK_OBJECT);
		return InkNative_Object_Index(engine, context, argc, argv, this_p);
	}
	base_str = as<Ink_String>(base)->getWValue();
	index = getRealIndex(as<Ink_Numeric>(argv[0])->getValue(), base_str.length());

	return new Ink_String(engine, base_str.substr(index, 1));
}

Ink_Object *InkNative_String_Length(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");

	ASSUME_BASE_TYPE(engine, INK_STRING);

	return new Ink_Numeric(engine, (Ink_SInt64)as<Ink_String>(base)->getWValue().length());
}

Ink_Object *InkNative_String_SubStr(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");
	wstring::size_type offset;
	wstring::size_type length;

	ASSUME_BASE_TYPE(engine, INK_STRING);

	if (!checkArgument(engine, argc, argv, 1, INK_NUMERIC)) {
		return NULL_OBJ;
	}

	wstring origin = as<Ink_String>(base)->getWValue();
	if (argc > 1 && argv[1]->type == INK_NUMERIC) {
		offset = getRealIndex(as<Ink_Numeric>(argv[0])->getValue(), origin.length());
		length = getInt(as<Ink_Numeric>(argv[1])->getValue());
	} else {
		offset = getRealIndex(as<Ink_Numeric>(argv[0])->getValue(), origin.length());
		length = string::npos;
	}

	if (offset >= origin.length()) {
		InkWarn_String_Index_Exceed(engine);
		return NULL_OBJ;
	} else if (!(length == string::npos || offset + length <= origin.length())) {
		InkWarn_Sub_String_Exceed(engine);
		return NULL_OBJ;
	}

	return new Ink_String(engine, origin.substr(offset, length));
}

Ink_Object *InkNative_String_Split(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");
	wstring::size_type i, last;

	ASSUME_BASE_TYPE(engine, INK_STRING);
	if (!checkArgument(engine, argc, argv, 1, INK_STRING)) {
		return NULL_OBJ;
	}

	wstring base_str = as<Ink_String>(base)->getWValue();
	wstring split = as<Ink_String>(argv[0])->getWValue();
	Ink_ArrayValue ret_val = Ink_ArrayValue();
	wstring tmp;

	for (i = 0, last = 0; i < base_str.size(); i++) {
		if (base_str.substr(i, split.length()) == split) {
			ret_val.push_back(new Ink_HashTable(new Ink_String(engine, tmp = (i > last
																	   ? base_str.substr(last, i - last)
																	   : L""))));
			last = i + split.length();
			i += split.length() - 1;
		}
	}

	ret_val.push_back(new Ink_HashTable(new Ink_String(engine, i > last
															   ? base_str.substr(last, i - last)
															   : L"")));

	return new Ink_Array(engine, ret_val);
}

Ink_Object *InkNative_String_Greater(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");

	ASSUME_BASE_TYPE(engine, INK_STRING);
	if (!checkArgument(engine, argc, argv, 1, INK_STRING)) {
		return NULL_OBJ;
	}

	return new Ink_Numeric(engine, as<Ink_String>(base)->getWValue()[0]
								   > as<Ink_String>(argv[0])->getWValue()[0]);
}

Ink_Object *InkNative_String_Less(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");

	ASSUME_BASE_TYPE(engine, INK_STRING);
	if (!checkArgument(engine, argc, argv, 1, INK_STRING)) {
		return NULL_OBJ;
	}

	return new Ink_Numeric(engine, as<Ink_String>(base)->getWValue()[0]
								   < as<Ink_String>(argv[0])->getWValue()[0]);
}

Ink_Object *InkNative_String_GreaterOrEqual(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");

	ASSUME_BASE_TYPE(engine, INK_STRING);
	if (!checkArgument(engine, argc, argv, 1, INK_STRING)) {
		return NULL_OBJ;
	}

	return new Ink_Numeric(engine, as<Ink_String>(base)->getWValue()[0]
								   >= as<Ink_String>(argv[0])->getWValue()[0]);
}

Ink_Object *InkNative_String_LessOrEqual(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");

	ASSUME_BASE_TYPE(engine, INK_STRING);
	if (!checkArgument(engine, argc, argv, 1, INK_STRING)) {
		return NULL_OBJ;
	}

	return new Ink_Numeric(engine, as<Ink_String>(base)->getWValue()[0]
								   <= as<Ink_String>(argv[0])->getWValue()[0]);
}

Ink_Object *InkNative_String_ToString(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	Ink_Object *base = context->searchSlot(engine, "base");
	ASSUME_BASE_TYPE(engine, INK_STRING);
	return base;
}

extern int string_native_method_table_count;
extern InkNative_MethodTable string_native_method_table[];

void Ink_String::Ink_StringMethodInit(Ink_InterpreteEngine *engine)
{
	setSlot_c("+", new Ink_FunctionObject(engine, InkNative_String_Add));
	setSlot_c("<", new Ink_FunctionObject(engine, InkNative_String_Index));
	setSlot_c(">", new Ink_FunctionObject(engine, InkNative_String_Greater));
	setSlot_c("<", new Ink_FunctionObject(engine, InkNative_String_Less));
	setSlot_c(">=", new Ink_FunctionObject(engine, InkNative_String_GreaterOrEqual));
	setSlot_c("<=", new Ink_FunctionObject(engine, InkNative_String_LessOrEqual));
	setSlot_c("[]", new Ink_FunctionObject(engine, InkNative_String_Index));
	setSlot_c("length", new Ink_FunctionObject(engine, InkNative_String_Length));
	setSlot_c("substr", new Ink_FunctionObject(engine, InkNative_String_SubStr));
	setSlot_c("split", new Ink_FunctionObject(engine, InkNative_String_Split));
	setSlot_c("to_str", new Ink_FunctionObject(engine, InkNative_String_ToString));

	return;
}

}
