#include <cstdint>
#include <algorithm>

namespace ecs {

template<typename T, size_t MaxSize>
class StaticVector {
    static_assert(MaxSize > 0, "StaticVector must have a positive max size");
public:
    using value_type = T;
    using size_type  = size_t;

    // 默认构造为空
    StaticVector() = default;
    // 默认拷贝 / 移动均为平凡（因为仅有 array 和 int）
    ~StaticVector() = default;

    // ---------- 基本操作 ----------
    void push_back(const T& item) {
        if (size_ < MaxSize) {
            data_[size_++] = item;
        }
        // 若满则静默丢弃（可改为返回 bool 或 assert）
    }

    bool has(const T& item) const {
        for (size_t i = 0; i < size_; ++i) {
            if (data_[i] == item) return true;
        }
        return false;
    }

    void erase(const T& item) {
        for (size_t i = 0; i < size_; ++i) {
            if (data_[i] == item) {
                // 与最后一个有效元素交换，然后缩短长度
                data_[i] = data_[size_ - 1];
                --size_;
                return; // 假设元素唯一，否则可继续扫描
            }
        }
    }

    // ---------- 便利接口 ----------
    bool full()  const { return size_ == MaxSize; }
    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }
    const T& operator[](size_t i) const { return data_[i]; }
    T& operator[](size_t i) { return data_[i]; }

    // 一次性添加，若已存在或满则失败
    bool try_add(const T& item) {
        if (has(item)) return false;
        if (full())    return false;
        push_back(item);
        return true;
    }

    void clear() { size_ = 0; }

    // 支持范围 for 循环
    const T* begin() const { return &data_[0]; }
    const T* end()   const { return &data_[size_]; }
    T* begin() { return &data_[0]; }
    T* end()   { return &data_[size_]; }

private:
    std::array<T, MaxSize> data_{};
    size_t size_ = 0;
};

} // namespace ecs