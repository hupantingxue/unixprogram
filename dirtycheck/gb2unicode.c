#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iconv.h>
#include <errno.h>
#include <unistd.h>
#include "gb2unicode.h"

#define LibraryRows 6763
char Unicode_Library[2*LibraryRows];
char *Index_Unicode[LibraryRows];


int gb2unicode(char *gb, char *unicode, int *unicode_len)
{
	char Un[2];
	int iGb=0, gbLen, iUn=0;

	gbLen = strlen(gb);

	//printf("gblen: %d\n", gbLen);

	while (iGb<gbLen) 
	{
		if ((unsigned char)gb[iGb++] > 0x7f)
		{
			if ((unsigned char)gb[iGb] > 0x7f)
			{
				iGb++;
				if (!char_gb2unicode(gb+iGb-2, Un))
				{
					unicode[iUn++] = Un[0];
					unicode[iUn++] = Un[1];
				}				
			}
		}
		else 
		{
			unicode[iUn++] = 0;
			unicode[iUn++] = *(gb+iGb-1);
		}
	}

	*unicode_len = iUn;
	return 0;
}

int unicode2gb(char *unicode, char *gb, int unicode_len)
{
	int iUn = 0, iGb = 0;

	while (iUn<unicode_len)
	{
		if (unicode[iUn])
		{
			if (!char_unicode2gb(unicode+iUn, gb+iGb))
				iGb+=2;
		}
		else 
		{
			if (unicode[iUn+1]) gb[iGb++] = unicode[iUn+1];
		}
		iUn+=2;
	}
	gb[iGb] = 0;
        return 0;
}


void InitUnicodeIndex()
{
	int i;

	for (i=0;i<LibraryRows;i++)
	{
		Index_Unicode[i] = &Unicode_Library[2*i];
	}
}



/***************************************************************/
void MySetWordA(void *p, void *w)
{
	char *p1, *p2;

	p1 = (char*)p;
	p2 = (char*)w;

	p1[0] = p2[0];
	p1[1] = p2[1];
}

int HalfSearchUnicode(unsigned short int UnicodeValue, int LowIndex, int HighIndex)
{
	unsigned short int MidValue;
	int MidIndex;

	if(HighIndex - LowIndex > 1)
	{
		MidIndex = (LowIndex + HighIndex) / 2;
		MySetWordA(&MidValue , Index_Unicode[MidIndex]);

		if (MidValue == UnicodeValue) return MidIndex;
		else if (MidValue < UnicodeValue) 
			return HalfSearchUnicode(UnicodeValue, MidIndex, HighIndex);
		else return HalfSearchUnicode(UnicodeValue, LowIndex, MidIndex);
	}
	else return -1;
}


/***************************************************/
int char_gb2unicode(char *gb,char *unicode)
{
	int x, y, pos;

	x = *(unsigned char *)gb - 160;
	y = *((unsigned char *)gb+1) - 160;	
	pos = (y + (x-16)*94 - 1) * 2;

	if (pos < 0 || pos >= sizeof(Unicode_Library)) return -1;

	*unicode = Unicode_Library[pos];
	*(unicode+1) = Unicode_Library[pos+1];
	return 0;
}

int char_unicode2gb(char *unicode, char *gb)
{
	int x, y, pos, index;

	index = GetIndexOfUnicode(unicode);
	if (index == -1) return -1;

	pos =  Index_Unicode[index] - Unicode_Library;

	x = (pos/2)/94 + 16;
	y = ((pos/2) %94) + 1;

	*(unsigned char *)gb = x + 160;
	*(unsigned char *)(gb+1) = y + 160;

	return 0;
}

int GetIndexOfUnicode(char *Unicode)
{
	unsigned short int UnicodeValue, LowValue, HighValue;

	MySetWordA(&UnicodeValue, Unicode);
	MySetWordA(&LowValue, Index_Unicode[0]);
	MySetWordA(&HighValue, Index_Unicode[LibraryRows-1]);

	if (UnicodeValue == LowValue) return 0;
	else if(UnicodeValue == HighValue) return LibraryRows-1;
	else return HalfSearchUnicode(UnicodeValue, 0, LibraryRows-1);
}

