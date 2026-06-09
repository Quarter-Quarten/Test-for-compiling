#include "tools/particles/particles.hpp"

#include <tools/call/call.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>



void ParticleBase::init(int p_count_max, Node* p_controller) {
    count_max = p_count_max;
    controller = p_controller;

    positions.resize(count_max);
    scales.resize(count_max);
    lifetimes.resize(count_max);
    times.resize(count_max);

    free_indexes.reserve(count_max);
    active_indexes.reserve(count_max);
    for (int i = 0; i < count_max; i++) {
        free_indexes.push_back(i);
    }
}

ParticleBase::ParticleBase(int p_count_max, Node* p_controller) {
    init(p_count_max, p_controller);
}

int ParticleBase::create_particle(Vector2 pos, float lifetime, Vector2 size) {
    int idx = get_free_idx();
    if (idx == -1) return -1;

    active_indexes.push_back(idx);

    positions[idx] = pos;
    scales[idx] = size;
    lifetimes[idx] = lifetime;
    times[idx] = 0;
    return idx;
}

void ParticleBase::update(double delta, MyMultiMeshC* multimesh) {
    int ii = 0;
    while (ii < active_indexes.size()) {
        int i = active_indexes[ii];
        
        float lifetime = lifetimes[i];
        float time = times[i];
        _update(i, delta, multimesh, time, lifetime);
        
        multimesh->add_now();
        
        time += delta;
        times[i] = time;
        if (time >= lifetime) {
            delete_particle(i);
            
            active_indexes[ii] = active_indexes[active_indexes.size() - 1];
            active_indexes.pop_back();
        } else {
            ii += 1;
        }
    }
}










void ParticlesC::init(int p_count_max, Node* p_controller) {
    ParticleBase::init(p_count_max, p_controller);

    colors.resize(p_count_max);
    color_tos.resize(p_count_max);
    color_eases.resize(p_count_max);
    
    pos_tos.resize(p_count_max);
    pos_eases.resize(p_count_max);
    
    rotations.resize(p_count_max);
    
    scale_tos.resize(p_count_max);
    scale_eases.resize(p_count_max);
}
ParticlesC::ParticlesC(int p_count_max, Node* p_controller) {
    init(p_count_max, p_controller);
}

void ParticlesC::create(
    Vector2 pos, float rota, float length, float len_rand, float pos_ease,
    Color col_from, Color col_to, float col_ease,
    float lifetime, int count,
    Vector2 scale, float scale_rand_min, Vector2 scale_to, float scale_ease,
    bool radiate, float radiate_angle, float cone
) {
    //UtilityFunctions::print(count);
    for (int i = 0; i < count; i++) {
        int idx = create_particle(pos, lifetime, scale * Call::randf_range(scale_rand_min, 1.0));
        if (idx == -1) return;
        
        float p_rot = (radiate ? i * radiate_angle : Math::deg_to_rad(Call::randf_range(-cone, cone))) + rota;
        
        scale_tos[idx] = scale_to;
        scale_eases[idx] = scale_ease;
        
        colors[idx] = col_from;
        color_tos[idx] = col_to;
        color_eases[idx] = col_ease;
        
        rotations[idx] = p_rot;
        
        pos_eases[idx] = pos_ease;
        pos_tos[idx] = Vector2::from_angle(p_rot) * (Call::randf_range(-len_rand, len_rand) + length) + pos;
    }
}

void ParticlesC::_update(int idx, double delta, MyMultiMeshC* multimesh, float time, float lifetime) {
    float prog = time / lifetime;
    Color color = colors[idx].lerp(color_tos[idx], UtilityFunctions::ease(prog, color_eases[idx]));
    Transform2D transform(
        rotations[idx],
        scales[idx].lerp(scale_tos[idx], UtilityFunctions::ease(prog, scale_eases[idx])),
        0.0,
        (pos_tos[idx] - positions[idx]) * UtilityFunctions::ease(prog, pos_eases[idx]) + positions[idx]
    );
    multimesh->set_now_instance_color(color);
    multimesh->set_now_instance_transform(transform);
    
    _add_light(transform, color);
}

