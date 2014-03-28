#ifndef _GB2U_H_
#define _GB2U_H_

int gb2unicode(char *gb, char *unicode, int *unicode_len);
int unicode2gb(char *unicode, char *gb, int unicode_len);
void InitUnicodeIndex();
int HalfSearchUnicode(unsigned short int UnicodeValue, int LowIndex, int HighIndex);
int char_gb2unicode(char *gb,char *unicode);
int char_unicode2gb(char *unicode, char *gb);
int GetIndexOfUnicode(char *Unicode);
void SortUnicodeBubble();
void SortUnicodeQuick(int LowIndex, int HighIndex);
int Unicode_Lib_Init(char *filename);
int	ConvertUTF8toUCS2 (unsigned char* sourceStart, unsigned char* sourceEnd, unsigned char* target, int* iCount);
int Gb2312ToUtf8_2(const char * sIn,int iInLen,char * sOut,int iMaxOutLen);
int Utf8ToGb2312(const char * sIn,int iInLen,char * sOut,int iMaxOutLen);
int Gb2312ToUtf8(char * sIn,int iInLen,char * sOut,int iMaxOutLen);

int GbkToUtf8(const char * sIn, int iInLen,char * sOut, int iMaxOutLen);
int Utf8ToGbk(const char * sIn,int iInLen,char * sOut,int iMaxOutLen);
#endif

