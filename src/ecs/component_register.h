#pragma once

#include <atomic>
#include <cstdint>
#include <vector>
#include <cassert>

namespace comp {
    using ComponentID = std::uint8_t;
    constexpr size_t kMaxComponentCount = 256;
    constexpr ComponentID kMaxComponentID = 255;
    
    // 构造函数与析构函数指针类型
    using ComponentConstructor = void(*)(void*);
    using ComponentDestructor = void(*)(void*);
    
    struct ComponentMeta {
        ComponentID id;
        size_t size;
        size_t align;
        
        ComponentConstructor constructor;  // 用于 placement new 的构造函数
        ComponentDestructor destructor;    // 用于显式调用析构函数
    };

    class ComponentRegister {
    public:
        template <typename T>
        static ComponentID register_component() {
            ComponentMeta meta;
            meta.id = get_next_id();
            meta.size = sizeof(T);
            meta.align = alignof(T);
            
            // 利用无捕获 lambda 生成函数指针
            meta.constructor = [](void* ptr) { new(ptr) T(); };
            meta.destructor = [](void* ptr) { static_cast<T*>(ptr)->~T(); };
            
            components.push_back(meta);
            return meta.id;
        }
        
        static const ComponentMeta& get_meta(ComponentID id) {
            assert(id < components.size());
            return components[id];
        }

    private:
        static inline std::vector<ComponentMeta> components;

        static ComponentID get_next_id() {
            static size_t next_id{0};
            assert(next_id <= kMaxComponentID);
            return next_id++;
        }
    };
} // namespace comp

#define REGISTER_COMPONENT(T) \
namespace { \
    struct RegisterHelper { \
        RegisterHelper() { \
            comp::ComponentRegister::register_component<T>(); \
        } \
    } register_helper##__COUNTER__; \
}