#ifndef _DIRTY_CHECK_H_
#define _DIRTY_CHECK_H_

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <iconv.h>
#include <string>

#ifdef  __cplusplus
extern "C" {
#endif
#include "gb2unicode.h"
#ifdef  __cplusplus
}
#endif

#define  C_MAX_WORD_LEN   256
#define  C_MAX_TABLE_LEN  256*256
#define  C_MAX_WORD_NUM   5000

using namespace std;

#define SHOW_CHINESE  (1)
#define SHOW_ENGLISH  (2)


#define GBK_CODE  (1)
#define UTF8_CODE  (2)
#define MAX_GBK_BUFF_LEN (1024*10)

/*
#define  IS_DOUBLE_CHAR(ch) (ch>=0xA0&&ch<0xFF)  
#define  IS_ENGLISH_CHAR(ch) (ch>='a'&&ch<='z'||ch>='A'&&ch<='Z') 
#define  IS_CHINESE_CHAR(lo,hi) (lo>=0xB0&&lo<=0xF7&&hi>=0xA1&&hi<=0xFE) 
#define  IS_ENGLISH_DOUBLE(lo,hi) (lo==0xA3&&(hi<=0xDA&&hi>=0xC1||hi<=0xFA&&hi>=0xE1))
#define  CONVERT_DOUBLE_TO_SINGLE(ch,lo,hi) ch=(hi>=0xC1&&hi<=0xDA)?(hi-0x80):(hi-0xA0); 
#define  CONVERT_CAPITAL_CHAR(ch,lo) ch=(lo>='a'&&lo<='z')?lo-0x20:lo;
#define  EQUAL_ENGLISH_CHAR(lo,hi) (lo==hi||lo==(hi-0x20))
 */
 //#define  IS_DOUBLE_CHAR(ch) 		( ch>=0xA0&&ch<0xFF)
#define  IS_DOUBLE_CHAR(ch) 		( ch>=0x80&&ch<0xFF) 
#define  IS_ENGLISH_CHAR(ch)		( (ch>='a'&&ch<='z') || (ch>='A'&&ch<='Z') )
//#define  IS_ENGLISH_CHAR(ch)		(ch>=0x20 && ch<=0x7E) //kelvinhuang modify for suppourt  ¿É¼ûµÄascii  ' '
#define  IS_CHINESE_CHAR(lo,hi)		( (lo>0x80 && lo<0xff) && ((hi>0x3f && hi<0x7f)||(hi>0x7f && hi<0xff)))
#define  IS_CHINESE_PUNC(lo,hi)		( (lo>0xa0 && lo<0xb0) && (hi>0x3f && hi<0xff))
#define  IS_ENGLISH_DOUBLE(lo,hi)	( (lo==0xA3) &&( (hi<=0xDA && hi>=0xC1) || (hi<=0xFA && hi>=0xE1) ))
#define  IS_DIGIT_DOUBLE(lo,hi) 	( (lo==0xA3) &&(hi>=0xB0 && hi<=0xB9) )
#define  IS_DIGIT(ch)  (isdigit(ch))
#define  CONVERT_DOUBLE_TO_SINGLE(ch,lo,hi)	ch=(hi>=0xC1&&hi<=0xDA)?(hi-0x80):(hi-0xA0);
#define  CONVERT_DOUBLE_DIGIT_TO_SINGLE(ch,lo,hi)  ch=hi-0x80;
#define  CONVERT_CAPITAL_CHAR(ch,lo)			 ch=(lo>='a'&&lo<='z')?lo-0x20:lo;

#define  EQUAL_ENGLISH_CHAR(lo,hi)  ( lo==hi || lo==(hi-0x20))

#define IS_SHOW_ENG(ch)  (ch>=0x20 && ch<=0x7E)

#define CHAR_KEYWORD_DELI '|'

