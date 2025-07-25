#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Vector结构体定义
typedef struct MyVector{
    void* data;         // 指向动态分配内存的指针
    size_t size;        // 当前元素数量
    size_t capacity;    // 当前容量
    size_t elem_size;   // 每个元素的大小（字节）
} vector;

// 初始化Vector
vector* vector_create(size_t elem_size) {
    vector* vec = (vector*)malloc(sizeof(vector));
    if (!vec) 
    return NULL;

    vec->size = 0;
    vec->capacity = 0;
    vec->elem_size = elem_size;
    vec->data = NULL;
    return vec;
}

// 销毁Vector并释放内存
void vector_destroy(vector* vec) {
    if (vec) {
        free(vec->data);  // 先释放数据内存
        free(vec);       // 再释放结构体内存
    }
}

// 调整Vector容量（内部函数）
static bool vector_resize(vector* vec, size_t new_capacity) {
    if (new_capacity < vec->size) 
    return false;  // 新容量不能小于当前大小

    void* new_data = realloc(vec->data, new_capacity * vec->elem_size);
    if (!new_data && new_capacity > 0) 
    return false;  // 分配失败

    vec->data = new_data;
    vec->capacity = new_capacity;
    return true;
}

// 在尾部添加元素
bool vector_push_back(vector* vec, const void* element) {
    // 检查是否需要扩容
    if (vec->size >= vec->capacity) {
        // 计算新容量：初始为4，后续翻倍增长
        size_t new_cap = (vec->capacity == 0) ? 4 : vec->capacity * 2;
        if (!vector_resize(vec, new_cap)) 
        return false;
    }

    // 计算目标地址并拷贝元素
    void* target = (char*)vec->data + vec->size * vec->elem_size;
    memcpy(target, element, vec->elem_size);
    vec->size++;
    return true;
}

// 移除尾部元素
bool vector_pop_back(vector* vec) {
    if (vec->size == 0) 
    return false;
    vec->size--;  // 仅需减少大小，内存保留
    
    // 缩容策略：当元素少于容量的1/4时缩至一半
    if (vec->size > 0 && vec->size <= vec->capacity / 4) {
        size_t new_cap = vec->capacity / 2;
        vector_resize(vec, new_cap);
    }
    return true;
}

// 在指定位置插入元素
bool vector_insert(vector* vec, size_t index, const void* element) {
    if (index > vec->size) 
    return false;  // 索引越界

    // 尾部插入优化
    if (index == vec->size) return vector_push_back(vec, element);

    // 检查扩容
    if (vec->size >= vec->capacity) {
        size_t new_cap = (vec->capacity == 0) ? 4 : vec->capacity * 2;
        if (!vector_resize(vec, new_cap)) 
        return false;
    }

    // 移动插入点后的元素
    void* src = (char*)vec->data + index * vec->elem_size;
    void* dest = (char*)src + vec->elem_size;
    size_t bytes_to_move = (vec->size - index) * vec->elem_size;
    memmove(dest, src, bytes_to_move);

    // 插入新元素
    memcpy(src, element, vec->elem_size);
    vec->size++;
    return true;
}

// 删除指定位置元素
bool vector_erase(vector* vec, size_t index) {
    if (index >= vec->size) 
    return false;

    // 移动覆盖目标元素
    void* dest = (char*)vec->data + index * vec->elem_size;
    void* src = (char*)dest + vec->elem_size;
    size_t bytes_to_move = (vec->size - index - 1) * vec->elem_size;
    memmove(dest, src, bytes_to_move);
    vec->size--;

    // 缩容检查
    if (vec->size > 0 && vec->size <= vec->capacity / 4) {
        size_t new_cap = vec->capacity / 2;
        vector_resize(vec, new_cap);
    }
    return true;
}

// 获取指定位置元素
void* vector_at(vector* vec, size_t index) {
    if (index >= vec->size) return NULL;
    return (char*)vec->data + index * vec->elem_size;
}

// 获取Vector大小
size_t vector_size(const vector* vec) {
    return vec->size;
}

// 获取Vector容量
size_t vector_capacity(const vector* vec) {
    return vec->capacity;
}

// 预留存储空间
bool vector_reserve(vector* vec, size_t new_capacity) {
    if (new_capacity <= vec->capacity) return true;  // 无需操作
    return vector_resize(vec, new_capacity);
}

// 缩容到实际大小
bool vector_shrink_to_fit(vector* vec) {
    if (vec->size == 0) {
        free(vec->data);
        vec->data = NULL;
        vec->capacity = 0;
        return true;
    }
    return vector_resize(vec, vec->size);
}

// 清空Vector（保留容量）
void vector_clear(vector* vec) {
    vec->size = 0;
    // 可选：缩容到初始大小
    // vector_resize(vec, 0);
}

// 交换两个Vector
void vector_swap(vector* a, vector* b) {
    vector temp = *a;
    *a = *b;
    *b = temp;
}