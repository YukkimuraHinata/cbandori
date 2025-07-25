#ifndef MySet_H
#define MySet_H
#endif
typedef struct MySet set;

// 初始化Set
set* set_create();

// 检查元素是否存在
bool set_containsElement(set *_in_set ,int element);

// 添加元素
void set_addElement(set *_in_set ,int element);

// 删除元素
void set_removeElement(set *_in_set ,int element);

// 返回元素
int set_at(set *_in_set ,int index);

// 获取Set的大小
int set_getSize(set *_in_set);

// 清空Set
void set_destroySet(set *_in_set);
