#ifndef MyVector_H
#define MyVector_H
typedef struct MyVector vector;

/* 创建和销毁函数 */
/**
 * @brief 创建新的Vector
 * 
 * @param elem_size 每个元素的大小（字节）
 * @return vector* 成功返回Vector指针，失败返回NULL
 */
vector* vector_create(size_t elem_size);

/**
 * @brief 销毁Vector并释放所有内存
 * 
 * @param vec 目标Vector指针
 */
void vector_destroy(vector* vec);

/* 元素访问操作 */
/**
 * @brief 获取指定位置的元素
 * 
 * @param vec 目标Vector
 * @param index 元素索引（0-based）
 * @return void* 成功返回元素指针，失败返回NULL
 */
void* vector_at(vector* vec, size_t index);

/**
 * @brief 获取Vector首元素
 * 
 * @param vec 目标Vector
 * @return void* 首元素指针，Vector为空时返回NULL
 */
void* vector_front(vector* vec);

/**
 * @brief 获取Vector尾元素
 * 
 * @param vec 目标Vector
 * @return void* 尾元素指针，Vector为空时返回NULL
 */
void* vector_back(vector* vec);

/**
 * @brief 获取底层数据数组
 * 
 * @param vec 目标Vector
 * @return void* 数据数组指针，Vector为空时返回NULL
 */
void* vector_data(vector* vec);

/* 容量操作 */
/**
 * @brief 获取当前元素数量
 * 
 * @param vec 目标Vector
 * @return size_t 元素数量
 */
size_t vector_size(const vector* vec);

/**
 * @brief 获取当前容量
 * 
 * @param vec 目标Vector
 * @return size_t 容量大小
 */
size_t vector_capacity(const vector* vec);

/**
 * @brief 检查Vector是否为空
 * 
 * @param vec 目标Vector
 * @return true 为空
 * @return false 非空
 */
bool vector_empty(const vector* vec);

/**
 * @brief 预留存储空间
 * 
 * @param vec 目标Vector
 * @param new_capacity 新容量
 * @return true 成功
 * @return false 失败（内存分配错误）
 */
bool vector_reserve(vector* vec, size_t new_capacity);

/**
 * @brief 缩容到实际大小
 * 
 * @param vec 目标Vector
 * @return true 成功
 * @return false 失败（内存分配错误）
 */
bool vector_shrink_to_fit(vector* vec);

/* 修改操作 */
/**
 * @brief 清空Vector（保留容量）
 * 
 * @param vec 目标Vector
 */
void vector_clear(vector* vec);

/**
 * @brief 在尾部插入元素
 * 
 * @param vec 目标Vector
 * @param element 待插入元素的指针
 * @return true 成功
 * @return false 失败（内存分配错误）
 */
bool vector_push_back(vector* vec, const void* element);

/**
 * @brief 移除尾部元素
 * 
 * @param vec 目标Vector
 * @return true 成功
 * @return false 失败（Vector为空）
 */
bool vector_pop_back(vector* vec);

/**
 * @brief 在指定位置插入元素
 * 
 * @param vec 目标Vector
 * @param index 插入位置索引
 * @param element 待插入元素的指针
 * @return true 成功
 * @return false 失败（索引越界或内存分配错误）
 */
bool vector_insert(vector* vec, size_t index, const void* element);

/**
 * @brief 删除指定位置元素
 * 
 * @param vec 目标Vector
 * @param index 待删除元素的索引
 * @return true 成功
 * @return false 失败（索引越界）
 */
bool vector_erase(vector* vec, size_t index);

/**
 * @brief 交换两个Vector内容
 * 
 * @param a 第一个Vector
 * @param b 第二个Vector
 */
void vector_swap(vector* a, vector* b);

/* 特殊操作 */
/**
 * @brief 对每个元素执行操作
 * 
 * @param vec 目标Vector
 * @param func 函数指针，接收元素指针和上下文参数
 * @param context 传递给函数的上下文参数
 */
void vector_for_each(vector* vec, void (*func)(void*, void*), void* context);
#endif