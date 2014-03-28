#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "dirtycheck.h"

DirtyCheck::DirtyCheck()
{
	iMaxDirtyWordNum     = 500; 
	iMaxDirtyEnglishWordNum     = 500; 	
	iShmID               =-1;  
	iEngShmID            =-1;    
	iGbInitiated 	 = 0;
	iEngInitiated 	 = 0;
}

int DirtyCheck::Chn_Dirty_Init(key_t iKey,int   iMaxNum,char  *sChnFileName)
{
	 int  iShmSize,iCount=0,iRet;
	 DIRTY_CHN_STRUCT *pstChnIndexTab;
	 unsigned char         *pShm,sDirtyWord[C_MAX_WORD_LEN+1],sKeyWord[3]; 
	 FILE                  *pFile;
	 

	 if (iGbInitiated)
	 	return 0;

	 	
	 iMaxDirtyWordNum=(iMaxNum>iMaxDirtyWordNum)?iMaxNum:iMaxDirtyWordNum;
	 if (iMaxDirtyWordNum>C_MAX_WORD_NUM){
	 	sprintf(sErrMsg,"The maxnumber is larger than %d",C_MAX_WORD_NUM);
	 	return -1;
	 }
	 iShmSize=sizeof(DIRTY_CHN_STRUCT)+sizeof(DIRTY_CHN_RECORD)*iMaxDirtyWordNum;
	 if ((iShmID = shmget(iKey, iShmSize, IPC_CREAT | IPC_EXCL | 0666)) < 0) {
		if ((iShmID = shmget(iKey, iShmSize, 0666))<0)                   //有可能是已经存在同样的key_shm,则试图连接
		{
			sprintf(sErrMsg,"Fail to shmget. ShmKey is %d", iKey);
			
			return -1;
		}
		#ifdef DIRTYDEBUG 
		cout<<"iShmID="<<iShmID<<endl;
		#endif
		return 0;
	 }

	//try to access shm 
	if ((pShm = (unsigned char*)shmat(iShmID, NULL ,0)) == (unsigned char *) -1) 
	{
		sprintf(sErrMsg,"Fail to shmat. ShmID is %d", iShmID);
		//if access failed, try to remove the shareMemory
		shmctl(iShmID, IPC_RMID, NULL);
		return -2;
	}
	pstChnIndexTab=(DIRTY_CHN_STRUCT *)pShm;
	//pstChnIndexTab->pstChineseDirtyList=(DIRTY_CHN_RECORD *)(pShm+sizeof(DIRTY_CHN_STRUCT));

	for (iCount=0;iCount<C_MAX_TABLE_LEN;iCount++)
	       pstChnIndexTab->iDirtyIndexTable[iCount]=-1;
	iCount=0;        
	pFile =fopen(sChnFileName,"r");
	if (pFile==NULL) {
		sprintf(sErrMsg,"Fail to open file:%s",sChnFileName);
		shmdt(pShm);
		return -3; 
	} 

	/* -------- 从文件里读脏话词语 -------------------------*/
	while ((iRet=Chn_Load_From_File(pFile,sDirtyWord,sKeyWord))==0)
	{
		#ifdef DIRTYDEBUG
		    printf("Dirty string:%s\n",sDirtyWord);
		    printf("KeyWord string:%s\n",sKeyWord);
		#endif    
		if (sDirtyWord[0]=='\0'||sKeyWord[0]=='\0')
		        break;  
		if (Init_Chn_Table(pstChnIndexTab,sDirtyWord,sKeyWord,iCount++)<0){
			sprintf(sErrMsg,"Fail to init dirty word table!");
			fclose(pFile);
			shmdt(pShm);
			return -4;
		}
        }
        if (iRet<0){
        	shmdt(pShm);
        	return -5;
        } 

	pstChnIndexTab->iChnWordCount = iCount;

        shmdt(pShm);
	fclose(pFile);	

	iGbInitiated=1;

//	iRealChnWorkCount = iCount;
	return 0;
}
/**********************************************************************************************************************
Funciton 把中文脏话文件LOAD 到内存
注意：1。必须都是中文，如果有英文请转成中文格式
		   2。不包括任何标点
		
	   
如
他妈的|他
肚子饿|饿
他奶奶的|奶
江泽民|泽
６４事件|件
９１８|８
***********************************************************************************************************************/	

int DirtyCheck::Chn_Load_From_File (FILE * pFile,unsigned char * sDirtyWord,unsigned char * sKeyWord)
{
        unsigned char cReadChar;
        char          cSignedChar;
	int           iIsKeyWord=0;
	int           iCount=0;
	
	sDirtyWord[0]='\0';
	sKeyWord[0]='\0';
	while ((cSignedChar=fgetc(pFile))!=EOF)
	{
		cReadChar=cSignedChar;
		if (IS_DOUBLE_CHAR(cReadChar)||IS_ENGLISH_CHAR(cReadChar)){
			if (iIsKeyWord)
			   sKeyWord[iCount++]=cReadChar;
			else sDirtyWord[iCount++]=cReadChar;   
		} 
		else if (cReadChar==CHAR_KEYWORD_DELI){
			sDirtyWord[iCount]='\0';
			iCount=0;
			iIsKeyWord=1;
		}
		else if (cReadChar=='\n'){
			if (iIsKeyWord)
			   sKeyWord[iCount]='\0';
			else sDirtyWord[iCount]='\0';
			iCount=0;
		        break;	   
		}
		else if (cReadChar=='\r'||cReadChar==' ')
		        continue;
		else {
			sprintf(sErrMsg,"Invaild char in dirty-word file");
			return -1;
		}
	}
	if (cSignedChar==EOF){
		if (iIsKeyWord)
		   sKeyWord[iCount]='\0';
		else sDirtyWord[iCount]='\0';  
		return 1; 
	}
	return 0;
}

int DirtyCheck::Init_Chn_Table (DIRTY_CHN_STRUCT *pstChnIndexTab,unsigned char * sDirtyWord,unsigned char *sKeyWord,int iCount)
{
	DIRTY_CHN_RECORD    *pstDirtyRecord;
	unsigned char            *sSubstr;
	int                      iIndex,iOffset;
	if (iCount ==iMaxDirtyWordNum)
	{
	        sprintf(sErrMsg,"The number of the words is too large");	
	        return -1;
	}
//	pstDirtyRecord=pstChnIndexTab->pstChineseDirtyList;   
	pstDirtyRecord=(DIRTY_CHN_RECORD *)((char *)pstChnIndexTab+sizeof(DIRTY_CHN_STRUCT));
	strcpy((char*)pstDirtyRecord[iCount].sDirtyStr, (const char*)sDirtyWord); 
	strcpy((char*)pstDirtyRecord[iCount].sKeyWord, (const char*)sKeyWord);
	sSubstr=(unsigned char*)strstr((char*)sDirtyWord, (const char*)sKeyWord);
	if (sSubstr==NULL){
	        sprintf(sErrMsg,"Keyword(%s) in this file error",sKeyWord);
	        return -2;	
	}
	pstDirtyRecord[iCount].iKeyOffset=(sSubstr-sDirtyWord); 
	pstDirtyRecord[iCount].iNextKey=-1;
	
	#ifdef DIRTYDEBUG 
	    printf("Init_Chn_Table:KeyOffset:%d\n",pstDirtyRecord[iCount].iKeyOffset); 
	    printf("Init_Chn_Table:iCount:%d\n",iCount);
	#endif    
	 
	CAL_INDEX_OFFSET(iIndex,sKeyWord[0],sKeyWord[1])
	iOffset=pstChnIndexTab->iDirtyIndexTable[iIndex];
	if (iOffset==-1){
		pstChnIndexTab->iDirtyIndexTable[iIndex]=iCount;
	}
	else { 
	        while (iOffset!=-1)
	        {
	        	iIndex=iOffset;
	        	iOffset=pstDirtyRecord[iOffset].iNextKey;
	        }
	        pstDirtyRecord[iIndex].iNextKey=iCount;
	}
	return 0; 
}

