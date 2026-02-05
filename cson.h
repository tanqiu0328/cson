/**
 * @file cson.h
 * @author Aki
 * @author Letter (Original Author)
 * @version 1.0.0
 * @date 2026-02-05
 *
 * @copyright (c) 2019 Letter
 * @copyright (c) 2026 Aki
 *
 */

#ifndef __CSON_H__
#define __CSON_H__

#include "stddef.h"
#include "cJSON.h"

/**
 * @defgroup CSON cson
 * @brief json tools for C
 * @addtogroup CSON
 * @{
 */

/**
 * @brief CSON数据类型定义
 *
 */
typedef enum
{
        CSON_TYPE_OBJ = 0,
        CSON_TYPE_CHAR,
        CSON_TYPE_SHORT,
        CSON_TYPE_INT,
        CSON_TYPE_LONG,
        CSON_TYPE_FLOAT,
        CSON_TYPE_DOUBLE,
        CSON_TYPE_BOOL,
        CSON_TYPE_STRING,
        CSON_TYPE_STRUCT,
        CSON_TYPE_LIST,
        CSON_TYPE_ARRAY,
        CSON_TYPE_JSON,
} cson_type_t;

/**
 * @brief CSON数据模型定义
 *
 */
typedef struct cson_model
{
        cson_type_t type; /**< 数据类型 */
        char *key;        /**< 元素键值 */
        short offset;     /**< 元素偏移 */
        union
        {
                struct
                {
                        struct cson_model *model; /**< 子结构体模型 */
                        short size;               /**< 子结构体模型大小 */
                } sub;                            /**< 子结构体 */
                struct
                {
                        cson_type_t ele_type; /**< 数组元素类型 */
                        short size;           /**< 数组大小 */
                } array;                      /**< 数组 */
                int obj_size;                 /**< 对象大小 */
                cson_type_t basic_list_type;  /**< 基础数据链表类型 */
        } param;
} cson_model_t;

/**
 * @brief Cson链表
 *
 */
typedef struct cson_list
{
        struct cson_list *next; /**< 下一个元素 */
        void *obj;              /**< 对象 */
} cson_list_t;

extern cson_model_t g_cson_basic_list_model[]; /**< 基础类型链表数据模型 */

#define CSON_MODEL_CHAR_LIST &g_cson_basic_list_model[0]    /**< char型链表数据模型 */
#define CSON_MODEL_SHORT_LIST &g_cson_basic_list_model[2]   /**< short型链表数据模型 */
#define CSON_MODEL_INT_LIST &g_cson_basic_list_model[4]     /**< int型链表数据模型 */
#define CSON_MODEL_LONG_LIST &g_cson_basic_list_model[6]    /**< long型链表数据模型 */
#define CSON_MODEL_FLOAT_LIST &g_cson_basic_list_model[8]   /**< float型链表数据模型 */
#define CSON_MODEL_DOUBLE_LIST &g_cson_basic_list_model[10] /**< double型链表数据模型 */
#define CSON_MODEL_STRING_LIST &g_cson_basic_list_model[12] /**< string型链表数据模型 */

#define CSON_BASIC_LIST_MODEL_SIZE 2 /**< 基础类型链表数据模型大小 */

/**
 * @brief 对象数据模型
 *
 * @param type 对象类型
 */
#define CSON_MODEL_OBJ(type) \
        {CSON_TYPE_OBJ, NULL, 0, .param.obj_size = sizeof(type)}

/**
 * @brief char型数据模型
 *
 * @param type 对象模型
 * @param key 数据键值
 */
