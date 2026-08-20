#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 65536
#define NUM_OPS 300000

typedef struct Entry {
    int key;
    int value;
    struct Entry *next;
} Entry;

typedef struct HashTable {
    Entry *buckets[TABLE_SIZE];
} HashTable;

static HashTable table;

static unsigned int hash_fn(int key) {
    unsigned int x = (unsigned int)key;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x % TABLE_SIZE;
}

static void insert(HashTable *ht, int key, int value) {
    unsigned int idx = hash_fn(key);
    Entry *entry = (Entry *)malloc(sizeof(Entry));
    entry->key = key;
    entry->value = value;
    entry->next = ht->buckets[idx];
    ht->buckets[idx] = entry;
}

static int lookup(HashTable *ht, int key) {
    unsigned int idx = hash_fn(key);
    Entry *curr = ht->buckets[idx];
    while (curr) {
        if (curr->key == key)
            return curr->value;
        curr = curr->next;
    }
    return -1;
}

int main(void) {
    memset(&table, 0, sizeof(table));

    for (int i = 0; i < NUM_OPS; i++) {
        insert(&table, i * 7 + 3, i ^ 0x55aa);
    }

    long long sum = 0;
    int found_count = 0;
    for (int i = 0; i < NUM_OPS; i++) {
        int v = lookup(&table, i * 7 + 3);
        if (v != -1) {
            found_count++;
            sum += (v % 1000);
        }
    }

    /* Free table */
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry *curr = table.buckets[i];
        while (curr) {
            Entry *next = curr->next;
            free(curr);
            curr = next;
        }
    }

    printf("HashTable: Ops=%d Found=%d Sum=%lld\n", NUM_OPS, found_count, sum);
    return 0;
}