void SortUnicodeBubble()
{
	int i, j, CurBigIndex;
	unsigned short int  CurBigValue, CurValue;
	char *swap;

	for (i=0;i<LibraryRows;i++)
	{
		MySetWordA(&CurBigValue, Index_Unicode[i]);
		CurBigIndex = i;

		for (j=i+1;j<LibraryRows;j++)
		{
			MySetWordA(&CurValue, Index_Unicode[j]);
			if (CurValue > CurBigValue)
			{
				CurBigValue = CurValue;
				CurBigIndex = j;
			}
		}

		//swap		
		swap = Index_Unicode[i];
		Index_Unicode[i] = Index_Unicode[CurBigIndex];
		Index_Unicode[CurBigIndex] = swap;
	}	
}

void SortUnicodeQuick(int LowIndex, int HighIndex)
{
	unsigned short int MiddleValue, CurValue;
	int iLowIndex, iHighIndex;
	char *pSwp;

	MySetWordA(&MiddleValue, Index_Unicode[(LowIndex + HighIndex) / 2]);
	iLowIndex = LowIndex;
	iHighIndex = HighIndex;

	while(iLowIndex <= iHighIndex)
	{
		while (1)
		{
			MySetWordA(&CurValue, Index_Unicode[iLowIndex]);
			if(CurValue < MiddleValue) iLowIndex++;
			else break;
		}

		while (1)
		{
			MySetWordA(&CurValue, Index_Unicode[iHighIndex]);
			if(CurValue > MiddleValue) iHighIndex--;
			else break;
		}

		if(iLowIndex <= iHighIndex)
		{
			pSwp = Index_Unicode[iLowIndex];
			Index_Unicode[iLowIndex] = Index_Unicode[iHighIndex];
			Index_Unicode[iHighIndex] = pSwp;

			iHighIndex --;
			iLowIndex ++;
		}
	}
	if (LowIndex < iHighIndex) SortUnicodeQuick(LowIndex, iHighIndex);
	if (iLowIndex < HighIndex) SortUnicodeQuick(iLowIndex, HighIndex);	
}

int Unicode_Lib_Init(char *filename)
{
	int infd, count;

	if ((infd = open(filename, O_RDONLY)) == -1) return 1;

	if ((count = read(infd, Unicode_Library, sizeof(Unicode_Library))) != sizeof(Unicode_Library))
		return 2;

	close(infd);

	InitUnicodeIndex();
	//	SortUnicodeBubble();
	SortUnicodeQuick(0, LibraryRows-1);
	return 0;
}


/****************************************************************************/
/*void Check_Index()
  {
  int i;
  unsigned short int first, second;

  printf("begin check\n");
  for(i=1;i<LibraryRows;i++)
  {
  MySetWordA(&first, Index_Unicode[i-1]);
  MySetWordA(&second, Index_Unicode[i]);
  if (second <= first) 
  {
  printf("error : %d\n", i);
  break;
  }
  }
  if (Index_Unicode[i] != Bubble[i]) 
  {
  printf("error:%d\n", i);
  break;
  }
  }
  */


int file_transcode(char *infile, char *outfile)
{
	int infd, outfd, count;
	char inbuf[5], outbuf[2], tmp[5] ;

	if ((infd = open(infile, O_RDONLY)) == -1) exit(1);
	if ((outfd = open(outfile, (O_WRONLY | O_CREAT | O_TRUNC), (S_IRUSR | S_IWUSR))) == -1) exit(2);

	while((count = read(infd, inbuf, sizeof(inbuf))) > 0)
	{
		tmp[0] = '0';
		tmp[1] = 'x';
		tmp[4] = 0;

		memcpy(tmp+2, inbuf, 2);
		outbuf[0] = strtol(tmp, NULL, 16);

		memcpy(tmp+2, inbuf+2, 2);
		outbuf[1] = strtol(tmp, NULL, 16);

		if(write(outfd, outbuf, 2) != 2) exit(3);
	}

	close(infd);
	close(outfd);
        return 0;
}	




