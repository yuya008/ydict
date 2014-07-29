#ifndef __ydict_h__
#define __ydict_h__

#include <stdint.h>
#include <string.h>

#define string_key(s)    s, strlen(s)
#define key(s)           s, sizeof(*(s))

typedef struct ydict_t      ydict_t;
typedef struct ydict_list   ydict_list;
typedef struct ydict_bucket ydict_bucket;
typedef struct ydict_node   ydict_node;

struct ydict_t
{
    uint64_t count;
    ydict_list *list;
    ydict_bucket *bucket;
    uint32_t bucket_size;
};

struct ydict_list
{
    ydict_node *top;
    ydict_node *cur;
    uint64_t last_index;
};

struct ydict_bucket
{
    ydict_node *first;
};

struct ydict_node
{
    ydict_node *list_nxt;
    ydict_node *list_pre;
    ydict_node *key_nxt;
    uint64_t index;
    void *value;
    size_t key_size;
    char key_data[];
};

ydict_t *ydict_init();
int ydict_put(ydict_t*, const char*, size_t, void*);
void *ydict_get(ydict_t*, const char*, size_t);
void *ydict_get_index(ydict_t*, uint64_t);
uint64_t size(ydict_t*);
void clear(ydict_t*);
int remove_index(ydict_t*, uint64_t);
int remove_key(ydict_t*, const char*, size_t);
#endif