#include "tools/Call/Call.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static double s_time_point = 0.0;

Object* Call::vars = nullptr;

void Call::_bind_methods() {
    ClassDB::bind_static_method("Call", D_METHOD("x_to_tile", "x"), &Call::x_to_tile);
    ClassDB::bind_static_method("Call", D_METHOD("y_to_tile", "y"), &Call::y_to_tile);
    ClassDB::bind_static_method("Call", D_METHOD("pos_to_tile", "pos"), &Call::pos_to_tile);
    ClassDB::bind_static_method("Call", D_METHOD("tile_to_pos", "tile_pos"), &Call::tile_to_pos);
    ClassDB::bind_static_method("Call", D_METHOD("tile_to_region", "tile_pos"), &Call::tile_to_region);
    ClassDB::bind_static_method("Call", D_METHOD("region_to_tile", "region"), &Call::region_to_tile);
    ClassDB::bind_static_method("Call", D_METHOD("pos_to_region", "pos"), &Call::pos_to_region);
    ClassDB::bind_static_method("Call", D_METHOD("region_to_pos", "region"), &Call::region_to_pos);
    ClassDB::bind_static_method("Call", D_METHOD("tile_rect_to_global", "rect"), &Call::tile_rect_to_global);
    ClassDB::bind_static_method("Call", D_METHOD("global_rect_to_tile", "rect"), &Call::global_rect_to_tile);
    ClassDB::bind_static_method("Call", D_METHOD("pos_to_big_region", "pos"), &Call::pos_to_big_region);
    ClassDB::bind_static_method("Call", D_METHOD("tile_to_big_region", "tile_pos"), &Call::tile_to_big_region);
    ClassDB::bind_static_method("Call", D_METHOD("big_region_to_region", "region"), &Call::big_region_to_region);
    ClassDB::bind_static_method("Call", D_METHOD("region_center", "region"), &Call::region_center);
    ClassDB::bind_static_method("Call", D_METHOD("big_region_center", "region"), &Call::big_region_center);
    ClassDB::bind_static_method("Call", D_METHOD("get_tile_rect", "tile"), &Call::get_tile_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_region_rect", "region"), &Call::get_region_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_big_region_rect", "region"), &Call::get_big_region_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_region_tile_rect", "region"), &Call::get_region_tile_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_big_region_tile_rect", "region"), &Call::get_big_region_tile_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_circle_tile_rect", "center", "radius"), &Call::get_circle_tile_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_circle_aabb", "center", "radius"), &Call::get_circle_aabb);
    ClassDB::bind_static_method("Call", D_METHOD("get_line_tile_rect", "from", "to"), &Call::get_line_tile_rect);
    ClassDB::bind_static_method("Call", D_METHOD("region_tiles", "r", "method"), &Call::region_tiles);
    ClassDB::bind_static_method("Call", D_METHOD("big_region_regions", "r", "method"), &Call::big_region_regions);
    ClassDB::bind_static_method("Call", D_METHOD("circle_tiles", "pos", "radius", "method"), &Call::circle_tiles);
    ClassDB::bind_static_method("Call", D_METHOD("circle_collision", "pos", "radius", "method"), &Call::circle_collision);
    ClassDB::bind_static_method("Call", D_METHOD("circle_regions", "pos", "radius", "method"), &Call::circle_regions);
    ClassDB::bind_static_method("Call", D_METHOD("circle_big_regions", "pos", "radius", "method"), &Call::circle_big_regions);
    ClassDB::bind_static_method("Call", D_METHOD("rect_tiles", "pos", "width", "height", "method"), &Call::rect_tiles);
    ClassDB::bind_static_method("Call", D_METHOD("rect_collision", "pos", "width", "height", "method"), &Call::rect_collision);
    ClassDB::bind_static_method("Call", D_METHOD("rect_regions", "pos", "width", "height", "method"), &Call::rect_regions);
    ClassDB::bind_static_method("Call", D_METHOD("rect_big_regions", "pos", "width", "height", "method"), &Call::rect_big_regions);
    ClassDB::bind_static_method("Call", D_METHOD("rect2i_outer", "l", "t", "size_x", "size_y", "width", "method", "ignore_vertex"), &Call::rect2i_outer, DEFVAL(false));
    ClassDB::bind_static_method("Call", D_METHOD("ring_tiles", "pos", "radius_from", "radius", "method"), &Call::ring_tiles);
    ClassDB::bind_static_method("Call", D_METHOD("ring_collision", "pos", "radius_from", "radius", "method"), &Call::ring_collision);
    ClassDB::bind_static_method("Call", D_METHOD("ring_blocks", "pos", "radius_from", "radius", "method"), &Call::ring_blocks);
    ClassDB::bind_static_method("Call", D_METHOD("ring_regions", "pos", "radius_from", "radius", "method"), &Call::ring_regions);
    ClassDB::bind_static_method("Call", D_METHOD("ring_big_regions", "pos", "radius_from", "radius", "method"), &Call::ring_big_regions);
    ClassDB::bind_static_method("Call", D_METHOD("angle_range_transport", "angle"), &Call::angle_range_transport);
    ClassDB::bind_static_method("Call", D_METHOD("line_chunks", "from", "to", "width", "chunk_size", "method"), &Call::line_chunks);
    ClassDB::bind_static_method("Call", D_METHOD("line_chunks_without_width", "from", "to", "chunk_size", "method"), &Call::line_chunks_without_width);
    ClassDB::bind_static_method("Call", D_METHOD("iterate_points_in_r1_not_r2", "r1", "r2", "callback"), &Call::iterate_points_in_r1_not_r2);
    ClassDB::bind_static_method("Call", D_METHOD("rect_circle_intersects", "rect", "radius", "pos"), &Call::rect_circle_intersects);
    ClassDB::bind_static_method("Call", D_METHOD("rect_ring_intersects", "rect", "radius_from", "radius", "pos"), &Call::rect_ring_intersects);
    ClassDB::bind_static_method("Call", D_METHOD("l_c_handle", "from", "to", "width", "radius", "pos"), &Call::l_c_handle);
    ClassDB::bind_static_method("Call", D_METHOD("point_s_handle", "point", "radius", "pos", "dir", "half_angle"), &Call::point_s_handle);
    ClassDB::bind_static_method("Call", D_METHOD("screen_to_global", "screen_pos"), &Call::screen_to_global);
    ClassDB::bind_static_method("Call", D_METHOD("global_to_screen", "global_pos"), &Call::global_to_screen);
    ClassDB::bind_static_method("Call", D_METHOD("get_transform"), &Call::get_transform);
    ClassDB::bind_static_method("Call", D_METHOD("screen_left_up"), &Call::screen_left_up);
    ClassDB::bind_static_method("Call", D_METHOD("screen_global_size"), &Call::screen_global_size);
    ClassDB::bind_static_method("Call", D_METHOD("screen_first_tile"), &Call::screen_first_tile);
    ClassDB::bind_static_method("Call", D_METHOD("screen_first_region"), &Call::screen_first_region);
    ClassDB::bind_static_method("Call", D_METHOD("screen_to_global_rect"), &Call::screen_to_global_rect);
    ClassDB::bind_static_method("Call", D_METHOD("screen_tile_rect"), &Call::screen_tile_rect);
    ClassDB::bind_static_method("Call", D_METHOD("screen_region_rect"), &Call::screen_region_rect);
    ClassDB::bind_static_method("Call", D_METHOD("get_lightness", "c"), &Call::get_lightness);
    ClassDB::bind_static_method("Call", D_METHOD("ff"), &Call::ff);
    ClassDB::bind_static_method("Call", D_METHOD("time_start"), &Call::time_start);
    ClassDB::bind_static_method("Call", D_METHOD("time_print", "extra", "restart"), &Call::time_print, DEFVAL(String()), DEFVAL(true));
}