typedef unsigned long	UCS4;
typedef unsigned short	UCS2;
typedef unsigned char	UTF8;

const UCS4 kMaximumUCS2 =			0x0000FFFFUL;


/* ================================================================ */

UCS4 offsetsFromUTF8[6] =	{0x00000000UL, 0x00003080UL, 0x000E2080UL, 
	0x03C82080UL, 0xFA082080UL, 0x82082080UL};
char bytesFromUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5};

UTF8 firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};



/* ================================================================ */

int	ConvertUTF8toUCS2 (
		unsigned char* sourceStart, unsigned char* sourceEnd, 
		unsigned char* target, int* iCount)
{
	int iRet = 0;
	unsigned char* source = sourceStart;
	*iCount =0;


	while (source < sourceEnd) 
	{

		unsigned long ch = 0;
		unsigned short extraBytesToWrite = bytesFromUTF8[*source];
		if (source + extraBytesToWrite > sourceEnd) {
			iRet=1; 
			break;
		};
		switch(extraBytesToWrite) {	/* note: code falls through cases! */
			case 5:	ch += *source++; ch <<= 6;
			case 4:	ch += *source++; ch <<= 6;
			case 3:	ch += *source++; ch <<= 6;
			case 2:	ch += *source++; ch <<= 6;
			case 1:	ch += *source++; ch <<= 6;
			case 0:	ch += *source++;
		};
		ch -= offsetsFromUTF8[extraBytesToWrite];

		//printf("ch:%x\n", ch);

		if (ch <= kMaximumUCS2) 
		{
			memcpy(target, (char *)(&ch)+1, 1);
			memcpy(target+1, (char *)(&ch), 1);
			target+=2;
			//printf("target:%x\n", *(target-2));
		}
		else
		{
			iRet=2;
			break;
		}

		(*iCount)+=2;
	};


	return iRet;
}

int Gb2312ToUtf8_2(const char * sIn,
		int iInLen,
		char * sOut,
		int iMaxOutLen)
{
	const char * pIn = sIn;
	char * pOut = sOut;
	size_t ret;
	int iLeftLen;
	iconv_t cd;

	/* ¿ÉÖ§³Ö¶àÖÖ±àÂëµÄ×ª»» */
	/* gb2312×ª»»Îªutf-8 */
	cd = iconv_open("utf-8", "gb2312");
	if (cd == (iconv_t)-1)
	{
		fprintf(stderr, "ERROR: iconv_open: %s\n", strerror(errno));
		return -1;
	}

	iLeftLen = iMaxOutLen;

	/* ×ª»»´úÂë */	
	ret = iconv(cd, (char**)&pIn, (size_t*)&iInLen, &pOut, (size_t *)&iLeftLen);
	if (ret == (size_t)-1){
		fprintf(stderr, "iconv %s: %s", sIn, strerror(errno));
		return -1;
	}

	iconv_close(cd);

	/* ·µ»Ø±àÂëºósOut µÄ×Ö·ûÊý */
	return (iMaxOutLen - iLeftLen);
}

int Utf8ToGb2312(const char * sIn,
		int iInLen,
		char * sOut,
		int iMaxOutLen)
{
	const char * pIn = sIn;
	char * pOut = sOut;
	size_t ret;
	int iLeftLen;
	iconv_t cd;

	/* ¿ÉÖ§³Ö¶àÖÖ±àÂëµÄ×ª»» */
	/* gb2312×ª»»Îªutf-8 */
	cd = iconv_open("gb2312", "utf-8");
	if (cd == (iconv_t)-1)
	{
		fprintf(stderr, "ERROR: iconv_open: %s\n", strerror(errno));
		return -1;
	}

	iLeftLen = iMaxOutLen;

	/* ×ª»»´úÂë */	
	ret = iconv(cd, (char**)&pIn, (size_t*)&iInLen, &pOut, (size_t *)&iLeftLen);
	if (ret == (size_t)-1){
		fprintf(stderr, "iconv %s: %s", sIn, strerror(errno));
		return -1;
	}

	iconv_close(cd);

	pOut[iMaxOutLen - iLeftLen] = 0;

	/* ·µ»Ø±àÂëºósOut µÄ×Ö·ûÊý */
	return (iMaxOutLen - iLeftLen);
}