#define CSON_MODEL_CHAR(type, key) \
        {CSON_TYPE_CHAR, #key, offsetof(type, key)}

/**
 * @brief short型数据模型
 *
 * @param type 对象模型
 * @param key 数据键值
 */
#define CSON_MODEL_SHORT(type, key) \
        {CSON_TYPE_SHORT, #key, offsetof(type, key)}

/**
 * @brief int型数据模型
 *
 * @param type 对象模型
 * @param key 数据键值
 */
#define CSON_MODEL_INT(type, key) \
        {CSON_TYPE_INT, #key, offsetof(type, key)}

/**
 * @brief long型数据模型
 *
 * @param type 对象模型
 * @param key 数据键值
 */
#define CSON_MODEL_LONG(type, key) \
        {CSON_TYPE_LONG, #key, offsetof(type, key)}

/**
 * @brief float型数据模型
 *
 * @param type 对象模型
 * @param key 数据键值
 */
#define CSON_MODEL_FLOAT(type, key) \
        {CSON_TYPE_FLOAT, #key, offsetof(type, key)}

/**
 * @brief double型数据模型
 *
 * @param type 对象模型
 * @param key 数据键值
 */
#define CSON_MODEL_DOUBLE(type, key) \
        {CSON_TYPE_DOUBLE, #key, offsetof(type, key)}

/**
 * @brief bool型数据模型
 *
 * @param type 对象模型
 * @param key 数据键值
 */
#define CSON_MODEL_BOOL(type, key) \
        {CSON_TYPE_CHAR, #key, offsetof(type, key)}

/**
 * @brief 字符串型数据模型
 *
 * @param type 对象模型
 * @param key 数据键值
 */
#define CSON_MODEL_STRING(type, key) \
        {CSON_TYPE_STRING, #key, offsetof(type, key)}

/**
 * @brief 结构体型数据模型
 *
 * @param type 对象模型
 * @param key 数据键值
 * @param submodel 子结构体模型
 * @param subsize 子结构体模型大小
 */
#define CSON_MODEL_STRUCT(type, key, submodel, subsize) \
        {CSON_TYPE_STRUCT, #key, offsetof(type, key), .param.sub.model = submodel, .param.sub.size = subsize}

/**
 * @brief list型数据模型
 *
 * @param type 对象模型
 * @param key 数据键值
 * @param submodel 子结构体模型
 * @param subsize 子结构体模型大小
 */
#define CSON_MODEL_LIST(type, key, submodel, subsize) \
        {CSON_TYPE_LIST, #key, offsetof(type, key), .param.sub.model = submodel, .param.sub.size = subsize}

/**
 * @brief list型数据模型
 *
 * @param type 对象模型
 * @param key 数据键值
 * @param elementType 数组元素类型
 * @param arraySize 数组大小
 */
#define CSON_MODEL_ARRAY(type, key, elementType, arraySize) \
        {CSON_TYPE_ARRAY, #key, offsetof(type, key), .param.array.ele_type = elementType, .param.array.size = arraySize}

/**
 * @brief 子json数据模型
 *
 * @param type 对象模型
 * @param key 数据键值
 */
#define CSON_MODEL_JSON(type, key) \
        {CSON_TYPE_JSON, #key, offsetof(type, key)}

/**
 * @brief CSON断言
 *
 * @param expr 表达式
 * @param action 断言失败执行动作
 */
#define CSON_ASSERT(expr, action)                                                             \
        if (!(expr))                                                                          \
        {                                                                                     \
                printf(#expr " assert failed at file: %s, line: %d\r\n", __FILE__, __LINE__); \
                action;                                                                       \
        }

/**
 * @brief CSON初始化
 *
 * @param malloc 内存分配函数
 * @param free 内存释放函数
 */
void cson_init(void *malloc_func, void *free_func);

/**
 * @brief 解析JSON对象
 *
 * @param json JSON对象
 * @param model 数据模型
 * @param model_size 数据模型数量
 * @return void* 解析得到的对象
 */
void *cson_decode_object(cJSON *json, cson_model_t *model, int model_size);

/**
 * @brief 解析JSON对象
 *
 * @param json JSON对象
 * @param model 数据模型
 * @return void* 解析得到的对象
 */
#define cson_decode_object_ex(json, model) \
        cson_decode_object(json, model, sizeof(model) / sizeof(cson_model_t))

/**
 * @brief 解析JSON字符串
 *
 * @param json_str json字符串
 * @param model 数据模型
 * @param model_size 数据模型数量
 * @return void* 解析得到的对象
 */
void *cson_decode(const char *json_str, cson_model_t *model, int model_size);

/**
 * @brief 解析JSON字符串
 *
 * @param json_str json字符串
 * @param model 数据模型
 * @return void* 解析得到的对象
 */
#define cson_decode_ex(json_str, model) \
        cson_decode(json_str, model, sizeof(model) / sizeof(cson_model_t));

/**
 * @brief 编码成json字符串
 *
 * @param obj 对象
 * @param model 数据模型
 * @param model_size 数据模型数量
 * @return char* 编码得到的json字符串
 */
cJSON *cson_encode_object(void *obj, cson_model_t *model, int model_size);

/**
 * @brief 编码成json字符串
 *
 * @param obj 对象
 * @param model 数据模型
 * @param model_size 数据模型数量
 * @param buffer_size 分配给json字符串的空间大小
 * @param fmt 是否格式化json字符串
 * @return char* 编码得到的json字符串
 */
char *cson_encode(void *obj, cson_model_t *model, int model_size, int buffer_size, int fmt);

/**
 * @brief 编码成json字符串
 *
 * @param obj 对象
 * @param model 数据模型
 * @param buffer_size 分配给json字符串的空间大小
 * @param fmt 是否格式化json字符串
 * @return char* 编码得到的json字符串
 */
#define cson_encode_ex(obj, model, buffer_size, fmt) \
        cson_encode(obj, model, sizeof(model) / sizeof(cson_model_t), buffer_size, fmt)

/**
 * @brief 编码成json字符串
 *
 * @param obj 对象
 * @param model 数据模型
 * @param model_size 数据模型数量
 * @return char* 编码得到的json字符串
 */
char *cson_encode_unformatted(void *obj, cson_model_t *model, int model_size);

/**
 * @brief 编码成json字符串
 *
 * @param obj 对象
 * @param model 数据模型
 * @return char* 编码得到的json字符串
 */
#define cson_encode_unformatted_ex(obj, model) \
        cson_encode_unformatted(obj, model, sizeof(model) / sizeof(cson_model_t))

/**
 * @brief 释放CSON解析出的对象
 *
 * @param obj 对象
 * @param model 对象模型
 * @param model_size 对象模型数量
 */
void cson_free(void *obj, cson_model_t *model, int model_size);

/**
 * @brief 释放CSON解析出的对象
 *
 * @param obj 对象
 * @param model 对象模型
 */
#define cson_free_ex(obj, model) \
        cson_free(obj, model, sizeof(model) / sizeof(cson_model_t))

/**
 * @brief 释放cson编码生成的json字符串
 *
 * @param json_str json字符串
 */
void cson_free_json(const char *json_str);

/**
 * @brief CSON链表添加节点
 *
 * @param list 链表
 * @param obj 节点对象
 * @return cson_list_t 链表
 */
cson_list_t *cson_list_add(cson_list_t *list, void *obj);

/**
 * @brief CSON链表删除节点
 *
 * @param list 链表
 * @param obj 节点对象
 * @param free_mem 释放内存
 * @return cson_list_t 链表
 */
cson_list_t *cson_list_delete(cson_list_t *list, void *obj, char free_mem);

/**
 * @brief CSON新字符串
 *
 * @param src 源字符串
 * @return char* 新字符串
 * @note 此函数用于复制字符串，建议对结构体中字符串成员赋值时，使用此函数，
 *       方便使用`cson_free`进行内存释放
 */
char *cson_new_string(const char *src);

/**
 * @}
 */

#endif
