#include "register_types.h"
#include "godot_cpp/core/class_db.hpp"
#include "tools/consts_c/consts_c.hpp"
#include "tools/call/call.hpp"
#include "tools/math_helper/math_helper.hpp"
#include "tools/lighting/lighting.hpp"
#include "tools/lighting/warp_grid.hpp"
#include "tools/dhc/dhc.hpp"
#include "tools/my_multi_mesh_c/my_multi_mesh_c.hpp"
#include "tools/particles/particles.hpp"
#include "tools/single_thread_c/single_thread_c.hpp"
#include "ecs/ecs_world.hpp"
#include "tools/syncable/syncable.hpp"
#include "tools/quad_tree_manager/quad_tree_manager.hpp"
#include "tools/predict/predict.hpp"
#include "types/type_c.hpp"
#include "types/bullet_type_c.hpp"
#include "types/unit_type_c.hpp"
#include "ecs/tools/id_pool.hpp"

using namespace godot;

void open_tower_library_init(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    ClassDB::register_abstract_class<ConstsC>();
    ClassDB::register_abstract_class<Call>();
    ClassDB::register_abstract_class<MathHelper>();

    ClassDB::register_class<DHC>();

    ClassDB::register_class<Lighting>();

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

    ClassDB::register_class<MyMultiMeshC>();

    ClassDB::register_abstract_class<ParticleBase>();
    ClassDB::register_class<ParticlesC>();
    ClassDB::register_class<ParticleTorusC>();

    ClassDB::register_class<SingleThreadC>();

    ClassDB::register_class<ecs::ECSWorld>();

    ClassDB::register_class<Syncable>();

    ClassDB::register_class<TreeObject>();
    ClassDB::register_class<QuadTree>();
    ClassDB::register_class<QuadTreeManagerC>();

    ClassDB::register_class<TypeC>();
    ClassDB::register_class<BulletTypeC>();
    ClassDB::register_class<UnitTypeC>();

    ClassDB::register_abstract_class<Predict>();

    ClassDB::register_class<IDPooluint8_t>();
    ClassDB::register_class<IDPooluint16_t>();
    ClassDB::register_class<IDPooluint32_t>();
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