int gb_uni(char *gb, char *uni)
{
	char pe[5], sTemp[10000];
	int i, iLen;

	strcpy(uni, "");
	strcpy(sTemp, "");

	gb2unicode(gb, sTemp, &iLen);

	i=0;
	while(i<iLen)
	{
		strcat(uni, "&#x");
		sprintf(pe, "%.2x", (unsigned char)sTemp[i]); //Ð¡ÓÚ10µÄCODEÊä³öµ½ÍøÒ³Ê±±ØÐë²¹0
		strcat(uni, pe);
		sprintf(pe, "%.2x", (unsigned char)sTemp[++i]);
		strcat(uni, pe);
		strcat(uni, ";");

		++i;
	}

	return 0;
}

int Gb2312ToUtf8(char * sIn,
		int iInLen,
		char * sOut,
		int iMaxOutLen)

{
	int i;
	for(i=0;i<iInLen;i++)
	{
		if ((sIn[i] < 0x20) && (sIn[i] > 0x0) && (sIn[i]!=0x09) && (sIn[i]!=0x0D))
			sIn[i] = 0x20;
	}
	Gb2312ToUtf8_2(sIn, iInLen, sOut, iMaxOutLen);
	return 0;
}


int GbkToUtf8(const char * sIn,	int iInLen,char * sOut,	int iMaxOutLen)
{
	const char * pIn = sIn;
	char * pOut = sOut;
	size_t ret;
	int iLeftLen;
	iconv_t cd;

	/* ¿É§³¿à±àµÄª»» */
	/* gb2312¿»»¿utf-8 */
	cd = iconv_open("utf-8", "gbk");
	if (cd == (iconv_t)-1)
	{
		fprintf(stderr, "ERROR: iconv_open: %s\n", strerror(errno));
		return -1;
	}

	iLeftLen = iMaxOutLen;

	ret = iconv(cd, (char**)&pIn, (size_t*)&iInLen, &pOut, (size_t *)&iLeftLen);
	if (ret == (size_t)-1){
		fprintf(stderr, "iconv %s: %s", sIn, strerror(errno));
		iconv_close(cd);				
		return -1;
	}

	iconv_close(cd);

	return (iMaxOutLen - iLeftLen);
}
int Utf8ToGbk(const char * sIn,int iInLen,char * sOut,int iMaxOutLen)
{
	const char * pIn = sIn;
	char * pOut = sOut;
	size_t ret;
	int iLeftLen;
	iconv_t cd;

	/* ¿É§³¿à±àµÄª»» */
	/* gb2312¿»»¿utf-8 */
	cd = iconv_open("gbk", "utf-8");
	if (cd == (iconv_t)-1)
	{
		fprintf(stderr, "ERROR: iconv_open: %s\n", strerror(errno));
		return -1;
	}

	iLeftLen = iMaxOutLen;

	ret = iconv(cd, (char**)&pIn, (size_t*)&iInLen, &pOut, (size_t *)&iLeftLen);
	if (ret == (size_t)-1){
		fprintf(stderr, "iconv %s: %s", sIn, strerror(errno));
		return -1;
	}

	iconv_close(cd);

	pOut[iMaxOutLen - iLeftLen] = 0;

    return (iMaxOutLen - iLeftLen);
}
	/*
	main()
	{
	int i;

	i=Unicode_Lib_Init("/usr/local/wap/bin/GB2Unicode.lib");

	printf("error:%d\n", i);

	test();


	//	Check_Index();
	}
	*/