int DirtyCheck::Chn_Dirty_Check_Word(int iKeyOff,unsigned char  *sCheckStr,unsigned char  *sReservBuff,unsigned char  *sDirtyStr)
{
	int            iCount=0,iCheckCount=0;
	unsigned char  cHiChar,cLoChar; 
	
	#ifdef DIRTYDEBUG
	     printf("Check_Word:Checkstr:%s\n",sCheckStr);
	     printf("Check_Word:DirtyStr:%s\n",sDirtyStr);
	     printf("KeyOffset is:%d\n",iKeyOff);
	     sReservBuff[iKeyOff]='\0';
	     printf("ReservBuff is :%s\n",sReservBuff); 
	#endif     
	while ((iCount)<iKeyOff)
	{
		if (sReservBuff[iCount%C_MAX_WORD_LEN]!=sDirtyStr[iCount])
		    return 1;
		iCount++;    
	}
	#ifdef DIRTYDEBUG
	       printf("Dirty String :%s\n",sDirtyStr+iCount); 
	#endif 
	while (sDirtyStr[iCount]!='\0')
	{
		if (sCheckStr[iCheckCount]=='\0')   return 1; 
		cLoChar=sCheckStr[iCheckCount++];   
	        cHiChar=sCheckStr[iCheckCount]; 
		if (!IS_CHINESE_CHAR(cLoChar,cHiChar)) /* 是否全角非汉字? */
			continue;
//	        if ( IS_CHINESE_PUNC(cLoChar,cHiChar)) // 中文标点符号
	        if ( IS_CHINESE_PUNC(cLoChar,cHiChar) &&
			(!IS_DIGIT_DOUBLE(cLoChar,cHiChar)) && 
			(!IS_ENGLISH_DOUBLE(cLoChar,cHiChar))) 
	        {// 中文标点符号必须把中文英文和数字都去掉
			iCheckCount++;
	        	continue;
	        }
		iCheckCount++;

		#ifdef DIRTYDEBUG
		       printf("the word is :%c%c\n",cLoChar,cHiChar);
		       printf ("check word is:%c%c\n",sDirtyStr[iCount],sDirtyStr[iCount+1]);
		#endif
	
		if (cLoChar!=sDirtyStr[iCount++])
	           return 1;
		if (cHiChar!=sDirtyStr[iCount++])
		   return 1; 		
	}
	#ifdef DIRTYDEBUG 
	       printf ("Complete !\n");
	#endif  
	return 0;
}
		    
/**********************************************************************************************************************
Funciton    检查是否有中文脏字眼..
input       sCheckStr: 	//待检查的字符串 
output      sErrMsg	//错误信息	    
return:     0: 		//此字符串不带有‘脏字眼’ 		    
	    1: 		//此字符串带有‘脏字眼’ 		    
	    <0:         //Error
***********************************************************************************************************************/	
int DirtyCheck::Chn_Dirty_Check(unsigned char *sCheckStr)
{	
	unsigned char            sReservBuff[C_MAX_WORD_LEN],cLoChar,cHiChar;
	int                      iIsDirty=0,iCount=0,iBufCount=0,iIndex,iOffset;
	int                      iKeyOff;
	DIRTY_CHN_RECORD    *pstDirtyRecord;
	DIRTY_CHN_STRUCT    *pstChnIndexTab;
	unsigned char            *pShm;

	if (iShmID==-1){
		sprintf(sErrMsg,"The ShareMemory hasn't beend Initiate");
		return -1;	
	}
	if (sCheckStr==NULL){
		sprintf(sErrMsg,"The checking string is null");
		return -2;	
	}
	if ((pShm = (unsigned char*)shmat(iShmID, NULL ,0)) == (unsigned char *) -1) 
	{
		sprintf(sErrMsg,"Fail to shmat. Shmid is %d", iShmID);
		return -3;
	}

	pstChnIndexTab=(DIRTY_CHN_STRUCT *)pShm;
 pstDirtyRecord=(DIRTY_CHN_RECORD *)(pShm+sizeof(DIRTY_CHN_STRUCT));
//pstDirtyRecord=pstChnIndexTab->pstChineseDirtyList;
	/*   开始检查 字符串是否含有 ‘脏字眼’ */

	while ((!iIsDirty)&&(cLoChar=sCheckStr[iCount++])!='\0')
	{
	        cHiChar=sCheckStr[iCount];
//		printf("0x%02X0x%02X %d\n",cLoChar,cHiChar,IS_CHINESE_PUNC(cLoChar,cHiChar));
		if (!IS_CHINESE_CHAR(cLoChar,cHiChar))
		{
			continue;
		}
		
	        if ( IS_CHINESE_PUNC(cLoChar,cHiChar) &&
			(!IS_DIGIT_DOUBLE(cLoChar,cHiChar)) && 
			(!IS_ENGLISH_DOUBLE(cLoChar,cHiChar))) 
	        {// 中文标点符号必须把中文英文和数字都去掉
	        	iCount++;
	        	continue;
	        }
		iCount++;

		sReservBuff[(iBufCount++)%C_MAX_WORD_LEN]=cLoChar;
		sReservBuff[(iBufCount++)%C_MAX_WORD_LEN]=cHiChar;

		CAL_INDEX_OFFSET(iIndex,cLoChar,cHiChar)

		iOffset=pstChnIndexTab->iDirtyIndexTable[iIndex]; /*该字是否关键字*/

#ifdef DIRTYDEBUG
		/*test*************8*/
#if 0
		cout << "iCount  is " << iCount << endl;
		cout << "iBufCount is " << iBufCount<< endl;
		printf(" cLoChar[%x] cHiChar[%x] \n",cLoChar,cHiChar);
		sReservBuff[iBufCount]=0;
		cout << "***********************sReservBuff  is " << sReservBuff << endl;
#endif
#endif
		while (iOffset!=-1)
		{
		    iKeyOff=pstDirtyRecord[iOffset].iKeyOffset;	

                  #ifdef DIRTYDEBUG
//			cout << "iKeyOff  is " << iKeyOff << endl;
//			cout << "(iBufCount-iKeyOff-2)%C_MAX_WORD_LEN  is " << (iBufCount-iKeyOff-2)%C_MAX_WORD_LEN << endl;
		    #endif
	           if (!Chn_Dirty_Check_Word(iKeyOff+2,
		                                   sCheckStr+iCount,
		                                   sReservBuff+(iBufCount-iKeyOff-2)%C_MAX_WORD_LEN,
		                                   pstDirtyRecord[iOffset].sDirtyStr)){
		            iIsDirty=1;
			     strDirtyWord = string( (char *)pstDirtyRecord[iOffset].sDirtyStr);
		            break;

		    }
		    iOffset=pstDirtyRecord[iOffset].iNextKey;  /* 下一个含有该关键字得短语*/	
		}       

	}


	shmdt(pShm);
	return iIsDirty;
}
/*******-**********************************************************-*****************************************************
Funciton    释放共享内存
input       
output      sErrMsg	//错误信息	    
return:     0: 		:success
            <0		:failed 
attention:  CGI一般不要调用
*******-****************************************************************************************************************/	
int DirtyCheck::Chn_Dirty_Destroy()
{
      if (iShmID==-1){
		sprintf(sErrMsg,"The ShareMemory hasn't beend Initiate");
		return -1;	

      }
	
      if (shmctl(iShmID, IPC_RMID, NULL)<0){
		sprintf(sErrMsg,"Faile to Remove The ShareMemory ,Sharem Key %d",iShmID);
		return -2;	
      }	
      iShmID=-1;
      return 0;		
}

