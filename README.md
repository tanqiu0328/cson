# CSON

![version](https://img.shields.io/badge/version-1.0.0-brightgreen.svg)
![build](https://img.shields.io/badge/build-2026.02.05-brightgreen.svg)
![license](https://img.shields.io/badge/license-MIT-brightgreen.svg)

基于cJSON，运行于C语言平台的json-struct模型解析工具

## 快速开始

### 1. 引入项目

将 `cson.c`、`cson.h` 以及 `cJSON` 库（`cJSON.c`, `cJSON.h`）添加到你的工程中

### 2. 初始化

在使用 CSON 之前，需要指定内存管理函数：

```c
#include "cson.h"
#include <stdlib.h>

void app_main(void)
{
    // 初始化 CSON，指定内存分配和释放函数
    cson_init(malloc, free);
}
```

### 3. 定义数据模型

假设有一个用户对象，包含 ID、用户名和年龄：

```c
// 定义 C 结构体
typedef struct {
    int id;
    char *name;
    int age;
} user_t;

// 定义映射模型
cson_model_t user_model[] = {
    CSON_MODEL_OBJ(user_t),             // 绑定对象大小
    CSON_MODEL_INT(user_t, id),         // 映射整型 id
    CSON_MODEL_STRING(user_t, name),    // 映射字符串 name
    CSON_MODEL_INT(user_t, age),        // 映射整型 age
};
```

### 4. 反序列化 (JSON->Struct)

```c
const char *json_str = "{\"id\": 1, \"name\": \"Aki\", \"age\": 18}";

// 解析 JSON 字符串
user_t *user = cson_decode_ex(json_str, user_model);

if (user) {
    printf("User ID: %d, Name: %s, Age: %d\n", user->id, user->name, user->age);

    // 使用完后释放对象及内部成员内存
    cson_free_ex(user, user_model);
}
```

### 5. 序列化 (Struct->JSON)

```c
user_t user = {
    .id = 101,
    .name = "Letter",
    .age = 25
};

// 编码为格式化的 JSON 字符串
char *json_res = cson_encode_ex(&user, user_model, 256, 1);

if (json_res) {
    printf("Serialized JSON:\n%s\n", json_res);
    // 释放生成的 JSON 字符串内存
    cson_free_json(json_res);
}
```

## 高阶用法

### 处理链表
CSON 内置了 `cson_list_t` 来处理动态数量的 JSON 数组

```c
typedef struct {
    cson_list_t *tags; // 字符串链表
} blog_t;

cson_model_t blog_model[] = {
    CSON_MODEL_OBJ(blog_t),
    CSON_MODEL_LIST(blog_t, tags, CSON_MODEL_STRING_LIST, CSON_BASIC_LIST_MODEL_SIZE),
};
```

### 处理结构体嵌套
使用 `CSON_MODEL_STRUCT` 宏可以轻松处理复杂的嵌套 JSON 结构
