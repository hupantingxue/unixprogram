/**********************************************************
 * Author: hupantingxue
 * Date:   2014-03-28
 * Hat-trie lib url: https://github.com/dcjones/hat-trie 
 *********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hat-trie.h"
#include <time.h>

int main(int argc, char *argv[]) {
    hattrie_t  *pht = NULL;
    value_t *vt = NULL;
    char *fname = NULL;
    char line[100] = {0};
    int iRet = 0;
    int len = 0;

    /* read dirty words in file*/
    if (1 >= argc) {
        printf("Usage: trie  triefile\n");
        return -1;
    }

    fname = argv[1];
    FILE *fp = fopen(fname, "r+");
    if (NULL == fp) {
        printf("Read file[%s] fail.\n", fname);
        return -2;
    }

    pht = hattrie_create();
    if (NULL == pht) {
        printf("trie create fail.\n");
    }

    while(!feof(fp)) {
        memset(line, 0, sizeof(line));
        if (NULL != fgets(line, sizeof(line), fp)) {
            len = strlen(line);
            if ('\n' == line[len-1])
                line[len-1] = '\0';
            printf("[%s] len[%d] insert into dirty-trie\n", line, len);
            vt = hattrie_get(pht, line, len);
        }
    }

    clock_t t0, t;
    t0 = clock();
    size_t r;
    hattrie_iter_t *it = NULL;
    const size_t repetitions = 1;
    for (r = 0; r < repetitions; ++r) {
        it = hattrie_iter_begin(pht, true);
        while (!hattrie_iter_finished(it)) {
            // printf("spam key[%d]\n", it);
            hattrie_iter_next(it);
        }
    }
    t = clock();
    printf("finished. (%0.2f seconds)\n", (double) (t - t0) / (double) CLOCKS_PER_SEC);

    char *pig = "中共";
    len = strlen(pig) + 1;
    if (NULL == (hattrie_tryget(pht, pig, len))) {
        printf("[%s] len[%d] not in.\n", pig, len);
    }
    else {
        printf("[%s] len[%d] dose in.\n", pig, len);
    }

    fclose(fp);
    hattrie_free(pht);
    return 0;
}