/**************************************

            以下为英文脏话检查部分...


*************************************************************************************/
/* *****************************************************************-****************************
Funciton    初始化英文脏话共享内存区
input       iKey: 共享内存的标识
	    iMaxNum:	//最大的脏话短语数，不能超过5000 	
	    sEngFileName:	//英文脏话文件
output      sErrMsg		//错误信息	    
return:     0: success <0:Error
*******-*****************************************************************************************/	
int DirtyCheck::Eng_Dirty_Init (key_t iKey,int   iMaxNum,char  *sEngFileName)
{
	 int                    iShmSize,iCount=0,iRet;
	 DIRTY_ENG_RECORD *pstEngDirtyList;
	 DIRTY_EN_STRUCT *pstEnIndexTab;

	 unsigned char         *pShm,sDirtyWord[C_MAX_WORD_LEN+1],sKeyWord[3]; 
	 FILE                  *pFile;
	 
	 if (iEngInitiated)
	 	return 0;
	 	
	 iMaxDirtyEnglishWordNum=(iMaxNum>iMaxDirtyEnglishWordNum)?iMaxNum:iMaxDirtyEnglishWordNum;
	 if (iMaxDirtyEnglishWordNum>C_MAX_WORD_NUM){
	 	sprintf(sErrMsg,"The maxnumber is larger than %d",C_MAX_WORD_NUM);
	 	return -1;
	 }

	 iShmSize=sizeof(DIRTY_ENG_RECORD)*iMaxDirtyEnglishWordNum + sizeof(DIRTY_EN_STRUCT);
	 if ((iEngShmID = shmget(iKey, iShmSize, IPC_CREAT | IPC_EXCL | 0666)) < 0) {
		if ((iEngShmID = shmget(iKey, iShmSize, 0666))<0)                   //有可能是已经存在同样的key_shm,则试图连接
		{
			sprintf(sErrMsg,"Fail to shmget. ShmKey is %d", iKey);
			return -1;
		}
		#ifdef DIRTYDEBUG 
		cout<<"iEngShmID="<<iEngShmID<<endl;
		#endif
		return 0;
	 }
	//try to access shm 
	if ((pShm = (unsigned char*)shmat(iEngShmID, NULL ,0)) == (unsigned char *) -1) {
		sprintf(sErrMsg,"Fail to shmat. ShmID is %d", iEngShmID);
		//if access failed, try to remove the shareMemory
		shmctl(iEngShmID, IPC_RMID, NULL);
		return -2;
	}
	pstEnIndexTab = (DIRTY_EN_STRUCT *)pShm;
	pstEngDirtyList=(DIRTY_ENG_RECORD *)(pShm+sizeof(DIRTY_EN_STRUCT));
	    /*先初始化 dirtystring list */
	for (iCount=0;iCount<iMaxDirtyEnglishWordNum;iCount++)
	        pstEngDirtyList[iCount].sDirtyStr[0]='\0';
	
	
	pFile =fopen(sEngFileName,"r");
	if (pFile==NULL) {
		sprintf(sErrMsg,"Fail to open file:%s",sEngFileName);
		shmdt(pShm);
		return -3; 
	} 
	iCount=0;        
	/* -------- 从文件里读脏话词语 -------------------------*/
	while ((iRet=En_Load_From_File(pFile,sDirtyWord,sKeyWord))==0)
	{
		#ifdef DIRTYDEBUG 
		    printf("Dirty string:%s\n",sDirtyWord);
		#endif     
		strcpy((char*)pstEngDirtyList[iCount++].sDirtyStr, (const char*)sDirtyWord);
        }

        if (iRet<0){
        	shmdt(pShm);
        	return -5;
        } 

	pstEnIndexTab->iEngWordCount= iCount;	

       shmdt(pShm);
	fclose(pFile);	
	iEngInitiated=1;
//	iRealEnWorkCount = iCount;
	return 0;
}
/**********************************************************************************************************************
Funciton 把英文脏话文件LOAD 到内存
注意和中文脏话文件格式有点不一样，只支持英文和可见ascii码
如	"xinhuawang",
	"9.18",
	"17da",

***********************************************************************************************************************/	

int DirtyCheck::En_Load_From_File (FILE * pFile,unsigned char * sDirtyWord,unsigned char * sKeyWord)
{
        unsigned char cReadChar;
        char          cSignedChar;
	int           iIsKeyWord=0;
	int           iCount=0;
	
	sDirtyWord[0]='\0';
	sKeyWord[0]='\0';
	while ((cSignedChar=fgetc(pFile))!=EOF)
	{
		cReadChar=cSignedChar;
		if( IS_ENGLISH_CHAR(cReadChar)||IS_SHOW_ENG(cReadChar)){
			if (iIsKeyWord)
			   sKeyWord[iCount++]=cReadChar;
			else sDirtyWord[iCount++]=cReadChar;   
		} 
		else if (cReadChar==CHAR_KEYWORD_DELI){
			sDirtyWord[iCount]='\0';
			iCount=0;
			iIsKeyWord=1;
		}
		else if (cReadChar=='\n'){
			if (iIsKeyWord)
			   sKeyWord[iCount]='\0';
			else sDirtyWord[iCount]='\0';
			iCount=0;
		        break;	   
		}
		else if (cReadChar=='\r'||cReadChar==' ')
		        continue;
		else {
			sprintf(sErrMsg,"Invaild char in dirty-word file");
			return -1;
		}
	}
	if (cSignedChar==EOF){
		if (iIsKeyWord)
		   sKeyWord[iCount]='\0';
		else sDirtyWord[iCount]='\0';  
		return 1; 
	}
	return 0;
}

