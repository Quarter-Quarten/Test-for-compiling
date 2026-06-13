#pragma once

#include "tools/consts_c/consts_c.hpp"
#include "tools/call/call.hpp"
#include "ecs/lib/flecs.h"
#include "ecs/components.hpp"

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <string>
#include <vector>
#include <cmath>
#include <numbers>

using namespace godot;

namespace ecs {
    class ECSWorld;

    static void register_systems(flecs::world &world, ECSWorld* ecs_world) {
        // 移动系统
        world.system<LastPosition*, Position, const Velocity>("MoveSystem")
            .kind(flecs::OnUpdate)
            .each([&world](LastPosition* lp, Position& p, const Velocity& v) {
                if (lp) lp->value = p.value;
                p.value += v.value * world.delta_time();
            });

        // 加速度系统
        world.system<Velocity, const Acceleration>("AccelerationSystem")
            .kind(flecs::OnUpdate)
            .interval(ConstsC::get_tick_time())
            .each([](Velocity& v, const Acceleration& a) {
                if (v.value.x || v.value.y) {
                    const float dt = static_cast<real_t>(ConstsC::get_tick_time());
                    v.value *= (v.value.length() * a.multiplier + (a.value * dt)) / v.value.length();
                }
            });

        // 旋转系统
        world.system<Rotation, const RotateSpeed>("RotateSystem")
            .kind(flecs::OnUpdate)
            .each([&world](Rotation& r, const RotateSpeed& rs) {
                const float dt = world.delta_time();
                r.value += rs.value * static_cast<real_t>(dt);
            });
        
        // 回血系统
        world.system<Health, const TickHealAmount>("RotateSystem")
            .interval(ConstsC::get_tick_time())
            .kind(flecs::OnUpdate)
            .each([](Health& h, const TickHealAmount& tha) {
                h.current += tha.value;
            });

        // Lifetime系统
        world.system<Lifetime>("LifetimeSystem")
            .kind(flecs::OnUpdate)
            .each([&](flecs::entity e, Lifetime& l) {
                const float dt = world.delta_time();
                l.age += dt;
                if (l.age >= l.lifetime) {
                    e.destruct();
                }
            });

        // 追踪系统
        world.system<const Lifetime, const Position, const HomingRange, const Team, TargetPosition, const HomingDelay*>("HomingTargetSystem")
            .interval(ConstsC::get_tick_time() / 6)
            // .multi_threaded(false)
            .kind(flecs::OnUpdate)
            .each([&world](const Lifetime& l, const Position& p, const HomingRange& hr, const Team& t, TargetPosition& tp, const HomingDelay* hd) {
                const float dt = world.delta_time();
                const QuadTreeComp& qt = world.get<QuadTreeComp>();
                if (!hd || l.age >= hd->value) {
                    qt.ptr->retrieve_circle_fn(p.value, hr.value, -1, t.value, [&tp](Object* obj) mutable -> bool {
                        tp.value = obj->call("get_pos");
                        return true;
                    });
                }
            });
        world.system<const HomingPower, const Lifetime, const Position, const TargetPosition, Velocity, const HomingDelay*>("HomingSystem")
            .interval(ConstsC::get_tick_time())
            .kind(flecs::OnUpdate)
            .each([&world](const HomingPower& h, const Lifetime& l, const Position& p, const TargetPosition& tp, Velocity& v, const HomingDelay* hd) {
                const float dt = ConstsC::get_tick_time();
                if (!hd || l.age >= hd->value) {
                    // const float angle_dif_abs = std::fmod((tp.value - p.value).angle() - v.value.angle(), Math_PI);
                    // float rotate_dir = angle_dif_abs > Math_PI * 0.5 ? -1.0 : 1.0;
                    // v.value = v.value.rotated(rotate_dir * std::min(h.value * dt, angle_dif_abs));
                }
            });
        
        
            
        
        /* 子弹 */ {
            // 销毁
            world.observer<const BulletTypeComp>("BulletDestroyObserver")
                .event(flecs::OnRemove)
                .with<const Position>()
                .with<const Rotation>()
                .with<const Team>()
                .with<BulletCollision>()
                .with<const BulletDamage>()
                .each([&](flecs::entity e, const BulletTypeComp& bt) {
                    const Position& p = e.get<const Position>();
                    const Rotation& r = e.get<const Rotation>();
                    const Team& t = e.get<const Team>();
                    const BulletCollision& bc = e.get<const BulletCollision>();
                    const BulletDamage& bd = e.get<const BulletDamage>();
                    const TargetPosition* tp = e.try_get<const TargetPosition>();

                    Object* vars = Call::get_vars();
                    Object* world_node = Call::get_world();

                    const BulletTypeC* bullet_type = bt.bullet_type;

                    // 特效
                    bullet_type->get_despawn_effect().call("at", p.value, r.value);

                    // 震动
                    const float shake = static_cast<float>(bt.bullet_type->get_shake());
                    if (shake > 0.0f && world_node) {
                        Object* camera = Object::cast_to<Object>(world_node->get("camera"));
                        if (camera) camera->call("shake_from", shake, p.value);
                    }

                    // 声音
                    if (bullet_type->get_despawn_sound()) {
                        Object* sounds = Object::cast_to<Object>(world_node->get("sounds"));
                        if (sounds) sounds->call("at", bullet_type->get_despawn_sound(), p.value, bullet_type->get_despawn_sound_volume());
                    }

                    // 分裂子弹
                    if (bullet_type->get_frag_bullet()) {
                        Object* bullets = Object::cast_to<Object>(world_node->get("bullets"));
                        if (bullets) {
                            for (int i = 0; i < bullet_type->get_frag_bullets(); i++) {
                                bullets->call("create_by_rot",
                                    t.value, p.value, r.value + bullet_type->get_frag_angle() + UtilityFunctions::randf_range(-bullet_type->get_frag_angle_random(), bullet_type->get_frag_angle_random()),
                                    bullet_type->get_frag_bullet(), Variant(), bd.damage_multi, bullet_type->get_frag_speed_from() < 1.0 ? UtilityFunctions::randf_range(bullet_type->get_frag_speed_from(), 1.0) : 1.0
                                );
                            }
                        }
                    }


                    const float sr = static_cast<float>(bt.bullet_type->get_splash_range());
                    const float sd = static_cast<float>(bt.bullet_type->get_splash_damage());
                    const float sk = static_cast<float>(bt.bullet_type->get_splash_knockback());
                    
                    const float crit_chance = static_cast<float>(bt.bullet_type->get_crit_chance());
                    const float knockback = static_cast<float>(bt.bullet_type->get_knockback());
                    
                    // 单位溅射
                    const QuadTreeComp& qt = world.get<QuadTreeComp>();
                    qt.ptr->retrieve_circle_fn(p.value, sr, -1, t.value,
                        [&](Object* unit_obj) -> bool {
                            if (!unit_obj) return false;
                            if (int(unit_obj->get("team")) == t.value) return false;

                            float dist = Vector2(unit_obj->get("global_position")).distance_to(p.value);
                            float d = sd * ((4.0f + sr - dist) / sr);
                            bool crit = crit_chance > 0.0f &&
                                static_cast<float>(std::rand()) / RAND_MAX <= crit_chance;
                            if (crit) {
                                static constexpr float CRIT_DMG_MULTI = 4.0f;
                                d = (CRIT_DMG_MULTI * (std::max(crit_chance - 1.0f, 0.0f) + 1.0f)) * d;
                            }

                            if (knockback > 0.0f) {
                                unit_obj->call("knockback", knockback, p.value);
                            }

                            d *= bd.damage_multi;

                            bool is_flying = false;
                            Variant type_var = unit_obj->get("type");
                            if (type_var.get_type() != Variant::NIL) {
                                Object* u_type = Object::cast_to<Object>(type_var);
                                if (u_type) {
                                    Variant flying_var = u_type->get("flying");
                                    if (flying_var.get_type() == Variant::BOOL) {
                                        is_flying = flying_var;
                                    }
                                }
                            }
                            float type_multi = is_flying ? static_cast<float>(bt.bullet_type->get_air_unit_damage_multi()) : static_cast<float>(bt.bullet_type->get_ground_unit_damage_multi());

                            unit_obj->call("damaged", d * type_multi, Variant(), bd.armor_pierce, true, crit);
                            return false;
                        });

                    // 方块溅射
                    if (vars) {
                        Call::circle_collision(p.value, sr, [&](Object* block) -> bool {
                            if (!block) return false;
                            if (int(block->get("team")) == t.value) return false;

                            float dist = (Vector2(block->get("pos")) * ConstsC::get_tile_size() +
                                ConstsC::get_half_tile_size()).distance_to(p.value);
                            float d = sd * ((4.0f + sr - dist) / sr);
                            bool crit = crit_chance > 0.0f &&
                                static_cast<float>(std::rand()) / RAND_MAX <= crit_chance;
                            if (crit) {
                                static constexpr float CRIT_DMG_MULTI = 4.0f;
                                d = (CRIT_DMG_MULTI * (std::max(crit_chance - 1.0f, 0.0f) + 1.0f)) * d;
                            }
                            d *= bd.damage_multi;
                            block->call("damaged", d, Variant(), bd.armor_pierce, crit);
                            return false;
                        });
                    }
                });


            // 辅助：子弹击中单位
            static auto _bullet_hit_unit = [](Object* unit_obj, const BulletTypeComp& bt, const Velocity& v, BulletCollision& bc, const BulletDamage& bd) {
            	const float crit_chance = static_cast<float>(bt.bullet_type->get_crit_chance());
            	const float knockback = static_cast<float>(bt.bullet_type->get_knockback());
            	
                // 重复碰撞检测
                int64_t id = unit_obj->get_instance_id();
                if (bc.collided.has(id)) return false;

                // 伤害计算： (damage + (vel_len / speed) * cutting_damage) * damage_multi
                float vel_len = v.value.length();
                float base_speed = bt.speed > 0.0f ? bt.speed : 1.0f;
                float d = (bd.damage + (vel_len / base_speed) * bd.cutting_damage) * bd.damage_multi;

                // 暴击判定与计算
                bool is_crit = crit_chance > 0.0f &&
                    static_cast<float>(std::rand()) / RAND_MAX <= crit_chance;
                if (is_crit) {
                    static constexpr float CRIT_DMG_MULTI = 4.0f;
                    d = (CRIT_DMG_MULTI * (std::max(crit_chance - 1.0f, 0.0f) + 1.0f)) * d;
                }

                // 击退
                if (knockback > 0.0f) {
                    unit_obj->call("knockback_by_dir", knockback, v.value);
                }

                // 判断飞行/地面单位
                bool is_flying = false;
                Variant type_var = unit_obj->get("type");
                if (type_var.get_type() != Variant::NIL) {
                    Object* u_type = Object::cast_to<Object>(type_var);
                    if (u_type) {
                        Variant flying_var = u_type->get("flying");
                        if (flying_var.get_type() == Variant::BOOL) {
                            is_flying = flying_var;
                        }
                    }
                }
                float type_multi = is_flying
                    ? static_cast<float>(bt.bullet_type->get_air_unit_damage_multi())
                    : static_cast<float>(bt.bullet_type->get_ground_unit_damage_multi());

                // 应用状态效果(GDScript)
                // bt.bullet_type->_apply_bullet_status
                // if (ctx.gd_type_index >= 0) {
                //     Object* vars = Call::get_vars();
                //     if (vars) {
                //         vars->call("_apply_bullet_status", ctx.gd_type_index,
                //             unit_obj, ctx.damage_multi, is_crit);
                //     }
                // }

                // 造成伤害
                unit_obj->call("damaged", d * type_multi, Variant(),
                    bd.armor_pierce, true, is_crit);

                // 记录碰撞 & 减少穿透
                bc.collided.push_back(id);
                (bc.pierce_remaining)--;
                // bt.hit_any_unit = true;

                // 穿透耗尽时停止迭代
                if (bc.pierce_remaining == -1) return true;
                // 还可继续穿透其它单位
                return false;
            };
            

            // 子弹碰撞系统（碰撞检测 + 伤害 + 溅射/销毁）
            world.system<const Position, const LastPosition, const Velocity, const Rotation, const Team, BulletCollision, const SizeValue,
                const BulletTypeComp, const BulletDamage, const BulletOrigin*>("BulletCollisionSystem")
                .kind(flecs::OnUpdate)
                .without<PointBullet>()
                //.multi_threaded(false)
                .each([&world, &ecs_world](flecs::entity e, const Position& p, const LastPosition& lp, const Velocity& v, const Rotation& r, const Team& t,
                    BulletCollision& bc, const SizeValue& s, const BulletTypeComp& bt, const BulletDamage& bd, const BulletOrigin* origin)
                {
                    if (!bc.collide || !bc.hittable) return;

                    const QuadTreeComp& qt = world.get<QuadTreeComp>();
                    Object* vars = Call::get_vars();
                    if (!vars) return;

                    // 计算含 cutting_damage 的伤害值（用于方块）
                    const float vel_len = v.value.length();
                    // UtilityFunctions::print(bd.cutting_damage, vel_len / bt.speed);
                    const float cutting_damage = bt.speed ? ((vel_len / bt.speed) * bd.cutting_damage) : 0.0;
                    const float hit_damage = (bd.damage + cutting_damage) * bd.damage_multi;

                    // === 单位碰撞 ===
                    bool hit_unit = false;
                    if (bt.bullet_type->get_collide_unit()) {
                        // 调用四叉树做圆形碰撞检测 + 回调
                        qt.ptr->retrieve_circle_fn(p.value, s.value, -1, t.value,
                            [&](Object* unit_obj) -> bool {
                                hit_unit = true;
                                return _bullet_hit_unit(unit_obj, bt, v, bc, bd);
                            });
                    }

                    // === 方块碰撞 ===
                    bool hit_block = false;
                    if (bt.bullet_type->get_collide_block() && !hit_unit && Call::pos_to_tile(p.value) != Call::pos_to_tile(lp.value)) {
                        Dictionary block_inst = vars->get("block_instances");
                        Variant block_v = block_inst.get(Call::pos_to_tile(p.value), Variant());
                        if (block_v.get_type() != Variant::NIL && static_cast<int>(block_v.get("team")) != t.value) {
                            Object* block = block_v;
                            int64_t id = block->get_instance_id();
                            if (!bc.collided.has(id)) {
                                bc.collided.push_back(id);
                                block->call("damaged", hit_damage * static_cast<float>(bt.bullet_type->get_block_damage_multi()),
                                    Variant(), bd.armor_pierce);
                                hit_block = true;
                                (bc.pierce_remaining)--;
                            }
                        }
                    }

                    // hit_once：首次碰撞检测后禁用后续帧
                    if (bt.bullet_type->get_hit_once()) {
                        bc.collide = false;
                    }

                    // 处理碰撞结果（销毁或 pierce）
                    if (hit_unit || hit_block) {
                        if (bc.pierce_remaining == -1) {
                            e.destruct();
                        }
                    }

                    // 距离检测 弃
                    // if (origin && bt.r < 999998.0f) {
                    //     float max_dist = bt.r;
                    //     if ((p.value - origin->value).length_squared() > max_dist * max_dist) {
                    //         ecs_world->_destroy_bullet(e, p.value, v.value.angle(), bd.damage_multi);
                    //     }
                    // }
                });

            // 子弹绘制
            world.system<const Position, const Rotation, const ColorComp, const SizeValue, const BulletTypeComp, const ScaleVector*>("BulletDrawPrepare")
                .kind(flecs::OnUpdate)
                .multi_threaded(false)
                .each([&world](const Position& p, const Rotation& r, const ColorComp& c, const SizeValue& s, const BulletTypeComp& bt, const ScaleVector* scl) {
                    const BulletDrawer& bullet_drawer = world.get<BulletDrawer>();
                    const Vector2 scale = s.value * 2.0f * (scl ? scl->value : Vector2(1, 1));

                    bullet_drawer.ptr->set_now_instance_transform(Transform2D(r.value, scale, 0.0, p.value));
                    bullet_drawer.ptr->set_now_instance_color(c.value);
                    bullet_drawer.ptr->set_now_instance_custom(Color(114514.0f, 1919.0f, 0.0f, 0.0f));
                    bullet_drawer.ptr->add_now();
                });
            world.system<>("BulletDrawSubmit")
                .kind(flecs::OnUpdate)
                .multi_threaded(false)
                .each([&world]() {
                    const BulletDrawer& bullet_drawer = world.get<BulletDrawer>();
                    bullet_drawer.ptr->update();
                });
            
            // Scale变动
            world.system<const Lifetime, ScaleVector, const ScaleVectorFromTo>("Scaler")
                .kind(flecs::OnUpdate)
                .without<PointBullet>()
                .each([&world](const Lifetime& l, ScaleVector& scl, const ScaleVectorFromTo& svft) {
                    scl.value = svft.scale_from.lerp(svft.scale_to, l.age / l.lifetime);
                });
            // PointBullet Scale变动
            world.system<const Lifetime, ScaleVector, const ScaleVectorFromTo, PointBullet>("PointBullerScaler")
                .kind(flecs::OnUpdate)
                .each([&world](const Lifetime& l, ScaleVector& scl, const ScaleVectorFromTo& svft, PointBullet) {
                    const float prog = sinf(l.age / l.lifetime * 3.14159265f);
                    scl.value = svft.scale_from.lerp(svft.scale_to, prog);
                });
        } // 子弹

        /* 粒子 */ {
            // Circle粒子绘制
            world.system<const Lifetime, const ParticleInfo, CircleParticle>("CircleParticleDraw")
                .kind(flecs::OnUpdate)
                .each([&world](flecs::entity e, const Lifetime& l, const ParticleInfo& pi, CircleParticle) {
                    float prog = l.age / pi.lifetime;
                    Color color = pi.color_from.lerp(pi.color_to, UtilityFunctions::ease(prog, pi.color_ease));
                    Vector2 pos = (pi.pos_to - pi.pos_from) * UtilityFunctions::ease(prog, pi.pos_ease) + pi.pos_from;
                    Vector2 scale = pi.scale_from.lerp(pi.scale_to, UtilityFunctions::ease(prog, pi.scale_ease));
                    Transform2D transform(pi.rotation, scale, 0.0, pos);

                    const EffectCircleDrawer& d = world.get<EffectCircleDrawer>();
                    d.ptr->set_instance_color(color, pi.id);
                    d.ptr->set_instance_transform(transform, pi.id);

                    const PointLightDrawer& pld = world.get<PointLightDrawer>();
                    Transform2D lt = transform;
                    lt[0] *= 2.5f; lt[1] *= 2.5f;
                    Color lc = color; lc.a *= 0.3f;
                    pld.ptr->set_instance_color(lc, pi.id);
                    pld.ptr->set_instance_transform(lt, pi.id);
                });

            // Triangle粒子绘制
            world.system<const Lifetime, const ParticleInfo, TriangleParticle>("TriangleParticleDraw")
                .kind(flecs::OnUpdate)
                .each([&world](flecs::entity e, const Lifetime& l, const ParticleInfo& pi, TriangleParticle) {
                    float prog = l.age / pi.lifetime;
                    Color color = pi.color_from.lerp(pi.color_to, UtilityFunctions::ease(prog, pi.color_ease));
                    Vector2 pos = (pi.pos_to - pi.pos_from) * UtilityFunctions::ease(prog, pi.pos_ease) + pi.pos_from;
                    Vector2 scale = pi.scale_from.lerp(pi.scale_to, UtilityFunctions::ease(prog, pi.scale_ease));
                    Transform2D transform(pi.rotation, scale, 0.0, pos);

                    const EffectTriangleDrawer& d = world.get<EffectTriangleDrawer>();
                    d.ptr->set_instance_color(color, pi.id);
                    d.ptr->set_instance_transform(transform, pi.id);

                    const PointLightDrawer& pld = world.get<PointLightDrawer>();
                    Transform2D lt = transform;
                    lt[0] *= 2.0f; lt[1] *= 2.0f;
                    Color lc = color; lc.a *= 0.3f;
                    pld.ptr->set_instance_color(lc, pi.id);
                    pld.ptr->set_instance_transform(lt, pi.id);
                });

            // Rect粒子绘制
            world.system<const Lifetime, const ParticleInfo, RectParticle>("RectParticleDraw")
                .kind(flecs::OnUpdate)
                .each([&world](flecs::entity e, const Lifetime& l, const ParticleInfo& pi, RectParticle) {
                    float prog = l.age / pi.lifetime;
                    Color color = pi.color_from.lerp(pi.color_to, UtilityFunctions::ease(prog, pi.color_ease));
                    Vector2 pos = (pi.pos_to - pi.pos_from) * UtilityFunctions::ease(prog, pi.pos_ease) + pi.pos_from;
                    Vector2 scale = pi.scale_from.lerp(pi.scale_to, UtilityFunctions::ease(prog, pi.scale_ease));
                    Transform2D transform(pi.rotation, scale, 0.0, pos);

                    const EffectRectDrawer& d = world.get<EffectRectDrawer>();
                    d.ptr->set_instance_color(color, pi.id);
                    d.ptr->set_instance_transform(transform, pi.id);

                    const PointLightDrawer& pld = world.get<PointLightDrawer>();
                    Transform2D lt = transform;
                    lt[0] *= 2.0f; lt[1] *= 2.0f;
                    Color lc = color; lc.a *= 0.3f;
                    pld.ptr->set_instance_color(lc, pi.id);
                    pld.ptr->set_instance_transform(lt, pi.id);
                });

            // Torus粒子绘制（多段环形）
            world.system<const Lifetime, const ParticleInfo, const TorusParticleInfo, TorusParticle>("TorusParticleDraw")
                .kind(flecs::OnUpdate)
                .each([&world](flecs::entity e, const Lifetime& l, const ParticleInfo& pi, const TorusParticleInfo& ti, TorusParticle) {
                    float prog = l.age / pi.lifetime;
                    float size_prog = UtilityFunctions::ease(prog, pi.scale_ease);
                    Vector2 scale = pi.scale_from.lerp(pi.scale_to, size_prog);
                    int point_count = UtilityFunctions::ceili(scale.x / 2.0f);
                    if (point_count <= 0) return;

                    Color color = pi.color_from.lerp(pi.color_to, UtilityFunctions::ease(prog, pi.color_ease));
                    float w = UtilityFunctions::lerpf(ti.width_from, ti.width_to, size_prog);
                    float angle = Math_TAU / point_count;
                    Vector2 center = (pi.pos_to - pi.pos_from) * UtilityFunctions::ease(prog, pi.pos_ease) + pi.pos_from;

                    const EffectRectDrawer& d = world.get<EffectRectDrawer>();

                    Vector2 base_vector = Vector2(scale.x, 0.0f);
                    Vector2 last_pos = base_vector + center;
                    Transform2D rotation_matrix(angle, Vector2(0.0f, 0.0f));
                    for (int i = 0; i < point_count; i++) {
                        base_vector = rotation_matrix.basis_xform(base_vector);
                        Vector2 next_pos = base_vector + center;
                        float trans_rot = (i + 0.5f) * angle + Math_PI / 2.0f;
                        Vector2 trans_scale((next_pos - last_pos).length() + w, w);
                        Transform2D transform(trans_rot, trans_scale, 0.0f, (next_pos + last_pos) / 2.0f);

                        const int instance_id = pi.id + i;
                        d.ptr->set_instance_color(color, instance_id);
                        d.ptr->set_instance_transform(transform, instance_id);

                        const PointLightDrawer& pld = world.get<PointLightDrawer>();
                        Transform2D lt = transform;
                        lt[0] *= 1.2f; lt[1] *= 3.5f;
                        Color lc = color; lc.a *= 0.3f;
                        pld.ptr->set_instance_color(lc, instance_id);
                        pld.ptr->set_instance_transform(lt, instance_id);

                        last_pos = next_pos;
                    }
                });

            // 粒子绘制提交
            world.system<>("EffectDrawSubmit")
                .kind(flecs::OnUpdate)
                .multi_threaded(false)
                .each([&world]() {
                    world.get<EffectCircleDrawer>().ptr->update();
                    world.get<EffectRectDrawer>().ptr->update();
                    world.get<EffectTriangleDrawer>().ptr->update();
                    world.get<PointLightDrawer>().ptr->update();
                });
        } // 粒子


        // 拖尾系统
        // 每帧更新最新点
        world.system<const Position, Trail>("TrailUpdateLatest")
            .kind(flecs::OnUpdate)
            .each([](flecs::entity e, const Position& p, Trail& t) {
                if (t.now_len > 0) {
                    int latest = (t.next_point - 1 + t.max_len) % t.max_len;
                    t.points[latest] = p.value;
                }
            });

        // 每 3 tick 新增一个轨迹点
        world.system<const Position, Trail>("TrailTickUpdate")
            .kind(flecs::OnUpdate)
            .interval(static_cast<real_t>(ConstsC::get_tick_time() * 3.0f))
            .each([](flecs::entity e, const Position& p, Trail& t) {
                t.points[t.next_point] = p.value;
                t.next_point = (t.next_point + 1) % t.max_len;
                t.now_len = Math::min(t.now_len + 1, t.max_len);
            });

        // 拖尾绘制
        auto draw_trail = [](RID rid, const Trail& t) {
            if (t.now_len < 2 || t.max_width <= Trail::EPS) return;

            int n = t.now_len;
            std::vector<Vector2> center_points(n);
            for (int i = 0; i < n; ++i) {
                int idx = (t.next_point - 1 - i + t.max_len) % t.max_len;
                center_points[i] = t.points[idx];
            }

            std::vector<Vector2> clean;
            clean.reserve(n);
            for (const auto& p : center_points) {
                if (clean.empty() || clean.back().distance_squared_to(p) > Trail::EPS) {
                    clean.push_back(p);
                }
            }
            if (clean.size() < 2) return;

            std::vector<Vector2> filtered;
            filtered.reserve(clean.size());
            for (const auto& p : clean) {
                if (p.distance_squared_to(Vector2()) > Trail::EPS) {
                    filtered.push_back(p);
                }
            }
            if (filtered.size() < 2) return;

            n = static_cast<int>(filtered.size());
            float denom = Math::max(n - 1, 1);

            PackedVector2Array verts;
            PackedColorArray cols;
            verts.resize(n * 2);
            cols.resize(n * 2);
            Vector2* verts_w = verts.ptrw();
            Color* cols_w = cols.ptrw();

            for (int i = 0; i < n; ++i) {
                const Vector2& p = filtered[i];

                Vector2 prev = (i > 0) ? filtered[i - 1] : (p + (p - filtered[i + 1]));
                Vector2 next = (i < n - 1) ? filtered[i + 1] : (p + (p - filtered[i - 1]));
                Vector2 tangent = (next - prev).normalized();

                if (tangent.length_squared() <= Trail::EPS) {
                    if (i < n - 1) tangent = (filtered[i + 1] - p).normalized();
                    else if (i > 0) tangent = (p - filtered[i - 1]).normalized();
                    else tangent = Vector2(1.0f, 0.0f);
                }

                Vector2 normal(-tangent.y, tangent.x);
                float width = t.max_width * (1.0f - static_cast<float>(i) / denom);

                Color color = t.front_color;
                if (t.gradient()) {
                    color = t.front_color.lerp(t.back_color, static_cast<float>(i) / denom);
                }

                verts_w[i * 2] = p + normal * width;
                verts_w[i * 2 + 1] = p - normal * width;
                cols_w[i * 2] = color;
                cols_w[i * 2 + 1] = color;
            }

            int tri_count = n - 1;
            PackedInt32Array indices;
            indices.resize(tri_count * 6);
            int* idx_w = indices.ptrw();
            int idx = 0;
            for (int i = 0; i < n - 1; ++i) {
                int a = i * 2, b = i * 2 + 1, c = (i + 1) * 2, d = (i + 1) * 2 + 1;
                idx_w[idx++] = a; idx_w[idx++] = b; idx_w[idx++] = c;
                idx_w[idx++] = b; idx_w[idx++] = c; idx_w[idx++] = d;
            }

            RenderingServer::get_singleton()->canvas_item_add_triangle_array(rid, indices, verts, cols);
        };

        world.system<const Trail>("TrailDraw")
            .kind(flecs::OnUpdate)
            .multi_threaded(false)
            .run([&world, &draw_trail](flecs::iter& it) {
                const TrailDrawer& td = world.get<TrailDrawer>();
                if (!td.canvas_rid.is_valid()) return;

                RenderingServer::get_singleton()->canvas_item_clear(td.canvas_rid);

                while (it.next()) {
                    auto trails = it.field<const Trail>(0);
                    for (int i = 0; i < it.count(); ++i) {
                        draw_trail(td.canvas_rid, trails[i]);
                    }
                }
            });
        
        world.system<const UnitTypeComp>("UnitDraw")
            .kind(flecs::OnUpdate)
            .multi_threaded(false)
            .run([&world](flecs::iter& it) {
                const UnitDrawer& ud = world.get<UnitDrawer>();
                if (!ud.canvas_rid.is_valid()) return;
        
                RenderingServer::get_singleton()->canvas_item_clear(ud.canvas_rid);
        
                while (it.next()) {
                    auto units = it.field<const UnitTypeComp>(0);
                    auto positions = it.field<const Position>(1);
                    auto rotations = it.field<const Rotation>(2);
        
                    for (int i = 0; i < it.count(); ++i) {
                        const UnitTypeComp& utc = units[i];
                        const Position& p = positions[i];
                        const Rotation& r = rotations[i];
                        utc.unit_type->call_deferred("_draw", r.value, p.value, 2.0,
                                                     Color(1, 1, 1, 1), ud.canvas_rid);
                    }
                }
            });

    }
} // namespace ecs
