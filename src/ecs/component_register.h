#pragma once

#include <atomic>
#include <cstdint>
#include <vector>
#include <cassert>

namespace comp {
    using ComponentID = std::uint16_t;
    constexpr ComponentID kMaxComponentID = 65535;
    
    // 构造函数与析构函数指针类型
    using ComponentConstructor = void(*)(void*);
    using ComponentDestructor = void(*)(void*);
    
    struct ComponentMeta {
        ComponentID id;
        ComponentConstructor constructor;  // 用于 placement new 的构造函数
        ComponentDestructor destructor;    // 用于显式调用析构函数
    };

    class ComponentRegister {
    public:
        template <typename T>
        static ComponentID register_component() {
            ComponentMeta meta;
            meta.id = get_next_id();
            // 利用无捕获 lambda 生成函数指针
            meta.constructor = [](void* ptr) { new(ptr) T(); };
            meta.destructor = [](void* ptr) { static_cast<T*>(ptr)->~T(); };
            
            components.push_back(meta);
            return meta.id;
        }

    private:
        static inline std::vector<ComponentMeta> components;

        static ComponentID get_next_id() {
            static ComponentID next_id{0};
            assert(next_id < kMaxComponentID);
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