int DirtyCheck::Eng_Dirty_Check_Word(unsigned char  *sCheckStr,unsigned char  *sDirtyStr)
{
	int            iCount=0,iCheckCount=0;
	unsigned char  cHiChar,cLoChar,cCompChar; 
	
	#ifdef DIRTYDEBUG 
	       printf("check string:%s\n",sCheckStr);
	       printf("dirty string:%s\n",sDirtyStr);
	#endif 
	while ((cCompChar=sDirtyStr[iCount])!='\0')
	{
		cLoChar=sCheckStr[iCheckCount++];
		if (cLoChar=='\0')
		   return 1;
		if (IS_DOUBLE_CHAR(cLoChar)){ 
	           cHiChar=sCheckStr[iCheckCount++];
	           if (!IS_ENGLISH_DOUBLE(cLoChar,cHiChar))
	                continue;
	           else {
	           	CONVERT_DOUBLE_TO_SINGLE(cLoChar,cLoChar,cHiChar)
	           }    
	        } 
		else if (IS_ENGLISH_CHAR(cLoChar)){
			CONVERT_CAPITAL_CHAR(cLoChar,cLoChar)
		}else if(IS_SHOW_ENG(cLoChar))
             {//增加支持可见ascii码
	             CONVERT_CAPITAL_CHAR(cLoChar,cLoChar)
             }
		else continue;
		if (!EQUAL_ENGLISH_CHAR(cLoChar,cCompChar)){
	                #ifdef DIRTYDEBUG 
	                   printf("first char:%c,second char :%c\n",cLoChar,cCompChar);
	                #endif 		
		        return 1; 
		}
		iCount++;   
	}
	return 0;
}
/**********************************************************************************************************************
Funciton    检查是否有英文脏字眼..
input       sCheckStr: 	//待检查的字符串 
output      sErrMsg	//错误信息	    
return:     0: 		//此字符串不带有‘脏字眼’ 		    
	    1: 		//此字符串带有‘脏字眼’ 		    
	    <0:         //Error
***********************************************************************************************************************/	
int DirtyCheck::Eng_Dirty_Check(unsigned char *sCheckStr)
{	
	unsigned char            cLoChar,cHiChar;
	int                      iIsDirty=0,iCount=0,iIndex=0;
	DIRTY_ENG_RECORD    *pstDirtyRecord;
	DIRTY_EN_STRUCT    *pstEnIndexTab;	
	unsigned char            *pShm;
	
	if (iEngShmID==-1){
		sprintf(sErrMsg,"The ShareMemory hasn't beend Initiate");
		return -1;	
	}
	if (sCheckStr==NULL){
		sprintf(sErrMsg,"The checking string is null");
		return -2;	
	}
	if ((pShm = (unsigned char*)shmat(iEngShmID, NULL ,0)) == (unsigned char *) -1) 
	{
		sprintf(sErrMsg,"Fail to shmat. Shmid is %d", iEngShmID);
		return -3;
	}
	pstEnIndexTab = (DIRTY_EN_STRUCT *)pShm;
	pstDirtyRecord=(DIRTY_ENG_RECORD *)(pShm+sizeof(DIRTY_EN_STRUCT));


	/*   开始检查 字符串是否含有 ‘脏字眼’ */
	while ((!iIsDirty)&&(cLoChar=sCheckStr[iCount++])!='\0')
	{

		//printf("%d cLoChar is %x \n",__LINE__,cLoChar);

		if (IS_DOUBLE_CHAR(cLoChar)){ 
	           cHiChar=sCheckStr[iCount++];
	           if (!IS_ENGLISH_DOUBLE(cLoChar,cHiChar))
	                continue;
	           else {
	           	CONVERT_DOUBLE_TO_SINGLE(cLoChar,cLoChar,cHiChar)
	           }    
	        } 
		else if (IS_ENGLISH_CHAR(cLoChar)){
			CONVERT_CAPITAL_CHAR(cLoChar,cLoChar)
		}
		else if(IS_SHOW_ENG(cLoChar))
		{//增加支持可见ascii码
			CONVERT_CAPITAL_CHAR(cLoChar,cLoChar)
		}
		else continue;
		iIndex=0; 
		while (pstDirtyRecord[iIndex++].sDirtyStr[0]!='\0')
		{
			if (EQUAL_ENGLISH_CHAR(cLoChar,pstDirtyRecord[iIndex-1].sDirtyStr[0]))
		        if (!Eng_Dirty_Check_Word(sCheckStr+iCount,pstDirtyRecord[iIndex-1].sDirtyStr+1))
			 {
		            iIsDirty=1;
    			     strDirtyWord = string((char *)pstDirtyRecord[iIndex-1].sDirtyStr);
		            break;
		        }
		}       
	}
	shmdt(pShm);
	return iIsDirty;
}
/*******-**********************************************************-*****************************************************
Funciton    释放共享内存
input       
output      sErrMsg	//错误信息	    
return:     0: 		:success
            <0		:failed 
attention:  CGI一般不要调用
*******-****************************************************************************************************************/	
int DirtyCheck::Eng_Dirty_Destroy()
{
      if (iEngShmID==-1){
		sprintf(sErrMsg,"The ShareMemory hasn't beend Initiate");
		return -1;	

      }
      if (shmctl(iEngShmID, IPC_RMID, NULL)<0){
		sprintf(sErrMsg,"Faile to Remove The ShareMemory ,Sharem Key %d",iEngShmID);
		return -2;	
      }	
      iEngShmID=-1;
      return 0;		
}
/**********************************************************************************************************************
Funciton  列出共享内存中 dirty信息 
input      in:1 中文 2英文
reutrn  0:              //OK
	    !=0:         //Error
***********************************************************************************************************************/	
int DirtyCheck :: Dirty_List_Word(int CharSet)
{

	DIRTY_ENG_RECORD    *pstEnDirtyRecord;
	DIRTY_CHN_RECORD    *pstChnDirtyRecord;
	DIRTY_CHN_STRUCT    *pstChnIndexTab;
	DIRTY_EN_STRUCT    *pstEnIndexTab;
	unsigned char            *pChnShm;
	unsigned char            *pEnShm;	

	if (iShmID==-1){
		sprintf(sErrMsg,"The ShareMemory hasn't beend Initiate");
		return -1;	
	}
	if (iEngShmID==-1){
		sprintf(sErrMsg,"The ShareMemory hasn't beend Initiate");
		return -1;	
	}
	if ((pChnShm = (unsigned char*)shmat(iShmID, NULL ,0)) == (unsigned char *) -1) 
	{
		sprintf(sErrMsg,"Fail to shmat. Shmid is %d", iShmID);
		return -3;
	}
	pstChnIndexTab=(DIRTY_CHN_STRUCT *)pChnShm;
 	pstChnDirtyRecord=(DIRTY_CHN_RECORD *)(pChnShm+sizeof(DIRTY_CHN_STRUCT));
	
	if ((pEnShm = (unsigned char*)shmat(iEngShmID, NULL ,0)) == (unsigned char *) -1) 
	{
		sprintf(sErrMsg,"Fail to shmat. Shmid is %d", iEngShmID);
		shmdt(pChnShm);		
		return -3;
	}
	pstEnIndexTab = (DIRTY_EN_STRUCT *)pEnShm;
	pstEnDirtyRecord=(DIRTY_ENG_RECORD *)(pEnShm+sizeof(DIRTY_EN_STRUCT));
	
	int		i, j ;
	
	if ( CharSet ==1) // list chinese word
	{
		printf("Chn count: %d\n",pstChnIndexTab->iChnWordCount);
		for( i = 0 , j = 0; i <pstChnIndexTab->iChnWordCount; i++)
		{
			if ( pstChnDirtyRecord[i].sKeyWord[0]==0 )
				continue;
			printf("[%03d][keyword=%s][keyoffset=%d][nexkey=%d] %s\n",i,
				pstChnDirtyRecord[i].sKeyWord,  pstChnDirtyRecord[i].iKeyOffset,
				pstChnDirtyRecord[i].iNextKey,  pstChnDirtyRecord[i].sDirtyStr);
			j++;
			if ( j%3 == 2)
				printf("\n");
		}
	}
	else if (CharSet ==2 ) // list english word
	{
		printf("Eng count : %d\n",pstEnIndexTab->iEngWordCount);
		for( i = 0 , j = 0; i <pstEnIndexTab->iEngWordCount; i++)
		{
			if ( pstEnDirtyRecord[i].sDirtyStr[0]==0 )
				continue;
			printf("[%03d]%s\t",i,pstEnDirtyRecord[i].sDirtyStr);
			j++;
			if ( j%3 == 2)
				printf("\n");
		}
	}else
	{
		printf("pls input  1 or 2");
	}
	printf("\n");

	shmdt(pChnShm);
	shmdt(pEnShm);		

	return 0;
}
/**********************************************************************************************************************
Funciton   混合检查  只保留 中英数
input       sCheckStr: 	//待检查的字符串 
output      sErrMsg	//错误信息	    
return:     0: 		//此字符串不带有‘脏字眼’ 		    
	    1: 		//此字符串带有‘脏字眼’ 		    
	    <0:         //Error
***********************************************************************************************************************/	
int DirtyCheck :: Mix_Dirty_Check(unsigned char *sCheckStr)
{
	unsigned char	cLoChar,cHiChar;
	//		cCompChar;
	unsigned char *sReservBuff;
	unsigned char szChinese[1024*5*2]={0};
	
	int				iIsDirty=0,iCount=0,iBufCount=0,iIndex,iOffset;
	int				iKeyOff;

	DIRTY_ENG_RECORD    *pstEnDirtyRecord;
	DIRTY_CHN_RECORD    *pstChnDirtyRecord;
	DIRTY_CHN_STRUCT    *pstChnIndexTab;
	unsigned char            *pChnShm;
	unsigned char            *pEnShm;	

	if (sCheckStr==NULL||strlen((const char *)sCheckStr)==0)
	{
		sprintf(sErrMsg,"The checking string is null");
		return -1;	
	}

	if (iShmID==-1){
		sprintf(sErrMsg,"The ShareMemory hasn't beend Initiate");
		return -1;	
	}
	if (iEngShmID==-1){
		sprintf(sErrMsg,"The ShareMemory hasn't beend Initiate");
		return -1;	
	}
	if ((pChnShm = (unsigned char*)shmat(iShmID, NULL ,0)) == (unsigned char *) -1) 
	{
		sprintf(sErrMsg,"Fail to shmat. Shmid is %d", iShmID);
		return -3;
	}
	pstChnIndexTab=(DIRTY_CHN_STRUCT *)pChnShm;
 	pstChnDirtyRecord=(DIRTY_CHN_RECORD *)(pChnShm+sizeof(DIRTY_CHN_STRUCT));
	
	if ((pEnShm = (unsigned char*)shmat(iEngShmID, NULL ,0)) == (unsigned char *) -1) 
	{
		sprintf(sErrMsg,"Fail to shmat. Shmid is %d", iEngShmID);
		shmdt(pChnShm);		
		return -3;
	}
	pstEnDirtyRecord=(DIRTY_ENG_RECORD *)(pEnShm+sizeof(DIRTY_EN_STRUCT));
	

	/* 
//	sReservBuff= (unsigned char *)malloc(strlen((const char *)sCheckStr)+10);
	sReservBuff= (unsigned char *)malloc(strlen((const char *)sCheckStr)*2+1);
	if(!sReservBuff) 
	{
		shmdt(pChnShm);
		shmdt(pEnShm);		
		return -2;
	}
	*/
	sReservBuff = szChinese;
	

/////////////// 把双字英文，数字转为单字英文，数字，并过滤空格\r 等无效字符

	while((cLoChar=sCheckStr[iCount++])!='\0')
	{//先保留
 	   if (IS_DOUBLE_CHAR(cLoChar))
 	   { 
 	     cHiChar=sCheckStr[iCount++];

//printf("[%d]   cLoChar %x  cHiChar %x \n",__LINE__,cLoChar,cHiChar);

 	     if(cHiChar=='\0') break;

 	     if( IS_DIGIT_DOUBLE(cLoChar,cHiChar) )
 	     {
		    CONVERT_DOUBLE_DIGIT_TO_SINGLE(sReservBuff[iBufCount++],cLoChar,cHiChar);
	      }     	 
	     else if (IS_ENGLISH_DOUBLE(cLoChar,cHiChar))
	     {
//printf("[%d]   cLoChar %x  cHiChar %x \n",__LINE__,cLoChar,cHiChar);
		    CONVERT_DOUBLE_TO_SINGLE(sReservBuff[iBufCount++],cLoChar,cHiChar);
	     }
 	     else if (IS_CHINESE_CHAR(cLoChar,cHiChar))//
	     {
//printf("[%d]   cLoChar %x  cHiChar %x \n",__LINE__,cLoChar,cHiChar);
//	     	if(!IS_CHINESE_PUNC(cLoChar,cHiChar))
	        if ( IS_CHINESE_PUNC(cLoChar,cHiChar) &&
			(!IS_DIGIT_DOUBLE(cLoChar,cHiChar)) && 
			(!IS_ENGLISH_DOUBLE(cLoChar,cHiChar))) 
	        {
	        	;// do nothing
		 }else
		 {// 中文标点符号必须把中文英文和数字都去掉
			sReservBuff[iBufCount++]=cLoChar;
			sReservBuff[iBufCount++]=cHiChar;

		 }
 	     }	
	   } 
  	   else if (IS_ENGLISH_CHAR(cLoChar)){
		CONVERT_CAPITAL_CHAR(sReservBuff[iBufCount++],cLoChar);
	   }
	   else if(IS_DIGIT(cLoChar))
	   {
		sReservBuff[iBufCount++] = cLoChar;
	   }
	   else if (cLoChar=='\r'||cLoChar==' ')
	  {//把空格和\r去掉, 方便英文词组检查
	        continue;
	   }
	   else if(IS_SHOW_ENG(cLoChar))
          {//增加支持可见ascii码
		CONVERT_CAPITAL_CHAR(sReservBuff[iBufCount++],cLoChar);
          }
	   
	}//end while
	sReservBuff[iBufCount]= 0;

	#ifdef DIRTYDEBUG 
	printf("double to single Mix_Dirty_Check  %s len is %d \n",sReservBuff,strlen((const char *)sReservBuff));
	#endif
	
	iCount =0;
	//pstDirtyRecord =pstDirtyCore->astChnDirtyRec;
	while((!iIsDirty)&&(cLoChar=sReservBuff[iCount++])!='\0')
	{
	   if(IS_ENGLISH_CHAR(cLoChar) ||IS_DIGIT(cLoChar) )
	   {

	 	iIndex=0; 

	 	while (pstEnDirtyRecord[iIndex++].sDirtyStr[0]!='\0')
		{
		  if( 	 EQUAL_ENGLISH_CHAR(cLoChar,pstEnDirtyRecord[iIndex-1].sDirtyStr[0]))		
		  {
			  if(strncasecmp((const char *)(sReservBuff+iCount-1),(const char *)( pstEnDirtyRecord[iIndex-1].sDirtyStr),
						strlen((const char *)pstEnDirtyRecord[iIndex-1].sDirtyStr))==0)
			  {
				iIsDirty=4;
				strDirtyWord = string( (char *)pstEnDirtyRecord[iIndex-1].sDirtyStr);
				break;
			  }

		  }
		 }//while engdirty
	    }//if eng char
	  else if(IS_DOUBLE_CHAR(cLoChar))	
	  {
	   	cHiChar=sReservBuff[iCount++]; 
	   	if(cHiChar =='\0') break;
	   	if(IS_CHINESE_CHAR(cLoChar,cHiChar))
	   	{
	   	  CAL_INDEX_OFFSET(iIndex,cLoChar,cHiChar);
	   	  iOffset=pstChnIndexTab->iDirtyIndexTable[iIndex]; //该字是否关键字//
	   	  while ((iOffset!=-1)&&(iOffset>=0)&&(iOffset<C_MAX_WORD_NUM))
		  {	
		  	iKeyOff=pstChnDirtyRecord[iOffset].iKeyOffset;
			if(pstChnDirtyRecord[iOffset].sKeyWord[0]!=cLoChar
				||pstChnDirtyRecord[iOffset].sKeyWord[1]!=cHiChar)
				//关键字不匹配可能发生在loadmem的时候
				break;
			if(iCount - 2 >= pstChnDirtyRecord[iOffset].iKeyOffset)
			{
				 if(strncasecmp((const char *)(sReservBuff+(iCount-2 - pstChnDirtyRecord[iOffset].iKeyOffset)),
						(const char *)pstChnDirtyRecord[iOffset].sDirtyStr,
					    strlen((const char *)pstChnDirtyRecord[iOffset].sDirtyStr))==0)
				 {
					iIsDirty=3;
				       strDirtyWord = string((char *) pstChnDirtyRecord[iOffset].sDirtyStr);
					break;
				 }

			}

		    iOffset=pstChnDirtyRecord[iOffset].iNextKey;  // 下一个含有该关键字得短语//
		  }//chndirty
	   	}// if chinese char
	   }//if doublechar
	}//end while

	if(iIsDirty != 0)
	{//发现有脏话
//		free((void*)sReservBuff);
		shmdt(pChnShm);
		shmdt(pEnShm);		
		return iIsDirty;
	}



/////////////// 把单字英文，数字转为双字英文，数字，并过滤空格\r 等无效字符
	iCount = 0;
	iBufCount = 0;
	
	while((cLoChar=sCheckStr[iCount++])!='\0')
	{//先保留
 	   if (IS_DOUBLE_CHAR(cLoChar))
 	   { 
 	     cHiChar=sCheckStr[iCount++];
//printf("[%d]   cLoChar %x  cHiChar %x \n",__LINE__,cLoChar,cHiChar);
 	     if(cHiChar=='\0') break;

 	     if( IS_DIGIT_DOUBLE(cLoChar,cHiChar) )
 	     {
//		    CONVERT_DOUBLE_DIGIT_TO_SINGLE(sReservBuff[iBufCount++],cLoChar,cHiChar);
			sReservBuff[iBufCount++]=cLoChar;
			sReservBuff[iBufCount++]=cHiChar;
	      }     	 
	     else if (IS_ENGLISH_DOUBLE(cLoChar,cHiChar))
	     {
//printf("[%d]   cLoChar %x  cHiChar %x \n",__LINE__,cLoChar,cHiChar);
//		    CONVERT_DOUBLE_TO_SINGLE(sReservBuff[iBufCount++],cLoChar,cHiChar);
			sReservBuff[iBufCount++]=cLoChar;
			sReservBuff[iBufCount++]=cHiChar;
	     }
 	     else if (IS_CHINESE_CHAR(cLoChar,cHiChar))//
	     {
//printf("[%d]   cLoChar %x  cHiChar %x \n",__LINE__,cLoChar,cHiChar);
//	     	if(!IS_CHINESE_PUNC(cLoChar,cHiChar))
	        if ( IS_CHINESE_PUNC(cLoChar,cHiChar) &&
			(!IS_DIGIT_DOUBLE(cLoChar,cHiChar)) && 
			(!IS_ENGLISH_DOUBLE(cLoChar,cHiChar))) 
	        {
	        	;// do nothing
		 }else
		 {// 中文标点符号必须把中文英文和数字都去掉
			sReservBuff[iBufCount++]=cLoChar;
			sReservBuff[iBufCount++]=cHiChar;

		 }
 	     }	
	   } 
  	   else if (IS_ENGLISH_CHAR(cLoChar))
	   {
//		CONVERT_CAPITAL_CHAR(sReservBuff[iBufCount++],cLoChar);
//printf("[%d]   cLoChar %x   \n",__LINE__,cLoChar);
		   // 全部转为大写
		    if((cLoChar>='a'&&cLoChar<='z'))
		    {
			sReservBuff[iBufCount++] = 0xA3;
			sReservBuff[iBufCount++] = cLoChar+0x60;

		    }else
		    {
			sReservBuff[iBufCount++] = 0xA3;
			sReservBuff[iBufCount++] = cLoChar+0x80;
		    }
	   }
	   else if(IS_DIGIT(cLoChar))
	   {
		sReservBuff[iBufCount++] = 0xA3;
		sReservBuff[iBufCount++] = cLoChar+0x80;		
	   }
	   else if (cLoChar=='\r'||cLoChar==' ')
	  {//把空格和\r去掉, 方便英文词组检查
	        continue;
	   }
	   else if(IS_SHOW_ENG(cLoChar))
          {//增加支持可见ascii码
//		CONVERT_CAPITAL_CHAR(sReservBuff[iBufCount++],cLoChar);
          }
	   
	}//end while
	sReservBuff[iBufCount]= 0;

	#ifdef DIRTYDEBUG 
	printf("single to double  Mix_Dirty_Check  %s  len  is %d \n",sReservBuff,strlen((const char *)sReservBuff));
	#endif
	
	iCount =0;
	//pstDirtyRecord =pstDirtyCore->astChnDirtyRec;
	while((!iIsDirty)&&(cLoChar=sReservBuff[iCount++])!='\0')
	{
	   if(IS_ENGLISH_CHAR(cLoChar) ||IS_DIGIT(cLoChar) )
	   {

	 	iIndex=0; 

	 	while (pstEnDirtyRecord[iIndex++].sDirtyStr[0]!='\0')
		{
		  if( 	 EQUAL_ENGLISH_CHAR(cLoChar,pstEnDirtyRecord[iIndex-1].sDirtyStr[0]))		
		  {
			  if(strncasecmp((const char *)(sReservBuff+iCount-1),(const char *)( pstEnDirtyRecord[iIndex-1].sDirtyStr),
						strlen((const char *)pstEnDirtyRecord[iIndex-1].sDirtyStr))==0)
			  {
				iIsDirty=6;
				strDirtyWord = string( (char *)pstEnDirtyRecord[iIndex-1].sDirtyStr);
				break;
			  }

		  }
		 }//while engdirty
	    }//if eng char
	  else if(IS_DOUBLE_CHAR(cLoChar))	
	  {
	   	cHiChar=sReservBuff[iCount++]; 
	   	if(cHiChar =='\0') break;
	   	if(IS_CHINESE_CHAR(cLoChar,cHiChar))
	   	{
	   	  CAL_INDEX_OFFSET(iIndex,cLoChar,cHiChar);
	   	  iOffset=pstChnIndexTab->iDirtyIndexTable[iIndex]; //该字是否关键字//
	   	  while ((iOffset!=-1)&&(iOffset>=0)&&(iOffset<C_MAX_WORD_NUM))
		  {	
		  	iKeyOff=pstChnDirtyRecord[iOffset].iKeyOffset;
			if(pstChnDirtyRecord[iOffset].sKeyWord[0]!=cLoChar
				||pstChnDirtyRecord[iOffset].sKeyWord[1]!=cHiChar)
				//关键字不匹配可能发生在loadmem的时候
				break;
			if(iCount - 2 >= pstChnDirtyRecord[iOffset].iKeyOffset)
			{
				 if(strncasecmp((const char *)(sReservBuff+(iCount-2 - pstChnDirtyRecord[iOffset].iKeyOffset)),
						(const char *)pstChnDirtyRecord[iOffset].sDirtyStr,
					    strlen((const char *)pstChnDirtyRecord[iOffset].sDirtyStr))==0)
				 {
					iIsDirty=5;
				       strDirtyWord = string((char *) pstChnDirtyRecord[iOffset].sDirtyStr);
					break;
				 }

			}

		    iOffset=pstChnDirtyRecord[iOffset].iNextKey;
		  }//chndirty
	   	}// if chinese char
	   }//if doublechar
	}//end while

//	free((void*)sReservBuff);
	shmdt(pChnShm);
	shmdt(pEnShm);

	return iIsDirty;
	
}