Object *Call::_vars() {
    if (vars) return vars;

    SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
    if (!tree || !tree->get_root()) return nullptr;

    vars = Object::cast_to<Object>(tree->get_root()->get_node_or_null(NodePath("/root/Vars")));
    return vars;
}

int Call::x_to_tile(double x) { return int(Math::floor(x / ConstsC::get_tile_size().x)); }
int Call::y_to_tile(double y) { return int(Math::floor(y / ConstsC::get_tile_size().y)); }
Vector2i Call::pos_to_tile(Vector2 pos) { return (pos / ConstsC::get_tile_size()).floor(); }
Vector2 Call::tile_to_pos(Vector2i tile_pos) { return (Vector2(tile_pos) + Vector2(0.5, 0.5)) * ConstsC::get_tile_size(); }
Vector2i Call::tile_to_region(Vector2i tile_pos) { return (Vector2(tile_pos) / Vector2(ConstsC::get_region_size())).floor(); }
Vector2i Call::region_to_tile(Vector2i region) { return region * ConstsC::get_region_size(); }
Vector2i Call::pos_to_region(Vector2 pos) { return (pos / ConstsC::get_real_region_size()).floor(); }
Vector2 Call::region_to_pos(Vector2i region) { return Vector2(region) * ConstsC::get_real_region_size(); }
Rect2 Call::tile_rect_to_global(Rect2i rect) { Vector2 ts = ConstsC::get_tile_size(); return Rect2(rect.position.x * ts.x, rect.position.y * ts.y, rect.size.x * ts.x, rect.size.y * ts.x); }
Rect2i Call::global_rect_to_tile(Rect2 rect) { Vector2i lt = pos_to_tile(rect.position); Vector2i rd = pos_to_tile(rect.position + rect.size); return Rect2i(lt, rd - lt + Vector2i(1,1)); }
Vector2i Call::pos_to_big_region(Vector2 pos) { return (pos / ConstsC::get_big_region_real_size()).floor(); }
Vector2i Call::tile_to_big_region(Vector2i tile_pos) { return (Vector2(tile_pos) / Vector2(ConstsC::get_big_region_tile_size())).floor(); }
Vector2i Call::big_region_to_region(Vector2i region) { return region * ConstsC::get_big_region_size(); }
Vector2 Call::region_center(Vector2i region) { return Vector2(region) * ConstsC::get_real_region_size() + ConstsC::get_hrrs(); }
Vector2 Call::big_region_center(Vector2i region) { return Vector2(region) * ConstsC::get_big_region_real_size() + ConstsC::get_half_big_region_real_size(); }
Rect2 Call::get_tile_rect(Vector2i tile) { Vector2 ts = ConstsC::get_tile_size(); return Rect2(Vector2(tile) * ts, ts); }
Rect2 Call::get_region_rect(Vector2i region) { Vector2 rs = ConstsC::get_real_region_size(); return Rect2(Vector2(region) * rs, rs); }
Rect2 Call::get_big_region_rect(Vector2i region) { Vector2 brs = ConstsC::get_big_region_real_size(); return Rect2(Vector2(region) * brs, brs); }
Rect2i Call::get_region_tile_rect(Vector2i region) { Vector2i rs = ConstsC::get_region_size(); return Rect2i(region * rs, rs); }
Rect2i Call::get_big_region_tile_rect(Vector2i region) { Vector2i brs = ConstsC::get_big_region_tile_size(); return Rect2i(region * brs, brs); }
Rect2i Call::get_circle_tile_rect(Vector2 center, double radius) { Vector2 ts = ConstsC::get_tile_size(); return Rect2i(pos_to_tile(Vector2(center.x - radius, center.y - radius)), Vector2i(Vector2(radius * 2 + ts.x, radius * 2 + ts.y) / ts)); }
Rect2 Call::get_circle_aabb(Vector2 center, double radius) { return Rect2(center.x - radius, center.y - radius, radius * 2, radius * 2); }
Rect2i Call::get_line_tile_rect(Vector2 from, Vector2 to) {
    float l = MIN(from.x, to.x), r = MAX(from.x, to.x), t = MIN(from.y, to.y), b = MAX(from.y, to.y);
    Vector2 ts = ConstsC::get_tile_size();
    return Rect2i(pos_to_tile(Vector2(l, t)), Vector2i(int(Math::floor((r - l) / ts.x)) + 1, int(Math::floor((b - t) / ts.y)) + 1));
}

