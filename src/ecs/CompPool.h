#pragma once
#include <vector>
#include <cassert>
#include <cstdint>

using Entity = std::uint32_t;
using MaxId = 4294967295;

/**
 * @brief 特定类型组件的存储池
 * @tparam Comp 组件类型
 */
template<typename Comp>
class CompPool final {
private:
    std::vector<Comp> comps;          ///< 组件数据数组（紧凑存储）
    std::vector<Entity> c_to_e_vec;     ///< 组件索引 -> 实体ID
    std::vector<size_t> e_to_c_vec;     ///< 实体ID -> 组件索引 (MaxId为空)
    
    void reserveSize(size_t size) {
    	comps.reserve(size);
    	c_to_e_vec.reserve(size);
    	e_to_c_vec.resize(size, MaxId);
    }
public:
	CompPool(size_t size) {
		reserveSize(size);
	};
	
    /**
     * @brief 为实体添加组件（若已存在则覆盖）
     * @param entity 目标实体
     * @param comp 组件数据
     */
    void addComponent(Entity entity, const Comp& comp) {
        if (hasComponent(entity)) {
            // 已存在：直接更新组件数据
            comps[e_to_c_vec[entity]] = comp;
        } else {
            // 新组件：在末端追加
            size_t idx = comps.size();
            comps.push_back(comp);
            c_to_e_vec.push_back(entity);
            // 扩展实体映射表（如果实体ID超出当前范围）
            if (entity >= e_to_c_vec.size()) {
                reserveSize(entity + 1);
            }
            e_to_c_vec[entity] = idx;
        }
    }

    /**
     * @brief 删除实体的组件
     * @param entity 目标实体
     */
    void removeComponent(Entity entity) {
        assert(hasComponent(entity) && "Entity does not have this component");
        size_t idx = e_to_c_vec[entity];  // 内部索引
        size_t last = comps.size() - 1;

        if (idx != last) {
            // 将末尾组件移动到被删除的位置，保持数组紧凑
            comps[idx] = std::move(comps[last]);
            c_to_e_vec[idx] = c_to_e_vec[last];
            e_to_c_vec[c_to_e_vec[idx]] = idx; // 更新移入组件的实体映射
        }

        // 移除末尾
        comps.pop_back();
        c_to_e_vec.pop_back();
        e_to_c_vec[entity] = MaxId; // 清除实体映射
    }

    /**
     * @brief 获取实体的组件（可变）
     * @return 组件指针，不存在时返回 nullptr
     */
    Comp* getComponent(Entity entity) {
        if (!hasComponent(entity)) return nullptr;
        return &comps[e_to_c_vec[entity]];
    }

    /**
     * @brief 获取实体的组件（只读）
     * @return 组件指针，不存在时返回 nullptr
     */
    const Comp* getComponent(Entity entity) const {
        if (!hasComponent(entity)) return nullptr;
        return &comps[e_to_c_vec[entity]];
    }

    /**
     * @brief 检查实体是否拥有该组件
     */
    bool hasComponent(Entity entity) const {
        return entity < e_to_c_vec.size() && e_to_c_vec[entity] != MaxId;
    }

    /**
     * @brief 获取当前存储的组件数量
     */
    size_t size() const {
        return comps.size();
    }

    /**
     * @brief 获取所有组件的只读列表（用于遍历）
     */
    const std::vector<Comp>& components() const {
        return comps;
    }

    /**
     * @brief 获取所有对应实体的只读列表（与 components() 一一对应）
     */
    const std::vector<Entity>& entities() const {
        return c_to_e_vec;
    }

    /**
     * @brief 清空所有组件
     */
    void clear() {
        comps.clear();
        c_to_e_vec.clear();
        e_to_c_vec.clear();
    }
};