Dirty::Dirty()
{
	 m_iEnKey = -1;
        m_iEnMaxNum = -1;
        m_strEnFileName ="";
        m_iChnKey = -1;
        m_iChnMaxNum = -1;
        m_strChnFileName = "";
	 m_cd = (iconv_t)-1;
	 m_pGbkBuff = NULL;
	 m_iGbkBuffLen = 0;
	 memset(m_szGbkBuff,0x00,sizeof(m_szGbkBuff));
}
string Dirty :: GetErrMsg()
{
	return m_strErrMsg;
}
string Dirty :: GetDirtyWord()
{
	return m_CDirtyCheck.strDirtyWord;
}
void Dirty :: SetEnShmKey(key_t iKey)
{
	m_iEnKey = iKey;
}
void Dirty :: SetChnShmKey(key_t iKey)
{
	m_iChnKey = iKey;
}
void Dirty :: SetEnMaxNum(int iNum)
{
	m_iEnMaxNum= iNum;
}
void Dirty :: SetChnMaxNum(int iNum)
{
	m_iChnMaxNum= iNum;
}
void Dirty :: SetEnFileName(string strFileName)
{
	m_strEnFileName= strFileName;
}
void Dirty :: SetChnFileName(string strFileName)
{
	m_strChnFileName= strFileName;
}

