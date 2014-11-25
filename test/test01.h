#ifndef __TEST_01_H
#define __TEST_01_H

#include <stdio.h>

typedef unsigned long  ULong;
typedef unsigned int   UInt;
typedef unsigned short UShort;
typedef unsigned char  UChar;

#pragma pack(1)

typedef struct {
  ULong   ulUid; 
  ULong   iConnIP;
  UShort  iLPort;
  UShort  iRPort;
#define jpushCmd iRPort
  ULong   ulTime;
  UChar   cFlag;
  UInt    uSid;
  char    res[4];
}StatusElement;

#pragma pack()
#endif
