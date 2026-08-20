#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 65536
#define NUM_OPS 300000

namespace Storage {

class Entry {
public:
    int key;
    int value;
    Entry *next;
};

class HashMap {
public:
    Entry *buckets[TABLE_SIZE];
};

Entry *create_entry(int k, int v, Entry *n) {
    Entry *e = new Entry;
    e->key = k;
    e->value = v;
    e->next = n;
    return e;
}

unsigned int hash_code(int key) {
    unsigned int x = static_cast<unsigned int>(key);
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x % TABLE_SIZE;
}

void hash_insert(HashMap &map, int key, int value) {
    unsigned int idx = hash_code(key);
    map.buckets[idx] = create_entry(key, value, map.buckets[idx]);
}

int hash_lookup(const HashMap &map, int key) {
    unsigned int idx = hash_code(key);
    Entry *curr = map.buckets[idx];
    while (curr != nullptr) {
        if (curr->key == key)
            return curr->value;
        curr = curr->next;
    }
    return -1;
}

void hash_cleanup(HashMap &map) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry *curr = map.buckets[i];
        while (curr != nullptr) {
            Entry *next = curr->next;
            delete curr;
            curr = next;
        }
        map.buckets[i] = nullptr;
    }
}

} // namespace Storage

static Storage::HashMap map;

int main() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        map.buckets[i] = nullptr;
    }

    for (int i = 0; i < NUM_OPS; i++) {
        Storage::hash_insert(map, i * 7 + 3, i ^ 0x55aa);
    }

    long long sum = 0;
    int found_count = 0;
    for (int i = 0; i < NUM_OPS; i++) {
        int v = Storage::hash_lookup(map, i * 7 + 3);
        if (v != -1) {
            found_count++;
            sum += (v % 1000);
        }
    }

    Storage::hash_cleanup(map);

    printf("CppHashTable: Ops=%d Found=%d Sum=%lld\n", NUM_OPS, found_count, sum);
    return 0;
}
