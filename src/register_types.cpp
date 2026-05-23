#include "register_types.h"
#include "godot_cpp/core/class_db.hpp"
#include "tools/ConstsC/ConstsC.h"
#include "tools/Call/Call.h"
#include "tools/MathHelper/MathHelper.h"
#include "tools/Lighting/Lighting.h"
#include "tools/Lighting/WarpGrid.h"
#include "tools/DHC/DHC.h"
#include "ecs/ECS.h"

using namespace godot;

void open_tower_library_init(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    ClassDB::register_abstract_class<ConstsC>();
    ClassDB::register_abstract_class<Call>();
    ClassDB::register_abstract_class<MathHelper>();
    ClassDB::register_runtime_class<DHC>();
    ClassDB::register_runtime_class<Lighting>();

    ClassDB::register_class<WrapGridFloat32>();
    ClassDB::register_class<WrapGridInt32>();
    ClassDB::register_class<WrapGridVector2>();
    ClassDB::register_class<WrapGridColor>();
    ClassDB::register_class<WrapGridFloat64>();
    ClassDB::register_class<WrapGridInt64>();
    ClassDB::register_class<WrapGridInt16>();
    ClassDB::register_class<WrapGridFloat16>();
    ClassDB::register_class<WrapGridRect2>();
    ClassDB::register_class<WrapGridRect2i>();
    ClassDB::register_class<WrapGridVector2i>();
    
    ClassDB::register_class<World>();
}

void open_tower_uninitialize_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}


extern "C" {
    // Initialization.
    GDExtensionBool GDE_EXPORT
    open_tower_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization) {
        GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(open_tower_library_init);
        init_obj.register_terminator(open_tower_uninitialize_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_CORE);

        return init_obj.init();
    }
}