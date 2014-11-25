#include "test01.h"

int main() {
    printf("ULong size[%d] Struct size[%d]\n", sizeof(ULong), sizeof(StatusElement));
    StatusElement se;
    se.iRPort = 17;
    printf("%d\n", se.jpushCmd);
    return 0;
}
