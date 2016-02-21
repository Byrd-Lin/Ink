#include <string.h>
#include "blueprint.h"
#include "core/object.h"
#include "core/native/general.h"

using namespace ink;

InkMod_ModuleID ink_native_blueprint_mod_id;

void setMathCalMode(Ink_InterpreteEngine *engine, InkMod_Blueprint_Math_CalMode mode)
{
	engine->getEngineComAs<InkMod_Blueprint_EngineCom>(ink_native_blueprint_mod_id)->math_cal_mode = mode;
	return;
}

InkMod_Blueprint_Math_CalMode getMathCalMode(Ink_InterpreteEngine *engine)
{
	return engine->getEngineComAs<InkMod_Blueprint_EngineCom>(ink_native_blueprint_mod_id)->math_cal_mode;
}

Ink_Object *InkMod_Blueprint_Loader(Ink_InterpreteEngine *engine, Ink_ContextChain *context, Ink_ArgcType argc, Ink_Object **argv, Ink_Object *this_p)
{
	if (!checkArgument(engine, argc, 2)) {
		return NULL_OBJ;
	}

	Ink_Object *self = argv[0];
	Ink_Object *apply_to = argv[1];
	Ink_Object **tmp_argv = (Ink_Object **)malloc(sizeof(Ink_Object *) * 2);

	/* blueprint.base */
	Ink_Object *base_pkg = self->getSlot(engine, "base");
	Ink_Object *base_loader = base_pkg->getSlot(engine, "load");

	tmp_argv[0] = base_pkg;
	tmp_argv[1] = apply_to;

	if (base_loader->type == INK_FUNCTION) {
		base_loader->call(engine, context, 2, tmp_argv);
	} else {
		InkWarn_Package_Broken(engine, "blueprint.base");
	}

	/* blueprint.sys */
	apply_to->setSlot_c("sys", self->getSlot(engine, "sys"));
	apply_to->setSlot_c("math", self->getSlot(engine, "math"));

	free(tmp_argv);

	return NULL_OBJ;
}

struct com_cleaner_arg {
	InkMod_ModuleID id;
	com_cleaner_arg(InkMod_ModuleID id)
	: id(id)
	{ }
};

void InkMod_Blueprint_EngineComCleaner(Ink_InterpreteEngine *engine, void *arg)
{
	com_cleaner_arg *tmp = (com_cleaner_arg *)arg;
	delete engine->getEngineComAs<InkMod_Blueprint_EngineCom>(tmp->id);
	delete tmp;
	return;
}

extern "C" {
	void InkMod_Loader(Ink_InterpreteEngine *engine, Ink_ContextChain *context)
	{
		InkMod_Blueprint_EngineCom *engine_com = NULL;

		if (!engine->getEngineComAs<InkMod_Blueprint_EngineCom>(ink_native_blueprint_mod_id)) {
			/* haven't added yet */
			engine_com = new InkMod_Blueprint_EngineCom();
			engine->addEngineCom(ink_native_blueprint_mod_id, engine_com);
			engine->addDestructor(Ink_EngineDestructor(InkMod_Blueprint_EngineComCleaner,
													   new com_cleaner_arg(ink_native_blueprint_mod_id)));
		}

		Ink_Object *blueprint_pkg = addPackage(engine, context, "blueprint",
											   new Ink_FunctionObject(engine, InkMod_Blueprint_Loader));
		Ink_Object *base_pkg = addPackage(engine, blueprint_pkg, "base",
										  new Ink_FunctionObject(engine, InkMod_Blueprint_Base_Loader));
		Ink_Object *sys_pkg = addPackage(engine, blueprint_pkg, "sys",
										 new Ink_FunctionObject(engine, InkMod_Blueprint_System_Loader));
		Ink_Object *math_pkg = addPackage(engine, blueprint_pkg, "math",
										  new Ink_FunctionObject(engine, InkMod_Blueprint_Math_Loader));

		InkMod_Blueprint_Base_bondTo(engine, base_pkg);
		InkMod_Blueprint_System_bondTo(engine, sys_pkg);
		InkMod_Blueprint_Math_bondTo(engine, math_pkg);

		return;
	}

	int InkMod_Init(InkMod_ModuleID id)
	{
		ink_native_blueprint_mod_id = id;
		return 0;
	}
}