void ParticlesC::_add_light(Transform2D transform, const Color& color) {
    transform[0] *= light_radius_mul.x;
    transform[1] *= light_radius_mul.y;
	controller->call("add_frame_light", color, transform, light_strength);
}








void ParticleTorusC::init(int p_count_max, Node* p_controller) {
    ParticlesC::init(p_count_max, p_controller);
    
    colors.resize(p_count_max);
    color_tos.resize(p_count_max);
    color_eases.resize(p_count_max);
    
    pos_tos.resize(p_count_max);
    pos_eases.resize(p_count_max);
    
    rotations.resize(p_count_max);
    
    scale_tos.resize(p_count_max);
    scale_eases.resize(p_count_max);

    width_froms.resize(p_count_max);
    width_tos.resize(p_count_max);
}
ParticleTorusC::ParticleTorusC(int p_count_max, Node* p_controller) {
    init(p_count_max, p_controller);
}

void ParticleTorusC::create(
    Vector2 pos, float rota, float length, float len_rand, float pos_ease,
    Color col_from, Color col_to, float col_ease,
    float lifetime, int count,
    Vector2 scale, float scale_rand_min, Vector2 scale_to, float scale_ease,
    bool radiate, float radiate_angle, float cone,
    float width_from, float width_to
) {
    for (int i = 0; i < count; i++) {
        int idx = create_particle(pos, lifetime, scale * Call::randf_range(scale_rand_min, 1.0));
        
        float p_rot = (radiate ? i * radiate_angle : Math::deg_to_rad(Call::randf_range(-cone, cone))) + rota;
        
        scale_tos[idx] = scale_to;
        scale_eases[idx] = scale_ease;
        
        colors[idx] = col_from;
        color_tos[idx] = col_to;
        color_eases[idx] = col_ease;
        
        rotations[idx] = p_rot;
        
        pos_eases[idx] = pos_ease;
        pos_tos[idx] = Vector2::from_angle(p_rot) * (Call::randf_range(-len_rand, len_rand) + length) + pos;
        
        width_froms[idx] = width_from;
        width_tos[idx] = width_to;
    }
}

void ParticleTorusC::_update(int idx, double delta, MyMultiMeshC* multimesh, float time, float lifetime) {
    float prog = time / lifetime;
    float size_prog = UtilityFunctions::ease(prog, scale_eases[idx]);

    Vector2 scale = scales[idx].lerp(scale_tos[idx], size_prog);

    int point_count = UtilityFunctions::ceili(scale.x / 2);

    if (point_count <= 0) return;

    Color color = colors[idx].lerp(color_tos[idx], UtilityFunctions::ease(prog, color_eases[idx]));
    float w = UtilityFunctions::lerpf(width_froms[idx], width_tos[idx], size_prog);
    float angle = Math_TAU / point_count;
    Vector2 center = (pos_tos[idx] - positions[idx]) * UtilityFunctions::ease(prog, pos_eases[idx]) + positions[idx];

    Vector2 base_vector = Vector2(scale.x, 0);
    Vector2 last_pos = base_vector + center;
    Transform2D rotation_matrix(angle, Vector2(0.0, 0.0));
    for (int i = 0; i < point_count; i++) {
        base_vector = rotation_matrix.basis_xform(base_vector);
        Vector2 next_pos = base_vector + center;

        multimesh->set_now_instance_color(color);
        Transform2D trans = Transform2D((i + 0.5) * angle + Math_PI / 2, Vector2((next_pos - last_pos).length() + w, w), 0.0, (next_pos + last_pos) / 2);

        multimesh->set_now_instance_transform(trans);
        if (i != point_count - 1) {
            multimesh->add_now();
        }

        _add_light(trans, color);
        last_pos = next_pos;
    }
}


// void WorldLabelsC::init(int p_count_max, Node* p_controller) {
//     ParticleBase::init(p_count_max, p_controller);

//     pos_tos.resize(p_count_max);
// }