//#define CAL_INDEX_OFFSET(offset,hi,lo)offset=(hi-0xA0)*128+lo;
//#define CAL_INDEX_OFFSET(offset,hi,lo)offset=(hi-0x80)*128+lo;
#define CAL_INDEX_OFFSET(offset,hi,lo)offset=(hi-0x80)*256+lo;//see C_MAX_TABLE_LEN

typedef struct _DIRTY_CHN_RECORD
{
     unsigned char sDirtyStr[C_MAX_WORD_LEN+1]; 
     unsigned char sKeyWord[3];     
     short         iKeyOffset;      
     short         iNextKey;        
}DIRTY_CHN_RECORD;

typedef struct _DIRTY_ENG_RECORD 
{
     unsigned char sDirtyStr[C_MAX_WORD_LEN+1];
}DIRTY_ENG_RECORD;

typedef struct _DIRTY_EN_STRUCT 
{
     DIRTY_ENG_RECORD *pstEnglishDirtyList;
     int  iEngWordCount;	
} DIRTY_EN_STRUCT;

typedef struct _DIRTY_CHN_STRUCT 
{
     short   iDirtyIndexTable[C_MAX_TABLE_LEN];
     DIRTY_CHN_RECORD *pstChineseDirtyList;
     int  iChnWordCount;	 
} DIRTY_CHN_STRUCT;

class DirtyCheck
{
public:
	DirtyCheck();
	int Eng_Dirty_Init(key_t iKey,int iMaxNum,char *sEngFileName);
	int Chn_Dirty_Init(key_t iKey,int iMaxNum,char *sChnFileName);
	int Chn_Dirty_Check(unsigned char *sCheckStr);
	int Eng_Dirty_Check(unsigned char *sCheckStr);
	int Eng_Dirty_Destroy();
	int Chn_Dirty_Destroy();

       int Mix_Dirty_Check(unsigned char *sCheckStr);
       int Mix_Dirty_Check_Chn(unsigned char *sCheckStr);
	int Dirty_List_Word(int CharSet);
private:
	int En_Load_From_File (FILE * pFile,unsigned char * sDirtyWord,unsigned char * sKeyWord);
	int Chn_Load_From_File (FILE * pFile,unsigned char * sDirtyWord,unsigned char * sKeyWord);    
	int Init_Chn_Table (DIRTY_CHN_STRUCT *pstChnIndexTab,unsigned char * sDirtyWord,unsigned char *sKeyWord,int iCount);
	int Chn_Dirty_Check_Word(int iKeyOff,unsigned char  *sCheckStr,unsigned char  *sReservBuff,unsigned char  *sDirtyStr);
	int Eng_Dirty_Check_Word(unsigned char  *sCheckStr,unsigned char  *sDirtyStr);
    
private:
	int iMaxDirtyWordNum; 
	int iShmID;  
	int iEngShmID;    
	int iGbInitiated;
	int iEngInitiated;
	int iMaxDirtyEnglishWordNum; 	
	
public:
	char sErrMsg[200];
	string strDirtyWord;
};

class Dirty
{
public:
	Dirty();
    
       string GetErrMsg();
       string GetDirtyWord();
       int CheckDirtyWord(int iCheckType, string strCheckStr);
       int List_Ditry_Word(int iCharSet);
       void SetEnShmKey(key_t iKey);
       void SetChnShmKey(key_t iKey);
       void SetEnMaxNum(int iNum);
       void SetChnMaxNum(int iNum);
       void SetEnFileName(string strFileName);
       void SetChnFileName(string strFileName);
       int InitShm();  
private:
	   int UTF8ToGBK(string strIn,char *sOut,int iMaxOutLen);
       void FreeMemory();
	 
        key_t m_iEnKey;
        int m_iEnMaxNum;
        string m_strEnFileName;
        key_t m_iChnKey;
        int m_iChnMaxNum;
        string m_strChnFileName;
        
        DirtyCheck m_CDirtyCheck;
        string m_strErrMsg;
		iconv_t m_cd;

		unsigned char * m_pGbkBuff;
		size_t m_iGbkBuffLen;
		unsigned char m_szGbkBuff[1024*5];	 
public:

};
#endif
