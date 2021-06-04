

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


static int XSectionFindKey(const char *pInputFileName, const char *pInputSectionName, const char *pInputKey)
{
	if(pInputFileName==NULL || pInputSectionName==NULL || pInputKey==NULL)
		return -1;

	if(access(pInputFileName, F_OK)!=0)
		return -2;

	FILE *pTempFile=NULL;
	pTempFile=fopen(pInputFileName, "r");
	if(pTempFile==NULL)
	{
		printf("Damon ==> open file[%s] failed !\n", pInputFileName);
		return -3;
	}

	int i=0;
	char *pTempLine=NULL;
	size_t tTempLen;
	size_t tTempReadLen;
	int tTempBreakLine=0;

	int tTempFindSection=0, tTempFindKey=0;
	
	while((tTempReadLen=getline(&pTempLine, &tTempLen, pTempFile))!=-1)
	{
		tTempBreakLine=0;
		for(i=0; i<tTempReadLen; i++)
		{
			if(pTempLine[i]<=0x20)
				continue;
			if(pTempLine[i]=='#')
				tTempBreakLine=1;
			break;
		}

		if(tTempBreakLine==0 && i<tTempReadLen)
		{
			char *pTempStartPt=strchr(pTempLine+i, '[');
			char *pTempEndPt=strchr(pTempLine+i, ']');
			if(pTempStartPt!=NULL && pTempEndPt!=NULL)
			{
				if(tTempFindSection==1)
				{
					break;
				}
				
				if(strncmp(pTempStartPt+1, pInputSectionName, strlen(pInputSectionName))==0)
				{
					tTempFindSection=1;
				}
			}else
			{
				if(tTempFindSection==1)
				{
					if(strncmp(pTempLine+i, pInputKey, strlen(pInputKey))==0)
					{
						tTempFindKey=1;
						break;
					}
				}
			}
		}else if(tTempBreakLine==1)
		{
			continue;
		}
	}

	if(pTempLine)
		free(pTempLine);

	fclose(pTempFile);

	printf("Damon ==> section save val : %d %d \n", tTempFindSection, tTempFindKey);
	if(tTempFindSection==1 && tTempFindKey==1)
	{
		return 2;
	}else if(tTempFindSection==1)
	{
		return 1;
	}

	return 0;
}

int XSectionGetValue(const char *pInputFileName, const char *pInputSectionName, const char *pInputKey, char *pOutputVal, int tInputDataLen)
{
	if(pInputFileName==NULL || pInputSectionName==NULL || pInputKey==NULL)
		return -1;

	if(access(pInputFileName, F_OK)!=0)
		return -2;

	FILE *pTempFile=NULL;
	pTempFile=fopen(pInputFileName, "r");
	if(pTempFile==NULL)
	{
		printf("Damon ==> open file[%s] failed !\n", pInputFileName);
		return -3;
	}

	int i=0;
	char *pTempLine=NULL;
	size_t tTempLen;
	size_t tTempReadLen;
	int tTempBreakLine=0;

	int tTempFindSection=0, tTempFindKey=0;
	
	while((tTempReadLen=getline(&pTempLine, &tTempLen, pTempFile))!=-1)
	{
		tTempBreakLine=0;
		for(i=0; i<tTempReadLen; i++)
		{
			if(pTempLine[i]<=0x20)
				continue;
			if(pTempLine[i]=='#')
				tTempBreakLine=1;
			break;
		}

		if(tTempBreakLine==0 && i<tTempReadLen)
		{
			char *pTempStartPt=strchr(pTempLine+i, '[');
			char *pTempEndPt=strchr(pTempLine+i, ']');
			if(pTempStartPt!=NULL && pTempEndPt!=NULL)
			{
				if(tTempFindSection==1)
				{
					break;
				}
			
				if(strncmp(pTempStartPt+1, pInputSectionName, strlen(pInputSectionName))==0)
				{
					tTempFindSection=1;
				}
			}else
			{
				if(tTempFindSection==1)
				{
					if(strncmp(pTempLine+i, pInputKey, strlen(pInputKey))==0)
					{
						tTempFindKey=1;
						break;
					}
				}
			}
		}else if(tTempBreakLine==1)
		{
			continue;
		}
	}

	int tTempRet=-4;
//	printf("Damon ==> section save val : %d %d \n", tTempFindSection, tTempFindKey);
	if(tTempFindSection==1 && tTempFindKey==1)
	{
		char *pTempStr=strstr(pTempLine, pInputKey);
		if(pTempStr!=NULL && pTempStr)
		{
			tTempLen=strlen(pTempLine);
			if(pTempStr[strlen(pInputKey)]=='=')
			{
				int j=0;
				for(i=strlen(pInputKey)+1; i<tTempLen; i++)
				{
					if(pTempLine[i]<0x20)
						break;
				
					if(j<tInputDataLen-1)
						pOutputVal[j++]=pTempLine[i];
					else
						break;
				}
				pOutputVal[j]=0;
				
				tTempRet=0;
			}
		}
	
	}

	if(pTempLine)
		free(pTempLine);

	fclose(pTempFile);

	return tTempRet;
}

