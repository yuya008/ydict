#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ydict.h"

static uint32_t default_bucket_size = 1024;

/*
    内存分配
*/
static inline void *new(size_t size)
{
    void *d = malloc(size);
    if (!d) {
        fprintf(stderr, "%s\n", "oom");
        exit(1);
    }
    memset(d, 0, size);
    return d;
}

/*
    计算key的hash值
*/
static inline uint64_t hash_code(const char *key, size_t key_size)
{
    uint64_t hash_code = 0;
    for (;key_size;key_size--) {
        hash_code = hash_code + *key++;
        hash_code++;
    }
    return hash_code;
}

/*
    创建一个node节点
*/
static inline ydict_node *ydict_create_node(const char *key, size_t key_size, void *value)
{
    ydict_node *new_node = new(sizeof(ydict_node) + key_size);
    new_node->value = value;
    new_node->key_size = key_size;
    memcpy(new_node->key_data, key, key_size);
    return new_node;
}

/*
    元素放入list
*/
static inline void ydict_put_list(ydict_t *d, ydict_node *new_node)
{
    ydict_list *list = d->list;
    new_node->index = list->last_index;
    list->last_index++;

    if (!list->top) {
        list->top = new_node;
        list->cur = new_node;
    } else {
        new_node->list_pre = list->cur;
        list->cur->list_nxt = new_node;
        list->cur = new_node;
    }
}

/*
    获取相应的桶位
*/
static inline ydict_bucket *ydict_get_bucket(ydict_t *d, const char *key, size_t key_size)
{
    uint64_t hash_val = hash_code(key, key_size);
    ydict_bucket *bucket = d->bucket;
    bucket = bucket + (hash_val % d->bucket_size);
    return bucket;
}

/*
    放入dict
*/
static inline void ydict_put_dict(ydict_t *d, ydict_node *new_node, const char *key, size_t key_size)
{
    ydict_bucket *bucket = ydict_get_bucket(d, key, key_size);
    if (!bucket->first) {
        bucket->first = new_node;
    } else {
        new_node->key_nxt = bucket->first;
        bucket->first = new_node;
    }
}

/*
    替换
*/
static inline int ydict_replace_dict(ydict_t *d, const char *key, size_t key_size, void *value)
{
    ydict_bucket *bucket = ydict_get_bucket(d, key, key_size);
    if (!bucket->first) {
        return 1;
    }
    ydict_node *iter = bucket->first;
    for (;iter;iter = iter->key_nxt) {
        if (iter->key_size == key_size &&
            memcmp(iter->key_data, key, key_size) == 0) {
            iter->value = value;
            return 0;
        }
    }
    return 1;
}

/*
    从list中删除
*/
static inline ydict_node *ydict_remove_list(ydict_t *d, uint64_t index)
{
    ydict_list *list = d->list;
    if (!list->top) {
        return NULL;
    }
    ydict_node *node = NULL;
    for (node = list->top; node; node = node->list_nxt) {
        if (node->index > index) {
            return NULL;
        }
        if (node->index == index) {
            break;
        }
    }
    if (!node) {
        return NULL;
    }
    if (!node->list_pre && !node->list_nxt) {
        list->cur = list->top = NULL;
        goto ret;
    }
    if (!node->list_pre && node->list_nxt) {
        node->list_nxt->list_pre = NULL;
        list->top = node->list_nxt;
        goto ret;
    }
    if (node->list_pre && !node->list_nxt) {
        node->list_pre->list_nxt = NULL;
        list->cur = node->list_pre;
        goto ret;
    }
    node->list_nxt->list_pre = node->list_pre;
    node->list_pre->list_nxt = node->list_nxt;
    ret:
    return node;
}

/*
    从dict中删除
*/
static inline ydict_node *ydict_remove_dict(ydict_t *d, const char *key_data, size_t key_size)
{
    ydict_bucket *bucket = ydict_get_bucket(d, key_data, key_size);
    if (!bucket->first) {
        return NULL;
    }
    ydict_node *iter = bucket->first;
    ydict_node *iter2 = iter;
    for (;iter;iter = iter->key_nxt) {
        if (iter->key_size == key_size &&
            memcmp(iter->key_data, key_data, key_size) == 0) {
            if (iter == bucket->first) {
                bucket->first = iter->key_nxt;
            } else {
                iter2->key_nxt = iter->key_nxt;
            }
            return iter;
        }
        iter2 = iter;
    }
    return NULL;
}

