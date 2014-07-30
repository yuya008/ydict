#ifndef __ydict_h__
#define __ydict_h__

#include <stdint.h>
#include <string.h>

#define string_key(s)    s, strlen(s)
#define key(s)           s, sizeof(*(s))

typedef struct ydict_t      ydict_t;        /* 主结构 */
typedef struct ydict_list   ydict_list;     /* 双向链表结构 */
typedef struct ydict_bucket ydict_bucket;   /* hash表 */
typedef struct ydict_node   ydict_node;     /* 主节点 */

struct ydict_t
{
    uint64_t count;        /* 节点总数 */
    ydict_list *list;      /* 双向链表 */
    ydict_bucket *bucket;  /* hash表 */
    uint32_t bucket_size;  /* hash表桶数 */
};

struct ydict_list
{
    ydict_node *top;     /* 双向链表首指针 */
    ydict_node *cur;     /* 双向链表尾指针 */
    uint64_t last_index; /* 最后索引值 */
};

struct ydict_bucket
{
    ydict_node *first; /* 桶中的节点首指针 */
};

struct ydict_node
{
    ydict_node *list_nxt; /* 双向链表next指针 */
    ydict_node *list_pre; /* 双向链表pre指针 */
    ydict_node *key_nxt;  /* 桶中节点next指针 */
    uint64_t index;       /* 节点自然索引 */
    void *value;          /* value指针 */
    size_t key_size;      /* key尺寸 */
    char key_data[];      /* key数据 */
};

/* ydict初始化 */
ydict_t *ydict_init();
/* 放入键值对 */
int ydict_put(ydict_t*, const char*, size_t, void*);
/* 用键取值 */
void *ydict_get(ydict_t*, const char*, size_t);
/* 通过自然索引获取 */
void *ydict_get_index(ydict_t*, uint64_t);
/* 取得元素个数 */
uint64_t ydict_size(ydict_t*);
/* 清空ydict */
void ydict_clear(ydict_t*);
/* 用索引删除元素 */
int remove_index(ydict_t*, uint64_t);
/* 用key删除元素 */
int remove_key(ydict_t*, const char*, size_t);

#endif