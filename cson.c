/**
 * @file cson.c
 * @author Aki
 * @author Letter (Original Author)
 * @version 1.0.0
 * @date 2026-02-05
 *
 * @copyright (c) 2019 Letter
 * @copyright (c) 2026 Aki
 *
 */

#include "cson.h"
#include "cJSON.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"

/**
 * @brief 基本类型链表数据模型
 *
 */
cson_model_t g_cson_basic_list_model[] =
    {
        [0] = {CSON_TYPE_OBJ, NULL, 0, .param.obj_size = sizeof(char)},
        [1] = {CSON_TYPE_CHAR, NULL, 0},
        [2] = {CSON_TYPE_OBJ, NULL, 0, .param.obj_size = sizeof(short)},
        [3] = {CSON_TYPE_SHORT, NULL, 0},
        [4] = {CSON_TYPE_OBJ, NULL, 0, .param.obj_size = sizeof(int)},
        [5] = {CSON_TYPE_INT, NULL, 0},
        [6] = {CSON_TYPE_OBJ, NULL, 0, .param.obj_size = sizeof(long)},
        [7] = {CSON_TYPE_LONG, NULL, 0},
        [8] = {CSON_TYPE_OBJ, NULL, 0, .param.obj_size = sizeof(float)},
        [9] = {CSON_TYPE_FLOAT, NULL, 0},
        [10] = {CSON_TYPE_OBJ, NULL, 0, .param.obj_size = sizeof(double)},
        [11] = {CSON_TYPE_DOUBLE, NULL, 0},
        [12] = {CSON_TYPE_OBJ, NULL, 0, .param.obj_size = sizeof(char *)},
        [13] = {CSON_TYPE_STRING, NULL, 0},
};

/**
 * @brief cson
 *
 */
static struct
{
    void *(*malloc)(int);
    void (*free)(void *);
} s_cson;

/**
 * @brief CSON初始化
 *
 * @param malloc 内存分配函数
 * @param free 内存释放函数
 */
void cson_init(void *malloc_func, void *free_func)
{
    s_cson.malloc = (void *(*)(int))malloc_func;
    s_cson.free = (void (*)(void *))free_func;
    cJSON_InitHooks(&(cJSON_Hooks){(void *(*)(size_t))s_cson.malloc, s_cson.free});
}

static signed char _cson_is_basic_list_model(cson_model_t *model)
{
    return (model >= &g_cson_basic_list_model[0] && model <= &g_cson_basic_list_model[13]) ? 1 : 0;
}

/**
 * @brief 解析JSON整型
 *
 * @param json JSON对象
 * @param key key
 * @return int 解析出的整型数
 */
static int _cson_decode_number(cJSON *json, char *key)
{
    cJSON *item = key ? cJSON_GetObjectItem(json, key) : json;
    if (item && item->type == cJSON_Number)
    {
        return item->valueint;
    }
    return 0;
}

/**
 * @brief 解析JSON浮点型
 *
 * @param json JSON对象
 * @param key key
 * @return double 解析出的浮点型数
 */
static double _cson_decode_double(cJSON *json, char *key)
{
    cJSON *item = key ? cJSON_GetObjectItem(json, key) : json;
    if (item && item->type == cJSON_Number)
    {
        return item->valuedouble;
    }
    return 0.0;
}

/**
 * @brief 解析JSON字符串数据
 *
 * @param json JSON对象
 * @param key key
 * @return char* 解析出的字符串
 */
static char *_cson_decode_string(cJSON *json, char *key)
{
    char *p = NULL;
    char *str = NULL;
    short str_len = 0;
    cJSON *item = key ? cJSON_GetObjectItem(json, key) : json;
    if (item && item->type == cJSON_String)
    {
        str = item->valuestring;
        if (item->valuestring)
        {
            str_len = strlen(str);
            p = s_cson.malloc(str_len + 1);
            if (p)
            {
                memcpy((void *)p, (void *)str, str_len);
                *(p + str_len) = 0;
                return p;
            }
        }
    }
    return NULL;
}

/**
 * @brief 解析JOSN布尔型数据
 *
 * @param json JSON对象
 * @param key key
 * @return char 解析出的bool
 */
