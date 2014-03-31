#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "dirtycheck.h"

int main(int argc, char *argv[]) {
    if (3 > argc) {
        std::cout << "Usage:load dirty words;\nargs:\n    dirty_shmkey dirty_file_name" << std::endl;
        return -1;
    }

    int key = atoi(argv[1]);
    char *fn = argv[2];
    int iRet = 0;
    DirtyCheck dtchk;
    unsigned char words[100] = {0};

    iRet = dtchk.Chn_Dirty_Init(key, 600, fn);
    if (0 != iRet) {
        std::cout << dtchk.sErrMsg << std::endl;
        return -2;
    }
    else {
        std::cout << "Dirty words init success. shmid: " << dtchk.iShmID << std::endl;
    }

    strncpy((char *)words, "法轮功", sizeof(words));
    iRet = dtchk.Chn_Dirty_Check(words);
    if (1 == iRet) {
        std::cout << words << " has dirty words." << std::endl;
    } else if (0 == iRet) {
        std::cout << words << " is clean." << std::endl;
    } else {
        std::cout << words << " check failed " << dtchk.sErrMsg << std::endl;
    }
 
    strncpy((char *)words, "我是中国人", sizeof(words));
    iRet = dtchk.Chn_Dirty_Check(words);
    if (1 == iRet) {
        std::cout << words << " has dirty words." << std::endl;
    } else if (0 == iRet) {
        std::cout << words << " is clean." << std::endl;
    } else {
        std::cout << words << " check failed " << dtchk.sErrMsg << std::endl;
    }

    std::cout << "Greatwall." << std::endl;
    return 0;
}