int Dirty::InitShm(void)
{
    int iRet = -1;
#if 0
    struct stat stFileStat;

    if (stat(m_strEnFileName.c_str(), &stFileStat) < 0)
    {
	 m_strErrMsg = "脏话文件："+m_strEnFileName+" 不存在！";
        return -1;
    }
    if (stat(m_strChnFileName.c_str(), &stFileStat) < 0)
    {
	 m_strErrMsg = "脏话文件："+m_strChnFileName+" 不存在！";
        return -1;
    }
#endif
    if((m_iEnKey == m_iChnKey)||(m_iEnKey < 0) ||(m_iChnKey < 0))
    {
	 m_strErrMsg = "m_iEnKey same to m_iChnKey or shmkey < 0";
        return -1;
    }
    if((m_iEnMaxNum < 0) ||(m_iChnMaxNum < 0))
    {
	 m_strErrMsg = "m_iEnKey same to m_iChnKey or shmkey < 0";
        return -1;
    }

    iRet = m_CDirtyCheck.Eng_Dirty_Init(m_iEnKey, m_iEnMaxNum, (char *)m_strEnFileName.c_str());
    if(iRet != 0)
    {
	    m_strErrMsg = "m_CDirtyCheck.Eng_Dirty_Init error : "+string(m_CDirtyCheck.sErrMsg );
	    return -2;
    }
    iRet = m_CDirtyCheck.Chn_Dirty_Init(m_iChnKey, m_iChnMaxNum, (char *)m_strChnFileName.c_str());
    if(iRet != 0)
    {
	    m_strErrMsg = "m_CDirtyCheck.Chn_Dirty_Init error : "+string(m_CDirtyCheck.sErrMsg );
	    return -2;
    }

    return 0;
}