// void DamageLabelsC::init(int p_count_max, Node* p_controller, const PackedColorArray& p_colors) {
//     WorldLabelsC::init(p_count_max, p_controller);

//     damages.resize(p_count_max);
//     color_signs.resize(p_count_max);
//     colors = p_colors;
// }
// void DamageLabelsC::create(float damage, Vector2 position, bool crit, bool is_player, bool is_mana) {
//     int idx = create_particle(position, DamageLabelsC::LIFETIME, DamageLabelsC::BASIC_SCALE);
//     if (idx == -1) return;

//     damages[idx] = damage;
//     color_signs[idx] = get_color_sign(damage, crit, is_player, is_mana);
// }
// char DamageLabelsC::get_color_sign(float damage, bool crit, bool is_player, bool is_mana) {
//     if (!is_mana) {
//         if (damage > 0) {
//             if (is_player) {
//                 return crit ? 0 : 1;
//             } else {
//                 return crit ? 2 : 3;
//             }
//         } else {
//             return 4;
//         }
//     } else {
//         return 5;
//     }
// }

// void DamageLabelsC::_update(int idx, double delta, MyMultiMeshC* multimesh, float time, float lifetime) {
//     float prog = time / lifetime;
//     Color color = colors[color_signs[idx]];
//     Transform2D transform(
//         0.0,
//         scales[idx].lerp(scale_tos[idx], UtilityFunctions::ease(prog, DamageLabelsC::POS_EASE)),
//         0.0,
//         positions[idx].lerp(pos_tos[idx], UtilityFunctions::ease(time / DamageLabelsC::LIFETIME, DamageLabelsC::POS_EASE))
//     );

//     multimesh->set_instance_color(color);
//     multimesh->set_instance_transform(transform);
// }

// func create_as_damage_label(value:float, p:Vector2, crit:bool, is_player := false, is_mana := false) -> void:
// 	var abs_value := absf(value)

// 	multi = sqrt(sqrt(abs_value)) / 1.25
// 	text_size = floori(basic_text_size * multi * (1 + int(crit) * 0.3))

// 	text = str(snappedf(abs_value, 0.01))
// 	if !is_mana:
// 		if value > 0:
// 			if is_player:
// 				if crit:
// 					color_index = 0
// 				else: color_index = 1
// 			elif crit:
// 				color_index = 2
// 			else: color_index = 3
// 		else: color_index = 4
// 	else: color_index = 5

// 	pos_ease = 0.3
// 	type = TYPES.DAMAGE

// 	recreate(p, Vector2.from_angle(-PI / 2 + randf() * 0.8 - 0.4) * randf_range(20.0, 40.0) + p, 1.0)
// func create_as_item_label(item_name:String, amount:int, p:Vector2, index:int) -> void:
// 	const item_label_size := 40
// 	text_size = item_label_size

// 	text = item_name + "x" + str(amount)
// 	pos_ease = 0.1
// 	type = TYPES.ITEM

// 	recreate(p, p + Vector2(0, -(item_label_size*LabelControler.scl + 1) * (index+2)), 2.5)

// func create_as_damage_label(value:float, p:Vector2, crit:bool, is_player := false, is_mana := false) -> void:
// 	var abs_value := absf(value)

// 	multi = sqrt(sqrt(abs_value)) / 1.25
// 	text_size = floori(basic_text_size * multi * (1 + int(crit) * 0.3))

// 	text = str(snappedf(abs_value, 0.01))
// 	if !is_mana:
// 		if value > 0:
// 			if is_player:
// 				if crit:
// 					color_index = 0
// 				else: color_index = 1
// 			elif crit:
// 				color_index = 2
// 			else: color_index = 3
// 		else: color_index = 4
// 	else: color_index = 5

// 	pos_ease = 0.3
// 	type = TYPES.DAMAGE

// 	recreate(p, Vector2.from_angle(-PI / 2 + randf() * 0.8 - 0.4) * randf_range(20.0, 40.0) + p, 1.0)
// func create_as_item_label(item_name:String, amount:int, p:Vector2, index:int) -> void:
// 	const item_label_size := 40
// 	text_size = item_label_size

