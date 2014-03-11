#include <iostream>
#include <string.h>
#include "md5.h"

int main()
{
    char pwd[20] = {0};
    CMD5 *pmd5 = new CMD5();
    string md5str;

    strncpy(pwd, "123456789", sizeof(pwd));
    pmd5->GenerateMD5((unsigned char *)pwd, strlen(pwd));
    md5str = pmd5->ToString();
    std::cout << md5str << std::endl;
    return 0;
}