static char _cson_decode_bool(cJSON *json, char *key)
{
    cJSON *item = cJSON_GetObjectItem(json, key);
    if (item && item->type == cJSON_True)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief 解析CsonList数据
 *
 * @param json JSON对象
 * @param key key
 * @param model CsonList成员数据模型
 * @param model_size SconList成员模型数量
 * @return void* CsonList对象
 */
static void *_cson_decode_list(cJSON *json, char *key, cson_model_t *model, int model_size)
{
    cson_list_t *list = NULL;
    cJSON *array = cJSON_GetObjectItem(json, key);

    if (array && array->type == cJSON_Array)
    {
        for (short i = 0; i < cJSON_GetArraySize(array); i++)
        {
            void *obj = cson_decode_object(cJSON_GetArrayItem(array, i), model, model_size);
            if (_cson_is_basic_list_model(model))
            {
                list = cson_list_add(list, (void *)(*((int *)obj)));
                s_cson.free(obj);
            }
            else
            {
                list = cson_list_add(list, obj);
            }
        }
    }
    return list;
}

/**
 * @brief 解析数组
 *
 * @param json json对象
 * @param key key
 * @param base 数组基址
 * @param element_type 数组元素类型
 * @param array_size 数组大小
 */
static void _cson_decode_array(cJSON *json, char *key, void *base, cson_type_t element_type, short array_size)
{
    cJSON *array = cJSON_GetObjectItem(json, key);
    cJSON *item;
    char *str;
    short str_len;

    if (array && array->type == cJSON_Array)
    {
        for (short i = 0; i < cJSON_GetArraySize(array); i++)
        {
            item = cJSON_GetArrayItem(array, i);
            switch (element_type)
            {
            case CSON_TYPE_CHAR:
                *(char *)((size_t)base + (i * sizeof(char))) = (char)item->valueint;
                break;
            case CSON_TYPE_SHORT:
                *(short *)((size_t)base + (i * sizeof(short))) = (short)item->valueint;
                break;
            case CSON_TYPE_INT:
                *(int *)((size_t)base + (i * sizeof(int))) = (int)item->valueint;
                break;
            case CSON_TYPE_LONG:
                *(long *)((size_t)base + (i * sizeof(long))) = (long)item->valueint;
                break;
            case CSON_TYPE_FLOAT:
                *(float *)((size_t)base + (i * sizeof(float))) = (float)item->valuedouble;
                break;
            case CSON_TYPE_DOUBLE:
                *(double *)((size_t)base + (i * sizeof(double))) = (double)item->valuedouble;
                break;
            case CSON_TYPE_STRING:
                str_len = strlen(item->valuestring);
                str = s_cson.malloc(str_len + 1);
                memcpy(str, item->valuestring, str_len);
                *(str + str_len) = 0;
                *(size_t *)((size_t)base + (i * sizeof(size_t))) = (size_t)str;
                break;
            default:
                break;
            }
        }
    }
}

/**
 * @brief 解析JSON对象
 *
 * @param json JSON对象
 * @param model 数据模型
 * @param model_size 数据模型数量
 * @return void* 解析得到的对象
 */
void *cson_decode_object(cJSON *json, cson_model_t *model, int model_size)
{
    CSON_ASSERT(json, return NULL);

    if (json->type == cJSON_NULL)
    {
        return NULL;
    }

    short obj_size = 0;
    for (short i = 0; i < model_size; i++)
    {
        if (model[i].type == CSON_TYPE_OBJ)
        {
            obj_size = model[i].param.obj_size;
        }
    }
    void *obj = s_cson.malloc(obj_size);
    CSON_ASSERT(obj, return NULL);

    for (short i = 0; i < model_size; i++)
    {
        switch (model[i].type)
        {
        case CSON_TYPE_CHAR:
            *(char *)((size_t)obj + model[i].offset) = (char)_cson_decode_number(json, model[i].key);
            break;
        case CSON_TYPE_SHORT:
            *(short *)((size_t)obj + model[i].offset) = (short)_cson_decode_number(json, model[i].key);
            break;
        case CSON_TYPE_INT:
            *(int *)((size_t)obj + model[i].offset) = (int)_cson_decode_number(json, model[i].key);
            break;
        case CSON_TYPE_LONG:
            *(long *)((size_t)obj + model[i].offset) = (long)_cson_decode_number(json, model[i].key);
            break;
        case CSON_TYPE_FLOAT:
            *(float *)((size_t)obj + model[i].offset) = (float)_cson_decode_double(json, model[i].key);
            break;
        case CSON_TYPE_DOUBLE:
            *(double *)((size_t)obj + model[i].offset) = _cson_decode_double(json, model[i].key);
            break;
        case CSON_TYPE_BOOL:
            *(char *)((size_t)obj + model[i].offset) = (char)_cson_decode_bool(json, model[i].key);
            break;
        case CSON_TYPE_STRING:
            *(size_t *)((size_t)obj + model[i].offset) = (size_t)_cson_decode_string(json, model[i].key);
            break;
        case CSON_TYPE_LIST:
            *(size_t *)((size_t)obj + model[i].offset) = (size_t)_cson_decode_list(json,
                                                                                   model[i].key, model[i].param.sub.model, model[i].param.sub.size);
            break;
        case CSON_TYPE_STRUCT:
            *(size_t *)((size_t)obj + model[i].offset) = (size_t)cson_decode_object(
                cJSON_GetObjectItem(json, model[i].key),
                model[i].param.sub.model, model[i].param.sub.size);
            break;
        case CSON_TYPE_ARRAY:
            _cson_decode_array(json, model[i].key, (void *)((size_t)obj + model[i].offset),
                               model[i].param.array.ele_type, model[i].param.array.size);
            break;
        case CSON_TYPE_JSON:
            *(size_t *)((size_t)obj + model[i].offset) = (size_t)cJSON_PrintUnformatted(
                cJSON_GetObjectItem(json, model[i].key));
            break;
        default:
            break;
        }
    }
    return obj;
}

/**
 * @brief 解析JSON字符串
 *
 * @param json_str json字符串
 * @param model 数据模型
 * @param model_size 数据模型数量
 * @return void* 解析得到的对象
 */
void *cson_decode(const char *json_str, cson_model_t *model, int model_size)
{
    void *obj;
    cJSON *json = cJSON_Parse(json_str);
    CSON_ASSERT(json, return NULL);
    obj = cson_decode_object(json, model, model_size);
    cJSON_Delete(json);
    return obj;
}

/**
 * @brief 数字编码编码JSON
 *
 * @param json json对象
 * @param key key
 * @param num 数值
 */
static void _cson_encode_number(cJSON *json, char *key, double num)
{
    if (key)
    {
        cJSON_AddNumberToObject(json, key, num);
    }
    else
    {
        json->type = cJSON_Number;
        json->valuedouble = num;
        json->valueint = (int)num;
    }
}

/**
 * @brief 字符串编码编码JSON
 *
 * @param json json对象
 * @param key key
 * @param str 字符串
 */
static void _cson_encode_string(cJSON *json, char *key, char *str)
{
    if (key)
    {
        cJSON_AddStringToObject(json, key, str);
    }
    else
    {
        json->type = cJSON_String;
        json->valuestring = str;
    }
}

/**
 * @brief CsonList编码成JSON对象
 *
 * @param list CsonList对象
 * @param model 数据模型
 * @param model_size 数据模型数量
 * @return cJSON* 编码得到的JOSN对象
 */
static cJSON *_cson_encode_list(cson_list_t *list, cson_model_t *model, int model_size)
{
    cJSON *root = cJSON_CreateArray();
    cJSON *item;
    cson_list_t *p = list;

    while (p)
    {
        if (p->obj)
        {
            if (_cson_is_basic_list_model(model))
            {
                item = cson_encode_object(&(p->obj), model, model_size);
            }
            else
            {
                item = cson_encode_object(p->obj, model, model_size);
            }
            cJSON_AddItemToArray(root, item);
        }
        p = p->next;
    }
    return root;
}

/**
 * @brief 数组编码成JSON对象
 *
 * @param base 数组基址
 * @param element_type 数组元素类型
 * @param array_size 数组大小
 * @return cJSON* 编码得到的JOSN对象
 */
static cJSON *_cson_encode_array(void *base, cson_type_t element_type, short array_size)
{
    cJSON *root = cJSON_CreateArray();
    cJSON *item;

    for (short i = 0; i < array_size; i++)
    {
        item = NULL;
        switch (element_type)
        {
        case CSON_TYPE_CHAR:
            item = cJSON_CreateNumber(*(char *)((size_t)base + (i * sizeof(char))));
            break;
        case CSON_TYPE_SHORT:
            item = cJSON_CreateNumber(*(short *)((size_t)base + (i * sizeof(short))));
            break;
        case CSON_TYPE_INT:
            item = cJSON_CreateNumber(*(int *)((size_t)base + (i * sizeof(int))));
            break;
        case CSON_TYPE_LONG:
            item = cJSON_CreateNumber(*(long *)((size_t)base + (i * sizeof(long))));
            break;
        case CSON_TYPE_FLOAT:
            item = cJSON_CreateNumber(*(float *)((size_t)base + (i * sizeof(float))));
            break;
        case CSON_TYPE_DOUBLE:
            item = cJSON_CreateNumber(*(double *)((size_t)base + (i * sizeof(double))));
            break;
        case CSON_TYPE_STRING:
            item = cJSON_CreateString((char *)*(size_t *)((size_t)base + (i * sizeof(size_t))));
            break;
        default:
            break;
        }
        if (item)
            cJSON_AddItemToArray(root, item);
    }
    return root;
}

/**
 * @brief 编码JSON对象
 *
 * @param obj 对象
 * @param model 数据模型
 * @param model_size 数据模型数量
 * @return cJSON* 编码得到的json对象
 */
cJSON *cson_encode_object(void *obj, cson_model_t *model, int model_size)
{
    if (!obj)
    {
        return cJSON_CreateNull();
    }
    cJSON *root = cJSON_CreateObject();

    for (short i = 0; i < model_size; i++)
    {
        switch (model[i].type)
        {
        case CSON_TYPE_CHAR:
            _cson_encode_number(root, model[i].key, *(char *)((size_t)obj + model[i].offset));
            break;
        case CSON_TYPE_SHORT:
            _cson_encode_number(root, model[i].key, *(short *)((size_t)obj + model[i].offset));
            break;
        case CSON_TYPE_INT:
            _cson_encode_number(root, model[i].key, *(int *)((size_t)obj + model[i].offset));
            break;
        case CSON_TYPE_LONG:
            _cson_encode_number(root, model[i].key, *(long *)((size_t)obj + model[i].offset));
            break;
        case CSON_TYPE_FLOAT:
            _cson_encode_number(root, model[i].key, *(float *)((size_t)obj + model[i].offset));
            break;
        case CSON_TYPE_DOUBLE:
            _cson_encode_number(root, model[i].key, *(double *)((size_t)obj + model[i].offset));
            break;
        case CSON_TYPE_BOOL:
            cJSON_AddBoolToObject(root, model[i].key, *(char *)((size_t)obj + model[i].offset));
            break;
        case CSON_TYPE_STRING:
            if ((char *)(*(size_t *)((size_t)obj + model[i].offset)))
            {
                _cson_encode_string(root, model[i].key, (char *)(*(size_t *)((size_t)obj + model[i].offset)));
            }
            break;
        case CSON_TYPE_LIST:
            if ((cson_list_t *)*(size_t *)((size_t)obj + model[i].offset))
            {
                cJSON_AddItemToObject(root, model[i].key,
                                      _cson_encode_list((cson_list_t *)*(size_t *)((size_t)obj + model[i].offset),
                                                        model[i].param.sub.model, model[i].param.sub.size));
            }
            break;
        case CSON_TYPE_STRUCT:
            if ((void *)(*(size_t *)((size_t)obj + model[i].offset)))
            {
                cJSON_AddItemToObject(root, model[i].key, cson_encode_object((void *)(*(size_t *)((size_t)obj + model[i].offset)), model[i].param.sub.model, model[i].param.sub.size));
            }
            break;
        case CSON_TYPE_ARRAY:
            cJSON_AddItemToObject(root, model[i].key, _cson_encode_array((void *)((size_t)obj + model[i].offset), model[i].param.array.ele_type, model[i].param.array.size));
            break;
        case CSON_TYPE_JSON:
            if ((char *)(*(size_t *)((size_t)obj + model[i].offset)))
            {
                cJSON_AddItemToObject(root, model[i].key,
                                      cJSON_Parse((char *)(*(size_t *)((size_t)obj + model[i].offset))));
            }
            break;
        default:
            break;
        }
    }
    return root;
}

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
char *cson_encode(void *obj, cson_model_t *model, int model_size, int buffer_size, int fmt)
{
    cJSON *json = cson_encode_object(obj, model, model_size);
    CSON_ASSERT(json, return NULL);
    char *json_str = cJSON_PrintBuffered(json, buffer_size, fmt);
    cJSON_Delete(json);
    return json_str;
}

/**
 * @brief 编码成json字符串
 *
 * @param obj 对象
 * @param model 数据模型
 * @param model_size 数据模型数量
 * @return char* 编码得到的json字符串
 */
char *cson_encode_unformatted(void *obj, cson_model_t *model, int model_size)
{
    cJSON *json = cson_encode_object(obj, model, model_size);
    CSON_ASSERT(json, return NULL);
    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return json_str;
}

/**
 * @brief 释放CSON解析出的对象
 *
 * @param obj 对象
 * @param model 对象模型
 * @param model_size 对象模型数量
 */
void cson_free(void *obj, cson_model_t *model, int model_size)
{
    cson_list_t *list, *p;
    size_t *tmp_node;

    for (short i = 0; i < model_size; i++)
    {
        switch ((int)model[i].type)
        {
        case CSON_TYPE_CHAR:
        case CSON_TYPE_SHORT:
        case CSON_TYPE_INT:
        case CSON_TYPE_LONG:
        case CSON_TYPE_FLOAT:
        case CSON_TYPE_DOUBLE:
            break;
        case CSON_TYPE_STRING:
        case CSON_TYPE_JSON:
            s_cson.free((char *)(*(size_t *)((size_t)obj + model[i].offset)));
            break;
        case CSON_TYPE_LIST:
            list = (cson_list_t *)*(size_t *)((size_t)obj + model[i].offset);
            while (list)
            {
                p = list;
                list = list->next;
                if (p->obj)
                {
                    if (_cson_is_basic_list_model(model[i].param.sub.model))
                    {
                        tmp_node = s_cson.malloc(sizeof(size_t));
                        *tmp_node = (size_t)(&(p->obj));
                    }
                    else
                    {
                        tmp_node = p->obj;
                    }
                    cson_free(tmp_node,
                              model[i].param.sub.model, model[i].param.sub.size);
                }
                s_cson.free(p);
            }
            break;
        case CSON_TYPE_STRUCT:
            cson_free((void *)(*(size_t *)((size_t)obj + model[i].offset)),
                      model[i].param.sub.model, model[i].param.sub.size);
            break;
        case CSON_TYPE_ARRAY:
            if (model[i].param.array.ele_type == CSON_TYPE_STRING)
            {
                for (short j = 0; j < model[i].param.array.size; j++)
                {
                    if (*(size_t *)((size_t)obj + model[i].offset + (j * sizeof(size_t))))
                    {
                        s_cson.free((void *)*(size_t *)((size_t)obj + model[i].offset + (j * sizeof(size_t))));
                    }
                }
            }
            break;
        default:
            break;
        }
    }
    s_cson.free(obj);
}

/**
 * @brief 释放cson编码生成的json字符串
 *
 * @param json_str json字符串
 */
void cson_free_json(const char *json_str)
{
    CSON_ASSERT(json_str, return);
    s_cson.free((void *)json_str);
}

/**
 * @brief CSON链表添加节点
 *
 * @param list 链表
 * @param obj 节点对象
 * @return cson_list_t 链表
 */
cson_list_t *cson_list_add(cson_list_t *list, void *obj)
{
    if (!list)
    {
        list = s_cson.malloc(sizeof(cson_list_t));
        if (!list)
        {
            return NULL;
        }
        list->next = NULL;
        list->obj = NULL;
    }
    cson_list_t *p = list;
    while (p->next)
    {
        p = p->next;
    }
    if (!p->obj)
    {
        p->obj = obj;
        p->next = NULL;
    }
    else
    {
        cson_list_t *node = s_cson.malloc(sizeof(cson_list_t));
        if (node)
        {
            node->obj = obj;
            node->next = NULL;
            p->next = node;
        }
    }
    return list;
}

/**
 * @brief CSON链表删除节点
 *
 * @param list 链表
 * @param obj 节点对象
 * @param free_mem 释放内存
 * @return cson_list_t 链表
 */
cson_list_t *cson_list_delete(cson_list_t *list, void *obj, char free_mem)
{
    CSON_ASSERT(list, return NULL);

    cson_list_t head = {0};
    head.next = list;
    cson_list_t *p = &head;
    cson_list_t *tmp;
    while (p->next)
    {
        if (p->next->obj && p->next->obj == obj)
        {
            tmp = p->next;
            p->next = p->next->next ? p->next->next : NULL;
            if (free_mem)
            {
                s_cson.free(tmp->obj);
                s_cson.free(tmp);
            }
            break;
        }
        p = p->next;
    }
    return head.next;
}

/**
 * @brief CSON新字符串
 *
 * @param src 源字符串
 * @return char* 新字符串
 * @note 此函数用于复制字符串，建议对结构体中字符串成员赋值时，使用此函数，
 *       方便使用`cson_free`进行内存释放
 */
char *cson_new_string(const char *src)
{
    int len = strlen(src);
    char *dest = s_cson.malloc(len + 1);
    strcpy(dest, src);
    return dest;
}