/*
    重新计算hash
*/
static void rebuild_bucket(ydict_t *d)
{
    
}

ydict_t *ydict_init()
{
    ydict_t *d = new(sizeof(ydict_t));
    d->list = new(sizeof(ydict_list));
    d->bucket = new(sizeof(ydict_bucket) * default_bucket_size);
    d->bucket_size = default_bucket_size;
    return d;
}

int ydict_put(ydict_t *d, const char *key, size_t key_size, void *value)
{
    if (!value) {
        return 1;
    }
    if (ydict_get(d, key, key_size)) {
        return ydict_replace_dict(d, key, key_size, value);
    }

    ydict_node *node = ydict_create_node(key, key_size, value);
    ydict_put_list(d, node);
    ydict_put_dict(d, node, key, key_size);
    d->count++;
    return 0;
}

void *ydict_get(ydict_t *d, const char *key, size_t key_size)
{
    ydict_bucket *bucket = ydict_get_bucket(d, key, key_size);
    if (!bucket->first) {
        return NULL;
    }
    ydict_node *iter = bucket->first;
    for (;iter;iter = iter->key_nxt) {
        if (iter->key_size == key_size &&
            memcmp(iter->key_data, key, key_size) == 0) {
            return iter->value;
        }
    }
    return NULL;
}

uint64_t ydict_size(ydict_t *d)
{
    return d->count;
}

void ydict_clear(ydict_t *d)
{
    ydict_list *list = d->list;
    ydict_node *free_ptr = NULL, *node = NULL;

    if (list->top) {
        for (node = list->top; node;) {
            free_ptr = node;
            node = node->list_nxt;
            free(free_ptr);
        }
    }
    memset(list, 0, sizeof(ydict_list));
    ydict_bucket *bucket = d->bucket;
    free(bucket);
    d->bucket = new(sizeof(ydict_bucket) * default_bucket_size);
    d->bucket_size = default_bucket_size;
    d->count = 0;
}

void *ydict_get_index(ydict_t *d, uint64_t index)
{
    ydict_list *list = d->list;
    if (!list->top) {
        return NULL;
    }
    ydict_node *node = NULL;
    for (node = list->top; node; node = node->list_nxt) {
        if (node->index > index) {
            break;
        }
        if (node->index == index) {
            return node->value;
        }
    }
    return NULL;
}

int remove_index(ydict_t *d, uint64_t index)
{
    ydict_node *node = ydict_remove_list(d, index);
    if (!node) {
        return 1;
    }
    node = ydict_remove_dict(d, node->key_data, node->key_size);
    if (!node) {
        return 1;
    }
    free(node);
    d->count--;
    return 0;
}

int remove_key(ydict_t *d, const char *key, size_t key_size)
{
    ydict_node *node = ydict_remove_dict(d, key, key_size);
    if (!node) {
        return 1;
    }
    node = ydict_remove_list(d, node->index);
    if (!node) {
        return 1;
    }
    free(node);
    d->count--;
    return 0;
}

int main(void)
{
    ydict_t *d = ydict_init();
    int a = 1000;
    ydict_put(d, string_key("a"), &a);
    int b = 2000;
    ydict_put(d, string_key("b"), &b);
    int c = 3000;
    ydict_put(d, string_key("c"), &c);
    int r = 4000;
    ydict_put(d, string_key("d"), &r);
    int e = 5000;
    ydict_put(d, string_key("e"), &e);

    // /remove_key(d, string_key("a"));
    // remove_key(d, string_key("b"));
    remove_key(d, string_key("c"));
    remove_key(d, string_key("d"));
    remove_key(d, string_key("e"));

    fprintf(stderr, "%d\n", *(int*)ydict_get_index(d, 1));
    return 0;
}
