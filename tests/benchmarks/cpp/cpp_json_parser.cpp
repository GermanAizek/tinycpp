#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { JSON_NULL, JSON_BOOL, JSON_NUMBER, JSON_STRING, JSON_ARRAY, JSON_OBJECT } JsonType;

typedef struct JsonValue {
    JsonType type;
    union {
        int bool_val;
        double num_val;
        char *str_val;
        struct {
            struct JsonValue **elements;
            int count;
        } array;
        struct {
            char **keys;
            struct JsonValue **values;
            int count;
        } object;
    } u;
} JsonValue;

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} Arena;

static Arena* arena_create(size_t capacity) {
    Arena *a = (Arena *)malloc(sizeof(Arena));
    a->data = (char *)malloc(capacity);
    a->size = 0;
    a->capacity = capacity;
    return a;
}

static void* arena_alloc(Arena *a, size_t size) {
    size_t aligned = (size + 7) & ~7;
    if (a->size + aligned > a->capacity) return NULL;
    void *ptr = a->data + a->size;
    a->size += aligned;
    return ptr;
}

static void arena_reset(Arena *a) {
    a->size = 0;
}

static const char* skip_whitespace(const char *p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

static JsonValue* parse_value(const char **p, Arena *a);

static JsonValue* parse_array(const char **p, Arena *a) {
    (*p)++; // Skip '['
    JsonValue *v = (JsonValue *)arena_alloc(a, sizeof(JsonValue));
    v->type = JSON_ARRAY;
    v->u.array.elements = (JsonValue **)arena_alloc(a, sizeof(JsonValue *) * 64);
    v->u.array.count = 0;

    *p = skip_whitespace(*p);
    if (**p == ']') { (*p)++; return v; }

    while (**p) {
        JsonValue *elem = parse_value(p, a);
        v->u.array.elements[v->u.array.count++] = elem;
        *p = skip_whitespace(*p);
        if (**p == ',') {
            (*p)++;
        } else if (**p == ']') {
            (*p)++;
            break;
        }
    }
    return v;
}

static JsonValue* parse_object(const char **p, Arena *a) {
    (*p)++; // Skip '{'
    JsonValue *v = (JsonValue *)arena_alloc(a, sizeof(JsonValue));
    v->type = JSON_OBJECT;
    v->u.object.keys = (char **)arena_alloc(a, sizeof(char *) * 32);
    v->u.object.values = (JsonValue **)arena_alloc(a, sizeof(JsonValue *) * 32);
    v->u.object.count = 0;

    *p = skip_whitespace(*p);
    if (**p == '}') { (*p)++; return v; }

    while (**p) {
        *p = skip_whitespace(*p);
        if (**p != '"') break;
        (*p)++;
        const char *k_start = *p;
        while (**p && **p != '"') (*p)++;
        size_t k_len = *p - k_start;
        char *key = (char *)arena_alloc(a, k_len + 1);
        memcpy(key, k_start, k_len);
        key[k_len] = '\0';
        if (**p == '"') (*p)++;

        *p = skip_whitespace(*p);
        if (**p == ':') (*p)++;

        JsonValue *val = parse_value(p, a);
        v->u.object.keys[v->u.object.count] = key;
        v->u.object.values[v->u.object.count] = val;
        v->u.object.count++;

        *p = skip_whitespace(*p);
        if (**p == ',') {
            (*p)++;
        } else if (**p == '}') {
            (*p)++;
            break;
        }
    }
    return v;
}

static JsonValue* parse_value(const char **p, Arena *a) {
    *p = skip_whitespace(*p);
    JsonValue *v = (JsonValue *)arena_alloc(a, sizeof(JsonValue));
    if (**p == '{') {
        return parse_object(p, a);
    } else if (**p == '[') {
        return parse_array(p, a);
    } else if (**p == '"') {
        (*p)++;
        const char *s_start = *p;
        while (**p && **p != '"') (*p)++;
        size_t s_len = *p - s_start;
        char *str = (char *)arena_alloc(a, s_len + 1);
        memcpy(str, s_start, s_len);
        str[s_len] = '\0';
        if (**p == '"') (*p)++;
        v->type = JSON_STRING;
        v->u.str_val = str;
        return v;
    } else if (**p == 't' || **p == 'f') {
        v->type = JSON_BOOL;
        v->u.bool_val = (**p == 't');
        while (**p >= 'a' && **p <= 'z') (*p)++;
        return v;
    } else if (**p == 'n') {
        v->type = JSON_NULL;
        while (**p >= 'a' && **p <= 'z') (*p)++;
        return v;
    } else {
        v->type = JSON_NUMBER;
        char *endptr;
        v->u.num_val = strtod(*p, &endptr);
        *p = endptr;
        return v;
    }
}

static double checksum_json(JsonValue *v) {
    if (!v) return 0.0;
    double sum = 0.0;
    switch (v->type) {
        case JSON_NUMBER: return v->u.num_val;
        case JSON_BOOL: return v->u.bool_val ? 1.0 : 0.0;
        case JSON_STRING: return (double)strlen(v->u.str_val);
        case JSON_ARRAY:
            for (int i = 0; i < v->u.array.count; i++)
                sum += checksum_json(v->u.array.elements[i]);
            return sum;
        case JSON_OBJECT:
            for (int i = 0; i < v->u.object.count; i++)
                sum += (double)strlen(v->u.object.keys[i]) + checksum_json(v->u.object.values[i]);
            return sum;
        default: return 0.0;
    }
}

static const char *SAMPLE_JSON = 
"{"
"  \"id\": 1024,"
"  \"name\": \"Benchmark Task\","
"  \"active\": true,"
"  \"score\": 98.75,"
"  \"tags\": [\"compiler\", \"fast\", \"tcc\", \"test\"],"
"  \"payload\": {"
"    \"nested_id\": 2048,"
"    \"values\": [10.5, 20.3, 30.1, 40.8, 50.2],"
"    \"meta\": { \"version\": \"1.0.0\", \"valid\": true, \"null_field\": null }"
"  }"
"}";

int main(void) {
    Arena *a = arena_create(1024 * 1024);
    double total_sum = 0.0;

    int iterations = 100000;
    for (int i = 0; i < iterations; i++) {
        const char *p = SAMPLE_JSON;
        JsonValue *root = parse_value(&p, a);
        total_sum += checksum_json(root);
        arena_reset(a);
    }

    printf("JSONParser: Iters=%d Checksum=%.1f\n", iterations, total_sum);

    free(a->data);
    free(a);
    return 0;
}
