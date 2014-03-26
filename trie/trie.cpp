#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hat-trie.h"

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

    char *pig = "中共传人";
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