int Dirty :: CheckDirtyWord(int iCheckType, string strCheckStr)
{
	int iRet = -1;
	
	if((iCheckType != UTF8_CODE) &&(iCheckType != GBK_CODE))
	{
		m_strErrMsg = "CheckStr CodeType not  UTF8_CODE or GBK_CODE";
		return -1;
	}
	
	if(strCheckStr.length() == 0)
	{
		return 0;
	}

	unsigned char * pCheckStr = (unsigned char *)strCheckStr.c_str();

	if(iCheckType == UTF8_CODE)
	{
		m_iGbkBuffLen = sizeof(m_szGbkBuff);	
		m_pGbkBuff = m_szGbkBuff;
		if(!m_pGbkBuff) 
		{
			m_iGbkBuffLen = 0;	
			m_strErrMsg = "malloc error!!!!!!!!";
			return -1;
		}

		#ifdef DIRTYDEBUG 
		cout << "strCheckStr.length() is " << strCheckStr.length() << "  and  malloc size is " << m_iGbkBuffLen << endl;
		#endif
		
		iRet =  UTF8ToGBK(strCheckStr,(char *)m_pGbkBuff,m_iGbkBuffLen);
		
		if((iRet < 0)||(iRet >= (int)m_iGbkBuffLen))
		{
			#ifdef DIRTYDEBUG 
			cout <<endl<< "Utf8ToGbk ret is  " << iRet << endl;
			cout << "UTF8ToGBK error:"<<  m_strErrMsg <<endl;
			#endif
		
//			FreeMemory();
			
			return -1;
		}else
		{
			*(m_pGbkBuff+iRet)=0;
		}

		pCheckStr = m_pGbkBuff;
	}

	iRet = InitShm();
	
	if(iRet != 0)
	{
		#ifdef DIRTYDEBUG 
		cout << "CheckDirtyWord() attch shm  error : "<<m_strErrMsg<< endl;
		#endif

//		FreeMemory();
		
		return -1;
	}


///english check
	iRet = m_CDirtyCheck.Eng_Dirty_Check((unsigned char *)pCheckStr);

	if(iRet == 0)
	{
		#ifdef DIRTYDEBUG 
		cout << "Eng_Dirty_Check  " <<pCheckStr << "  is clean" << endl;
		#endif
		
	}else if(iRet == 1)
	{
		#ifdef DIRTYDEBUG 
		cout <<"Eng_Dirty_Check  "<< pCheckStr << "  is dirty  " << endl;
		cout <<"dirty word is "<< m_CDirtyCheck.strDirtyWord << endl;
		#endif

//		FreeMemory();
			
		return 1;
	}else
	{
		#ifdef DIRTYDEBUG 
		cout << "Eng_Dirty_Check error : "<<m_CDirtyCheck.sErrMsg << endl;
		#endif
		m_strErrMsg = string(m_CDirtyCheck.sErrMsg);

//		FreeMemory();
		
		return -1;
	}

///chinese check
	iRet = m_CDirtyCheck.Chn_Dirty_Check((unsigned char *)pCheckStr);

	if(iRet == 0)
	{
		#ifdef DIRTYDEBUG 
		cout << "Chn_Dirty_Check  " <<pCheckStr << "  is clean" << endl;
		#endif
		
	}else if(iRet == 1)
	{
		#ifdef DIRTYDEBUG 
		cout <<"Chn_Dirty_Check  "<< pCheckStr << "  is dirty" << endl;
		cout <<"dirty word is "<< m_CDirtyCheck.strDirtyWord << endl;		
		#endif

//		FreeMemory();

		return 1;
	}else
	{
		#ifdef DIRTYDEBUG 
		cout << "Chn_Dirty_Check error : "<<m_CDirtyCheck.sErrMsg << endl;
		#endif

		m_strErrMsg = string(m_CDirtyCheck.sErrMsg);

//		FreeMemory();

		return -1;		
	}

//Check mix
	iRet = m_CDirtyCheck.Mix_Dirty_Check((unsigned char *)pCheckStr);

	if(iRet == 0)
	{
		#ifdef DIRTYDEBUG 
		cout << "Mix_Dirty_Check  " <<pCheckStr << "  is clean" << endl;
		#endif

//		FreeMemory();

		return 0;
		
	}else 
	{
		#ifdef DIRTYDEBUG 
		cout <<"Mix_Dirty_Check  "<< pCheckStr << "  is dirty" << endl;
		cout <<"dirty word is "<< m_CDirtyCheck.strDirtyWord << endl;		
		#endif

//		FreeMemory();

		return 1;
	}
		
	return -1;
}