bool Call::region_tiles(Vector2i r, Callable method) { Vector2i first = region_to_tile(r); Vector2i rs = ConstsC::get_region_size(); for (int y = first.y; y < first.y + rs.y; y++) for (int x = first.x; x < first.x + rs.x; x++) if (bool(method.call(Vector2i(x, y)))) return true; return false; }
bool Call::big_region_regions(Vector2i r, Callable method) { Vector2i first = big_region_to_region(r); Vector2i bs = ConstsC::get_big_region_size(); for (int y = first.y; y < first.y + bs.y; y++) for (int x = first.x; x < first.x + bs.x; x++) if (bool(method.call(Vector2i(x, y)))) return true; return false; }
bool Call::circle_tiles(Vector2 pos, double radius, Callable method) { Vector2i f = pos_to_tile(pos - Vector2(radius, radius)); Vector2 hs = ConstsC::get_half_tile_size(); for (int y = 0; y < Math::ceil(radius / hs.y) + 1; y++) for (int x = 0; x < Math::ceil(radius / hs.x) + 1; x++) { Vector2i tile = f + Vector2i(x,y); if (rect_circle_intersects(get_tile_rect(tile), radius, pos) && bool(method.call(tile))) return true; } return false; }
bool Call::circle_collision(Vector2 pos, double radius, Callable method) { Object *v = _vars(); if (!v) return false; Vector2i f = pos_to_tile(pos - Vector2(radius,radius)); int size = Math::ceil(radius / ConstsC::get_half_tile_size().x) + 1; Array collided; for (int y = f.y; y < f.y + size; y++) for (int x = f.x; x < f.x + size; x++) { Vector2i tile(x,y); Object *b = Object::cast_to<Object>(Dictionary(v->get("block_instances")).get(tile, Variant())); if (b && !collided.has(b)) { collided.append(b); if (rect_circle_intersects(b->call("get_collision_rect"), radius, pos) && bool(method.call(b))) return true; } } return false; }
bool Call::circle_regions(Vector2 pos, double radius, Callable method) { Vector2i f = pos_to_region(pos - Vector2(radius,radius)); Vector2 h = ConstsC::get_hrrs(); for (int y=0;y<Math::ceil(radius/h.y)+1;y++) for (int x=0;x<Math::ceil(radius/h.x)+1;x++){ Vector2i r=f+Vector2i(x,y); if(rect_circle_intersects(get_region_rect(r),radius,pos)&&bool(method.call(r))) return true;} return false; }
bool Call::circle_big_regions(Vector2 pos, double radius, Callable method) { Vector2i f = pos_to_big_region(pos - Vector2(radius,radius)); Vector2 h = ConstsC::get_half_big_region_real_size(); for (int y=0;y<Math::ceil(radius/h.y)+1;y++) for (int x=0;x<Math::ceil(radius/h.x)+1;x++){ Vector2i r=f+Vector2i(x,y); if(rect_circle_intersects(get_big_region_rect(r),radius,pos)&&bool(method.call(r))) return true;} return false; }
void Call::rect_tiles(Vector2 pos, double width, double height, Callable method) { Vector2 tl = pos - Vector2(width*0.5,height*0.5); Vector2i st = pos_to_tile(tl); Vector2 ts = ConstsC::get_tile_size(); int xc=Math::ceil(width/ts.x)+1, yc=Math::ceil(height/ts.y)+1; for(int y=0;y<yc;y++) for(int x=0;x<xc;x++) method.call(st+Vector2i(x,y)); }
void Call::rect_collision(Vector2 pos, double width, double height, Callable method) { Object *v = _vars(); if(!v) return; Rect2 rect(pos.x-width*0.5,pos.y-height*0.5,width,height); Vector2i st = pos_to_tile(rect.position); Vector2 ts = ConstsC::get_tile_size(); int xc=Math::ceil(width/ts.x)+1, yc=Math::ceil(height/ts.y)+1; Array collided; Dictionary inst=v->get("block_instances"); for(int y=0;y<yc;y++) for(int x=0;x<xc;x++){ Object *b=Object::cast_to<Object>(inst.get(st+Vector2i(x,y),Variant())); if(b && !collided.has(b) && Rect2(b->call("get_collision_rect")).intersects(rect)){ collided.append(b); method.call(b);} } }
void Call::rect_regions(Vector2 pos, double width, double height, Callable method) { Vector2 tl=pos-Vector2(width*0.5,height*0.5); Vector2i st=pos_to_region(tl); Vector2 h=ConstsC::get_hrrs(); int xc=Math::ceil(width/h.x)+1,yc=Math::ceil(height/h.y)+1; for(int y=0;y<yc;y++) for(int x=0;x<xc;x++) method.call(st+Vector2i(x,y)); }
void Call::rect_big_regions(Vector2 pos, double width, double height, Callable method) { Vector2 tl=pos-Vector2(width*0.5,height*0.5); Vector2i st=pos_to_big_region(tl); Vector2 h=ConstsC::get_half_big_region_real_size(); int xc=Math::ceil(width/h.x)+1,yc=Math::ceil(height/h.y)+1; for(int y=0;y<yc;y++) for(int x=0;x<xc;x++) method.call(st+Vector2i(x,y)); }
void Call::rect2i_outer(int l,int t,int size_x,int size_y,int width,Callable method,bool ignore_vertex){ int ign=int(ignore_vertex); int r=l+size_x-1,b=t+size_y-1,left=l-width,right=r+width,top=t-width,bottom=b+width; for(int w=0;w<width;w++){ int cl=left+w,cr=right-w,ct=top+w,cb=bottom-w; for(int x=cl+ign;x<=cr-ign;x++){ method.call(Vector2i(x,ct)); method.call(Vector2i(x,cb)); } for(int y=ct+1;y<cb;y++){ method.call(Vector2i(cl,y)); method.call(Vector2i(cr,y)); } } }
bool Call::ring_tiles(Vector2 pos,double rf,double r,Callable method){ Vector2i f=pos_to_tile(pos-Vector2(r,r)); Vector2 hs=ConstsC::get_half_tile_size(); for(int y=0;y<Math::ceil(r/hs.y)+1;y++) for(int x=0;x<Math::ceil(r/hs.x)+1;x++){ Vector2i t=f+Vector2i(x,y); if(rect_ring_intersects(get_tile_rect(t),rf,r,pos)&&bool(method.call(t))) return true; } return false; }
bool Call::ring_collision(Vector2 pos,double rf,double r,Callable method){ Object *v=_vars(); if(!v) return false; Vector2i f=pos_to_tile(pos-Vector2(r,r)); Vector2 hs=ConstsC::get_half_tile_size(); Array collided; Dictionary inst=v->get("block_instances"); for(int y=0;y<Math::ceil(r/hs.y)+1;y++) for(int x=0;x<Math::ceil(r/hs.x)+1;x++){ Vector2i t=f+Vector2i(x,y); Object *b=Object::cast_to<Object>(inst.get(t,Variant())); if(b && !collided.has(b)){ collided.append(b); if(rect_ring_intersects(b->call("get_collision_rect"),rf,r,pos)&&bool(method.call(b))) return true; }} return false; }
bool Call::ring_blocks(Vector2 pos,double rf,double r,Callable method){ Object *v=_vars(); if(!v) return false; Vector2i f=pos_to_tile(pos-Vector2(r,r)); Vector2 hs=ConstsC::get_half_tile_size(); Dictionary inst=v->get("block_instances"); for(int y=0;y<Math::ceil(r/hs.y)+1;y++) for(int x=0;x<Math::ceil(r/hs.x)+1;x++){ Vector2i t=f+Vector2i(x,y); Object *b=Object::cast_to<Object>(inst.get(t,Variant())); if(b && Vector2i(b->get("pos"))==t){ if(rect_ring_intersects(b->call("get_collision_rect"),rf,r,pos)&&bool(method.call(b))) return true; }} return false; }
bool Call::ring_regions(Vector2 pos,double rf,double r,Callable method){ Vector2i f=pos_to_region(pos-Vector2(r,r)); Vector2 h=ConstsC::get_hrrs(); for(int y=0;y<Math::ceil(r/h.y)+1;y++) for(int x=0;x<Math::ceil(r/h.x)+1;x++){ Vector2i rg=f+Vector2i(x,y); if(rect_ring_intersects(get_region_rect(rg),rf,r,pos)&&bool(method.call(rg))) return true; } return false; }
bool Call::ring_big_regions(Vector2 pos,double rf,double r,Callable method){ Vector2i f=pos_to_big_region(pos-Vector2(r,r)); Vector2 h=ConstsC::get_half_big_region_real_size(); for(int y=0;y<Math::ceil(r/h.y)+1;y++) for(int x=0;x<Math::ceil(r/h.x)+1;x++){ Vector2i rg=f+Vector2i(x,y); if(rect_ring_intersects(get_big_region_rect(rg),rf,r,pos)&&bool(method.call(rg))) return true; } return false; }
double Call::angle_range_transport(double angle){ return Math::fmod(angle + Math_TAU, Math_TAU); }
bool Call::line_chunks(Vector2 from,Vector2 to,double width,Vector2 chunk_size,Callable method){
    PackedVector2Array handled;
    auto call_unique = [&](Vector2i tile) -> bool {
        if (!handled.has(tile)) {
            handled.push_back(tile);
            return bool(method.call(tile));
        }
        return false;
    };
    double angle=(to-from).angle();
    Vector2 o1=Vector2::from_angle(angle+Math_PI/2.0)*width/2.0;
    Vector2 o2=Vector2::from_angle(angle-Math_PI/2.0)*width/2.0;

    Vector2 from1 = from + o1;
    Vector2 to1 = to + o1;
    Vector2i ft1(int(Math::floor(from1.x/chunk_size.x)), int(Math::floor(from1.y/chunk_size.y)));
    Vector2i tt1(int(Math::floor(to1.x/chunk_size.x)), int(Math::floor(to1.y/chunk_size.y)));
    int x0=ft1.x,y0=ft1.y,x1=tt1.x,y1=tt1.y;
    int dx=Math::abs(x1-x0),dy=Math::abs(y1-y0),sx=x0>x1?-1:1,sy=y0>y1?-1:1,err=dx-dy;
    while(true){ if(call_unique(Vector2i(x0,y0))) return true; if(x0==x1&&y0==y1) break; int e2=2*err; if(e2>-dy){err-=dy;x0+=sx;} if(e2<dx){err+=dx;y0+=sy;} }

    Vector2 from2 = from + o2;
    Vector2 to2 = to + o2;
    Vector2i ft2(int(Math::floor(from2.x/chunk_size.x)), int(Math::floor(from2.y/chunk_size.y)));
    Vector2i tt2(int(Math::floor(to2.x/chunk_size.x)), int(Math::floor(to2.y/chunk_size.y)));
    x0=ft2.x; y0=ft2.y; x1=tt2.x; y1=tt2.y;
    dx=Math::abs(x1-x0); dy=Math::abs(y1-y0); sx=x0>x1?-1:1; sy=y0>y1?-1:1; err=dx-dy;
    while(true){ if(call_unique(Vector2i(x0,y0))) return true; if(x0==x1&&y0==y1) break; int e2=2*err; if(e2>-dy){err-=dy;x0+=sx;} if(e2<dx){err+=dx;y0+=sy;} }
    return false;
}
bool Call::line_chunks_without_width(Vector2 from,Vector2 to,Vector2 chunk_size,Callable method){ Vector2i ft(int(Math::floor(from.x/chunk_size.x)), int(Math::floor(from.y/chunk_size.y))); Vector2i tt(int(Math::floor(to.x/chunk_size.x)), int(Math::floor(to.y/chunk_size.y))); int x0=ft.x,y0=ft.y,x1=tt.x,y1=tt.y; int dx=Math::abs(x1-x0),dy=Math::abs(y1-y0),sx=x0>x1?-1:1,sy=y0>y1?-1:1,err=dx-dy; while(true){ if(bool(method.call(Vector2i(x0,y0)))) return true; if(x0==x1 && y0==y1) break; int e2=2*err; if(e2>-dy){ err-=dy; x0+=sx; } if(e2<dx){ err+=dx; y0+=sy; }} return false; }
void Call::iterate_points_in_r1_not_r2(Rect2i r1,Rect2i r2,Callable cb){ int x1=r1.position.x,y1=r1.position.y,x1e=x1+r1.size.x-1,y1e=y1+r1.size.y-1; if(r1.size.x<=0||r1.size.y<=0) return; int x2=r2.position.x,y2=r2.position.y,x2e=x2+r2.size.x-1,y2e=y2+r2.size.y-1; int ix1=MAX(x1,x2),iy1=MAX(y1,y2),ix2=MIN(x1e,x2e),iy2=MIN(y1e,y2e); if(ix1>ix2||iy1>iy2){ for(int y=y1;y<=y1e;y++) for(int x=x1;x<=x1e;x++) cb.call(Vector2i(x,y)); return;} if(iy1>y1) for(int y=y1;y<iy1;y++) for(int x=x1;x<=x1e;x++) cb.call(Vector2i(x,y)); if(iy2<y1e) for(int y=iy2+1;y<=y1e;y++) for(int x=x1;x<=x1e;x++) cb.call(Vector2i(x,y)); if(ix1>x1) for(int y=iy1;y<=iy2;y++) for(int x=x1;x<ix1;x++) cb.call(Vector2i(x,y)); if(ix2<x1e) for(int y=iy1;y<=iy2;y++) for(int x=ix2+1;x<=x1e;x++) cb.call(Vector2i(x,y)); }
bool Call::rect_circle_intersects(Rect2 rect,double radius,Vector2 pos){ Vector2 dif=pos-rect.get_center(); Vector2 n(Math::clamp(dif.x,-rect.size.x/2,rect.size.x/2),Math::clamp(dif.y,-rect.size.y/2,rect.size.y/2)); return (dif-n).length_squared() <= radius*radius; }
bool Call::rect_ring_intersects(Rect2 rect,double rf,double r,Vector2 pos){ Vector2 dif=pos-rect.get_center(); Vector2 n(Math::clamp(dif.x,-rect.size.x/2,rect.size.x/2),Math::clamp(dif.y,-rect.size.y/2,rect.size.y/2)); Vector2 f(Math::clamp(-dif.x,-rect.size.x/2,rect.size.x/2),Math::clamp(-dif.y,-rect.size.y/2,rect.size.y/2)); return (dif-n).length_squared() <= r*r && (dif-f).length_squared() >= rf*rf; }
bool Call::l_c_handle(Vector2 from,Vector2 to,double width,double radius,Vector2 pos){ double dst=(from-pos).length(); double angle=(to-from).angle()-(pos-from).angle(); if(Math::cos(angle)*dst <= (from-to).length()) return Math::sin(angle)*dst <= width+radius; return dst<=radius || (to-pos).length()<=radius; }
bool Call::point_s_handle(Vector2 point,double radius,Vector2 pos,double dir,double half_angle){ return (point-pos).length_squared()<=radius*radius && Math::abs((point-pos).angle()-dir)<=half_angle; }
Vector2 Call::screen_to_global(Vector2 screen_pos){ Object *v=_vars(); if(!v) return Vector2(); Object *world=Object::cast_to<Object>(v->get("world")); Object *camera=world?Object::cast_to<Object>(world->get("camera")):nullptr; Vector2 zoom = camera ? Vector2(camera->get("zoom")) : Vector2(1,1); return screen_pos/zoom + screen_left_up(); }
Vector2 Call::global_to_screen(Vector2 global_pos){ return get_transform().xform(global_pos); }
Transform2D Call::get_transform(){ Object *v=_vars(); if(!v) return Transform2D(); Object *vp=Object::cast_to<Object>(v->call("get_viewport")); return vp ? Transform2D(vp->get("canvas_transform")) : Transform2D(); }
Vector2 Call::screen_left_up(){ Object *v=_vars(); if(!v) return Vector2(); Object *world=Object::cast_to<Object>(v->get("world")); Object *camera=world?Object::cast_to<Object>(world->get("camera")):nullptr; Vector2 gp = camera ? Vector2(camera->get("global_position")) : Vector2(); return gp - screen_global_size()/2.0; }
Vector2 Call::screen_global_size(){ Object *v=_vars(); if(!v) return Vector2(); Object *world=Object::cast_to<Object>(v->get("world")); Object *camera=world?Object::cast_to<Object>(world->get("camera")):nullptr; Vector2 zoom = camera ? Vector2(camera->get("zoom")) : Vector2(1,1); Vector2 screen = Vector2(v->get("screen_size")); return screen/zoom; }
Vector2i Call::screen_first_tile(){ return pos_to_tile(screen_left_up()); }
Vector2i Call::screen_first_region(){ return pos_to_region(screen_left_up()); }
Rect2 Call::screen_to_global_rect(){ return Rect2(screen_left_up(), screen_global_size()); }
Rect2i Call::screen_tile_rect(){ return Rect2i(screen_first_tile(), Vector2i((screen_global_size() / ConstsC::get_tile_size()).ceil()) + Vector2i(1,1)); }
Rect2i Call::screen_region_rect(){ return Rect2i(screen_first_region(), Vector2i((screen_global_size() / ConstsC::get_real_region_size()).ceil()) + Vector2i(1,1)); }
double Call::get_lightness(Color c){ return c.r*0.299 + c.g*0.114 + c.b*0.587; }
void Call::ff() {}
void Call::time_start(){ UtilityFunctions::print("-----------------------------"); s_time_point = Time::get_singleton()->get_ticks_usec(); }
void Call::time_print(String extra, bool restart){ double now = Time::get_singleton()->get_ticks_usec(); UtilityFunctions::print(now - s_time_point, "us", "_", extra); if (restart) s_time_point = now; }