// 	text = item_name + "x" + str(amount)
// 	pos_ease = 0.1
// 	type = TYPES.ITEM

// 	recreate(p, p + Vector2(0, -(item_label_size*LabelControler.scl + 1) * (index+2)), 2.5)


// func get_color() -> Color:
// 	return Vars.world.labels.colors[color_index]

// func reset_timer() -> void:
// 	timer = show_time

// func recreate(p:Vector2, to:Vector2, time:float) -> void:
// 	pos = p
// 	from_pos = p
// 	to_pos = to

// 	show_time = time
// 	timer = time

// 	alpha = 1.0

// func draw(rid:RID) -> void:
// 	var size := Vars.font.get_string_size(text, HORIZONTAL_ALIGNMENT_CENTER, -1, text_size)
// 	var offset := Vector2(-size.x/2, size.y/2)
// 	Vars.font.draw_string_outline(rid, pos/LabelControler.scl + offset, text, HORIZONTAL_ALIGNMENT_CENTER, size.x, text_size, ceili(text_size/5.0), Color.BLACK * alpha)
// 	Vars.font.draw_string(rid, pos/LabelControler.scl + offset, text, HORIZONTAL_ALIGNMENT_CENTER, size.x, text_size, Color.WHITE * alpha)

// func add_instances(drawer: LabelDrawer, t_size: int, txt: String, atlas_size_x: float) -> void:
// 	var scl := float(t_size) / basic_text_size
// 	var l := txt.length()
// 	var center_of_first_x := pos.x - l * LabelControler.draw_size.x * scl * 0.5

// 	for i in range(l):
// 		var offset := 0.0
// 		var code: int = txt.unicode_at(i)
// 		offset =  (code - 48.0) if (code >= 48 and code <= 57) else 10.0
// 		offset *= LabelControler.char_size.x

// 		var uv_offset := offset / atlas_size_x
		
// 		drawer.set_instance_custom(Color(uv_offset, alpha, color_index, 0))
// 		drawer.set_instance_transform(Transform2D(0.0, Vector2(scl, scl), 0.0, Vector2(center_of_first_x + i * LabelControler.draw_size.x * scl, pos.y)))
// 		drawer.add_now()









void ParticleBase::_bind_methods() {
    ClassDB::bind_method(D_METHOD("update", "delta", "multimesh"), &ParticleBase::update);
}

void ParticlesC::_bind_methods() {
    ClassDB::bind_method(D_METHOD("init", "p_count_max", "p_controller"), &ParticlesC::init);
    ClassDB::bind_method(D_METHOD("create", "pos", "rota", "length", "len_rand", "pos_ease", "col_from", "col_to", "col_ease", "lifetime", "count", "scale", "scale_rand_min", "scale_to", "scale_ease", "radiate", "radiate_angle", "cone"), &ParticlesC::create);
}

void ParticleTorusC::_bind_methods() {
    ClassDB::bind_method(D_METHOD("init", "p_count_max", "p_controller"), &ParticleTorusC::init);
    ClassDB::bind_method(D_METHOD("create", "pos", "rota", "length", "len_rand", "pos_ease", "col_from", "col_to", "col_ease", "lifetime", "count", "scale", "scale_rand_min", "scale_to", "scale_ease", "radiate", "radiate_angle", "cone", "width_from", "width_to"), &ParticleTorusC::create);
}

// void WorldLabelsC::_bind_methods() {
//     ClassDB::bind_method(D_METHOD("init", "p_count_max", "p_controller"), &WorldLabelsC::init);
// }
// void DamageLabelsC::_bind_methods() {
//     ClassDB::bind_method(D_METHOD("init", "p_count_max", "p_controller", "p_colors"), &DamageLabelsC::init);
//     ClassDB::bind_method(D_METHOD("create", "value", "position", "crit", "is_player", "is_mana"), &DamageLabelsC::create);
// }
