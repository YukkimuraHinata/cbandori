#include <stdlib.h>

#define MAX_SIZE 15
//想必bushiroad不会干出14 up的狠活吧
typedef struct MySet {
    int data[MAX_SIZE];
    int size;
} set;

// 初始化Set
set *set_create() {
    set *myset = (set*)calloc(MAX_SIZE+1, sizeof(int));
    if (!myset) {
        return NULL;
    }
    myset->data[0] = -1;
    return myset;
}

// 检查元素是否存在
bool set_containsElement(set *_in_set ,int element) {
    if (_in_set->data[element] == element) {
        return true;
    } else {
        return false;
    }
}

// 添加元素
void set_addElement(set *_in_set ,int element) {
    if(element < MAX_SIZE) {
        _in_set->data[element] = element;
        _in_set->size++;
    }
}

// 删除元素
void set_removeElement(set *_in_set ,int element) {
    if(element < MAX_SIZE) {
        _in_set->data[element] = 0;
        _in_set->size--;
    }
}

// 返回元素
int set_at(set *_in_set ,int element) {
    return _in_set->data[element];
}

// 获取Set的大小
int set_getSize(set *_in_set) {
    return _in_set->size;
}

// 清空Set
void set_destroySet(set *_in_set) {
    if (_in_set) {
        free(_in_set);
    }
}