int Dirty::List_Ditry_Word(int iCharSet)
{
	int iRet = -1;
	if((iCharSet != SHOW_CHINESE)&&(iCharSet != SHOW_ENGLISH))
	{
		m_strErrMsg = "iCharSet error ";
		return -1;
	}

	iRet = InitShm();
	
	if(iRet != 0)
	{
		#ifdef DIRTYDEBUG 
		cout << "List_Ditry_Word() attch shm  error : "<<m_strErrMsg << endl;
		#endif
		
		return -1;
	}

	
	iRet = m_CDirtyCheck.Dirty_List_Word(iCharSet);
	if(iRet != 0)
	{
		#ifdef DIRTYDEBUG 
		cout << "Dirty_List_Word() error : "<<m_CDirtyCheck.sErrMsg << endl;
		#endif

		m_strErrMsg = string(m_CDirtyCheck.sErrMsg);

		return -1;
	}	

	return 0;
}
 int Dirty:: UTF8ToGBK(string strIn,char *sOut,int iMaxOutLen)
 {
	const char * pIn = strIn.c_str();
	char * pOut = sOut;
	size_t ret;
	int iLeftLen;
	//iconv_t cd;
	int iInLen = strIn.length() ;

	if(iInLen == 0)
	{
		return 0;
	}
	
	if((iconv_t)-1 == m_cd)
	{
		#ifdef DIRTYDEBUG 
		cout << "((iconv_t)-1 == m_cd) so open it "<< endl;
		#endif

		m_cd =  iconv_open("gbk", "utf-8");

		if((iconv_t)-1 == m_cd)
		{
			m_strErrMsg =  "ERROR: iconv_open: "+string(strerror(errno));

			return -1;
		}
		
	}

	iLeftLen = iMaxOutLen;

	ret = iconv(m_cd, (char**)&pIn, (size_t*)&iInLen, &pOut, (size_t *)&iLeftLen);
        if (ret == (size_t)-1){
   	         m_strErrMsg = "iconv "+strIn+": "+ strerror(errno);

                return -1;
        }
		
	return (iMaxOutLen - iLeftLen);
 }

void Dirty:: FreeMemory()
{
	/*
	if(m_pGbkBuff != NULL)
	{
		#ifdef DIRTYDEBUG 
		cout << "m_pGbkBuff != NULL so free memory "<< endl;
		#endif
		
		free((void*)m_pGbkBuff);
		m_pGbkBuff = NULL;		
		m_iGbkBuffLen = 0;
	}
	*/
}