int XSectionSaveValue(const char *pInputFileName, const char *pInputSectionName, const char *pInputKey, char *pInputVal)
{
	if(pInputFileName==NULL || pInputSectionName==NULL || pInputKey==NULL || pInputVal==NULL)
		return -1;

	if(strlen(pInputSectionName)<=0 || strlen(pInputKey)<=0)
		return -1;

	int tTempFindRet=0;
	if(access(pInputFileName, F_OK)==0)
	{
		tTempFindRet=XSectionFindKey(pInputFileName, pInputSectionName, pInputKey);
		if(tTempFindRet<0)
			return -2;

//printf("\n\nDamon ==> find key ret : %s %s  - %d \n", pInputSectionName, pInputKey, tTempFindRet);


		int tTempFd=open(pInputFileName, O_RDWR);
		if(tTempFd<0)
		{
			printf("Damon ==> open file[%s] failed !\n", pInputFileName);
			return -3;
		}

		int tTempFileLen=lseek(tTempFd, 0, SEEK_END);
		 if(tTempFileLen<0)
		 {
		 	printf("Damon ==> seek file failed ret= %d !\n", tTempFileLen);
			close(tTempFd);
			return -4;
		 }
		 
		if(lseek(tTempFd, 0, SEEK_SET)<0)
		{
		 	printf("Damon ==> seek file failed !\n");
			close(tTempFd);
			return -5;
		}

	//	printf("Damon ==> file len = %d \n", tTempFileLen);
	
		int tTempRet=0;
		char *pTempReadBuf=NULL;
		char *pTempWriteBuf=NULL;
		int tTempWriteLen=0;

		if(tTempFileLen>0)
		{
			pTempReadBuf=(char *)malloc(tTempFileLen+1);
			if(pTempReadBuf==NULL)
			{
				printf("Damon ==> malloc buf failed !\n");
				tTempRet=-6;
				goto __error;
			}
			memset(pTempReadBuf, 0, tTempFileLen+1);
		
			if(read(tTempFd, pTempReadBuf, tTempFileLen)!=tTempFileLen)
			{
				printf("Damon ==> read file failed !\n");
				tTempRet=-7;
				goto __error;
			}
	//	printf("Damon ==> read buf : %s %d\n", pTempReadBuf, tTempFileLen);

			int tTempMallocLen=0;
			tTempMallocLen=tTempFileLen+strlen(pInputSectionName)+strlen(pInputKey)+strlen(pInputVal)+20;
			pTempWriteBuf=(char *)malloc(tTempMallocLen);
			if(pTempWriteBuf==NULL)
			{
				printf("Damon ==> malloc buf failed !\n");
				tTempRet=-8;
				goto __error;
			}
			memset(pTempWriteBuf, 0, tTempMallocLen);
			
			if(tTempFindRet==2)
			{
				char *pTempPt=strstr(pTempReadBuf, pInputKey);
				if(pTempPt==NULL || pTempPt[strlen(pInputKey)]!='=')
				{
					printf("Damon ==> find key[%s] failed !\n", pInputKey);
					tTempRet=-9;
					goto __error;
				}

				pTempPt+=strlen(pInputKey)+1;
				tTempWriteLen=pTempPt-pTempReadBuf;
				memcpy(pTempWriteBuf, pTempReadBuf, tTempWriteLen);
				sprintf(pTempWriteBuf+tTempWriteLen, "%s", pInputVal);
				tTempWriteLen+=strlen(pInputVal);
				pTempWriteBuf[tTempWriteLen++]=0x0d;
				pTempWriteBuf[tTempWriteLen++]=0x0a;
				while(pTempPt && (pTempPt+1))
				{
					if(*pTempPt==0x0d && *(pTempPt+1)==0x0a)
					{
						pTempPt+=2;
						break;
					}else if(*pTempPt<0x20)
					{
						pTempPt+=1;
						break;
					}
					pTempPt++;
				}

				if(pTempPt!=NULL && strlen(pTempPt)>2)
				{
					memcpy(pTempWriteBuf+tTempWriteLen, pTempPt, strlen(pTempPt));
					tTempWriteLen += strlen(pTempPt);
				}
			}else if(tTempFindRet==1)
			{
				char *pTempPt=strstr(pTempReadBuf, pInputSectionName);
				if(pTempPt==NULL || pTempPt[strlen(pInputSectionName)]!=']')
				{
					printf("Damon ==> find key[%s] failed !\n", pInputKey);
					tTempRet=-9;
					goto __error;
				}

				int tTempEnterChar=0;
				int tTempFindNewLine=0;
				pTempPt+=strlen(pInputSectionName)+1;
				while(pTempPt)
				{
					if(*pTempPt==0x0a)
					{
						tTempEnterChar++;
						if(tTempEnterChar>=2)
						{
							if(*(pTempPt-1)==0x0d)
								pTempPt--;
							tTempFindNewLine=1;
							break;
						}
					}else if(*pTempPt>0x20)
					{
						tTempEnterChar=0;
					}else if(*pTempPt==0)
					{
						tTempFindNewLine=2;
						break;
					}

					pTempPt++;
				}

		//	printf("Damon ==> find new line = %d , [%s]\n", tTempFindNewLine, pTempPt);
				if(tTempFindNewLine>0)
				{
					if(pTempPt)
						tTempWriteLen=pTempPt-pTempReadBuf;
					else
						tTempWriteLen=tTempFileLen;
					int tTempLeaveLen=tTempFileLen-tTempWriteLen;
					memcpy(pTempWriteBuf, pTempReadBuf, tTempWriteLen);
					snprintf(pTempWriteBuf+tTempWriteLen, sizeof(pTempWriteBuf)-tTempWriteLen-1, "%s=%s", pInputKey, pInputVal);
					tTempWriteLen+=strlen(pInputKey)+strlen(pInputVal)+1;
					pTempWriteBuf[tTempWriteLen++]=0x0d;
					pTempWriteBuf[tTempWriteLen++]=0x0a;
					if(tTempLeaveLen>0)
					{
						memcpy(pTempWriteBuf+tTempWriteLen, pTempReadBuf+(tTempFileLen-tTempLeaveLen), tTempLeaveLen);
						tTempWriteLen+=tTempLeaveLen;
					}
				}
			}else if(tTempFindRet==0)
			{
				memcpy(pTempWriteBuf, pTempReadBuf, tTempFileLen);
				tTempWriteLen=tTempFileLen;
				pTempWriteBuf[tTempWriteLen++]=0x0d;
				pTempWriteBuf[tTempWriteLen++]=0x0a;
			}
		}else	// file is empty
		{
			int tTempMallocLen=strlen(pInputSectionName)+strlen(pInputKey)+strlen(pInputVal)+20;
			pTempWriteBuf=(char *)malloc(tTempMallocLen);
			if(pTempWriteBuf==NULL)
			{
				printf("Damon ==> malloc buf failed !\n");
				tTempRet=-10;
				goto __error;
			}
			memset(pTempWriteBuf, 0, tTempMallocLen);
			tTempWriteLen=0;
		}

		if(tTempFileLen==0 || tTempFindRet==0)
		{
			snprintf(pTempWriteBuf+tTempWriteLen, sizeof(pTempWriteBuf)-tTempWriteLen-1, "[%s]", pInputSectionName);
			tTempWriteLen+=strlen(pInputSectionName);
			tTempWriteLen+=2;
			pTempWriteBuf[tTempWriteLen++]=0x0d;
			pTempWriteBuf[tTempWriteLen++]=0x0a;

			snprintf(pTempWriteBuf+tTempWriteLen, sizeof(pTempWriteBuf)-tTempWriteLen-1, "%s=%s", pInputKey, pInputVal);
			tTempWriteLen += strlen(pInputKey)+1+strlen(pInputVal);
			pTempWriteBuf[tTempWriteLen++]=0x0d;
			pTempWriteBuf[tTempWriteLen++]=0x0a;
		}

		if(tTempWriteLen>0)
		{
			if(ftruncate(tTempFd, 0)!=0)
			{
				printf("Damon ==> truncate file failed !\n");
				tTempRet=-11;
				goto __error;
			}

			lseek(tTempFd, 0, SEEK_SET);

			if(write(tTempFd, pTempWriteBuf, tTempWriteLen)!=tTempWriteLen)
			{
				printf("Damon ==> write file failed !\n");
				tTempRet=-12;
				goto __error;
			}
		}else
		{
			tTempRet=-13;
		}

//	printf("Damon ==> write buf =%s %d \n", pTempWriteBuf, tTempWriteLen);
//	printf("damon ==> 3333333 \n");

		sync();
	
	__error:
		if(pTempReadBuf != NULL)
			free(pTempReadBuf);
		if(pTempWriteBuf != NULL)
			free(pTempWriteBuf);
		close(tTempFd);
//	printf("Damon ==> ret = %d \n", tTempRet);
		return tTempRet;
	}else
	{
		char pTempWriteBuf[1024]={0};
		int tTempWriteLen=0;
		int tTempFd=open(pInputFileName, O_RDWR | O_CREAT, 0666);
		if(tTempFd<0)
		{
			printf("Damon ==> create file[%s] failed !\n", pInputFileName);
			return -2;
		}

		lseek(tTempFd, 0, SEEK_SET);

		tTempWriteLen=strlen(pInputSectionName);
		snprintf(pTempWriteBuf, sizeof(pTempWriteBuf)-1, "[%s]", pInputSectionName);
		tTempWriteLen+=2;
		pTempWriteBuf[tTempWriteLen++]=0x0d;
		pTempWriteBuf[tTempWriteLen++]=0x0a;

		snprintf(pTempWriteBuf+tTempWriteLen, sizeof(pTempWriteBuf)-tTempWriteLen-1, "%s=%s", pInputKey, pInputVal);
		tTempWriteLen += strlen(pInputKey)+1+strlen(pInputVal);
		pTempWriteBuf[tTempWriteLen++]=0x0d;
		pTempWriteBuf[tTempWriteLen++]=0x0a;
		
//printf("Damon ==> write len : %d \n", tTempWriteLen);
		if(write(tTempFd, pTempWriteBuf, tTempWriteLen)!=tTempWriteLen)
		{
			printf("Damon ==> write section file failed  !\n");
			close(tTempFd);
			return -3;
		}
				
		close(tTempFd);
	}

	sync();

	return 0;
}


