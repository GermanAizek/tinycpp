#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int min3(int a, int b, int c) {
    int m = a;
    if (b < m) m = b;
    if (c < m) m = c;
    return m;
}

static int levenshtein_dist(const char *s1, int len1, const char *s2, int len2, int *dp) {
    for (int i = 0; i <= len1; i++) dp[i * (len2 + 1) + 0] = i;
    for (int j = 0; j <= len2; j++) dp[0 * (len2 + 1) + j] = j;

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            int del_cost = dp[(i - 1) * (len2 + 1) + j] + 1;
            int ins_cost = dp[i * (len2 + 1) + (j - 1)] + 1;
            int sub_cost = dp[(i - 1) * (len2 + 1) + (j - 1)] + cost;
            dp[i * (len2 + 1) + j] = min3(del_cost, ins_cost, sub_cost);
        }
    }
    return dp[len1 * (len2 + 1) + len2];
}

#define STR_LEN 800
#define NUM_PAIRS 120

int main(void) {
    char *s1 = (char *)malloc(STR_LEN + 1);
    char *s2 = (char *)malloc(STR_LEN + 1);
    int *dp = (int *)malloc(sizeof(int) * (STR_LEN + 1) * (STR_LEN + 1));
    if (!s1 || !s2 || !dp) return 1;

    unsigned int seed = 1234567;
    long long total_dist = 0;

    for (int p = 0; p < NUM_PAIRS; p++) {
        /* Generate pseudo-random DNA strings with variations */
        const char *alphabet = "ACGT";
        for (int i = 0; i < STR_LEN; i++) {
            seed = seed * 1664525 + 1013904223;
            s1[i] = alphabet[seed % 4];
            seed = seed * 1664525 + 1013904223;
            /* 80% same, 20% mutations */
            if ((seed % 100) < 80) {
                s2[i] = s1[i];
            } else {
                s2[i] = alphabet[(seed >> 4) % 4];
            }
        }
        s1[STR_LEN] = '\0';
        s2[STR_LEN] = '\0';

        total_dist += levenshtein_dist(s1, STR_LEN, s2, STR_LEN, dp);
    }

    printf("Levenshtein: Pairs=%d Len=%d TotalDist=%lld\n", NUM_PAIRS, STR_LEN, total_dist);

    free(s1);
    free(s2);
    free(dp);
    return 0;
}
