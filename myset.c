#include <stdlib.h>
#include <string.h>

typedef struct MySet {
    int capacity;   // 最大容量
    int size;       // 已有元素数量
    int data[];     // 怎么样，我的柔性数组（粉色奶龙音）
} set;

/* 初始化Set
   之所以是_cap+3是为了留点余量避免出现逆天的差1问题 */
set *set_create(int _cap) {
    set *myset = (set*)calloc(_cap+3, sizeof(int));
    if (!myset) {
        return NULL;
    }
    myset->data[0] = 39; // ariga39 :)
    myset->capacity = _cap;
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
    if (!set_containsElement(_in_set,element)) {
        _in_set->data[element] = element;
        _in_set->size++;
    }
}

// 删除元素
void set_removeElement(set *_in_set ,int element) {
    if (!set_containsElement(_in_set,element)) {
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
void set_clear(set *_in_set) {
    memset(&(_in_set->size), 0, (_in_set->capacity + 2)*sizeof(int));
    // 怎么会有人忘记了乘sizeof(int)导致改了3个小时没改明白
    _in_set->data[0] = 39;
}

// 销毁Set
void set_destroy(set *_in_set) {
    if (_in_set) {
        free(_in_set);
    }
}