/*
last modify:
	2014/11/04   by duquan
	2015/10/23   by duquan, add define final or final and
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GROW_DIAG_FINAL_AND

#define MAX_LINE_LENGTH 102400

int main(int argc,char **argv)
{
	if (argc!=4)
	{
		printf("usage: e2c.A3.final c2e.A3.final Alignment.txt\n");
		return 1;
	}


	FILE *e2c=fopen(argv[1],"r");
	FILE *c2e=fopen(argv[2],"r");
	FILE *Alignment=fopen(argv[3],"w");

	if (e2c==NULL||c2e==NULL)
	{
		printf("input file error\n");
		return 1;
	}


	char LineNum[MAX_LINE_LENGTH+1];
	char Sentence[MAX_LINE_LENGTH+1];
	char Align[MAX_LINE_LENGTH+1];

	int CountLine=0;
	printf("Start combining e2c and c2e!!!\n");

	while (fgets(LineNum,MAX_LINE_LENGTH,c2e)!=NULL)
	{
		CountLine++;
		if (CountLine%10000==0)
		{
			printf("\r\t %d Lines processed!!",CountLine);
		}

		//get sentence source length and target length from first line.
		char *LinePointer=strstr(LineNum,"length");
		int SourceSentenceLength=0;           //english
		int TargetSentenceLength=0;			  //chinese
		LinePointer+=6;
		sscanf(LinePointer,"%d",&TargetSentenceLength);
		LinePointer=strstr(LinePointer,"length");
		LinePointer+=6;
		sscanf(LinePointer,"%d",&SourceSentenceLength);

		if (fgets(LineNum,MAX_LINE_LENGTH,e2c)==NULL)
		{
			printf("the two input line number is not the same!\ns");
			return 1;
		}
		//judge if e2c is equal to c2e
		LinePointer=strstr(LineNum,"length");
		int JudgeSourceSentenceLength=0;           //english
		int JudgeTargetSentenceLength=0;			  //chinese
		LinePointer+=6;
		sscanf(LinePointer,"%d",&JudgeSourceSentenceLength);
		LinePointer=strstr(LinePointer,"length");
		LinePointer+=6;
		sscanf(LinePointer,"%d",&JudgeTargetSentenceLength);

		if(TargetSentenceLength!=JudgeTargetSentenceLength || SourceSentenceLength != JudgeSourceSentenceLength )
		{
			if (fgets(Sentence,MAX_LINE_LENGTH,e2c)==NULL||fgets(Align,MAX_LINE_LENGTH,e2c)==NULL)
			{
				printf("target data format error!\n");
				return 1;
			}
			if (fgets(Sentence,MAX_LINE_LENGTH,c2e)==NULL||fgets(Align,MAX_LINE_LENGTH,c2e)==NULL)
			{
				printf("source input format error\n");
				return 1;
			}
			fprintf(Alignment,"\n");
			continue;
		}


		if (fgets(Sentence,MAX_LINE_LENGTH,c2e)==NULL||fgets(Align,MAX_LINE_LENGTH,c2e)==NULL)
		{
			printf("source input format error\n");
			return 1;
		}
		for( unsigned long i = strlen(Sentence)-1; i >= 0; i-- ) 
		{
			if( '\r' == Sentence[i] || '\n' == Sentence[i] )
			{
				Sentence[i]='\0';
			}
			else
			{
				break;
			}
		}

		//get and deal the third line.


		int *SourceArray=new int[SourceSentenceLength+1];
		int *TargetArray=new int[TargetSentenceLength+1];

		memset(SourceArray,0,SourceSentenceLength*sizeof(int));
		memset(TargetArray,0,TargetSentenceLength*sizeof(int));

		//Target

		char Word[5000];
		char *SentencePointer=Align;
		int AlignLength=strlen(Align);
		int CountProcess=0;
		int temp=0;

		for (int count=0;count<AlignLength-1;count++)
		{
			temp=0;
			if (Align[count]==')'&&count>0&&Align[count-1]=='}')
			{
				Align[count]='\0';
				memset(Word,'\0',5000*sizeof(char));
				sscanf(SentencePointer,"%s ({ %d %[ 0-9}]",Word,&temp,SentencePointer);

				if (strlen(Word)>100)
				{
					printf("Sentence pair %d exist a long word\n",CountLine);
				}

				if(strcmp(Word,"NULL")==0)
				{
					SentencePointer=Align+count+2;
					temp=0;
					continue;
				}

				if (temp==0)
				{
					SentencePointer=Align+count+2;
					CountProcess++;
					continue;
				}

				while(1)
				{
					if (temp>SourceSentenceLength)
					{
						printf("input source data exist errors!\n");
						return 1;
					}
					SourceArray[temp-1]=CountProcess+1;
					if (CountProcess>TargetSentenceLength)
					{
						printf("data error at line: %d!\n",CountLine);
						return 0;
					}
					if(strcmp(SentencePointer,"}")==0)
					{
						break;
					}
					temp=0;
					sscanf(SentencePointer,"%d %[ 0-9}]",&temp,SentencePointer);
				}

				SentencePointer=Align+count+2;
				CountProcess++;
			}
		}






		//Source


		if (fgets(Sentence,MAX_LINE_LENGTH,e2c)==NULL||fgets(Align,MAX_LINE_LENGTH,e2c)==NULL)
		{
			printf("target data format error!\n");
			return 1;
		}
		for( unsigned long i = strlen(Sentence)-1; i >= 0; i-- ) 
		{
			if( '\r' == Sentence[i] || '\n' == Sentence[i] )
			{
				Sentence[i]='\0';
			}
			else
			{
				break;
			}
		}
		//get and deal the third line.



		SentencePointer=Align;
		AlignLength=strlen(Align);
		CountProcess=0;

		for (int count=0;count<AlignLength-1;count++)
		{
			temp=0;
			if (Align[count]==')'&&Align[count-1]=='}')
			{
				Align[count]='\0';
				sscanf(SentencePointer,"%s ({ %d %[ 0-9}]",Word,&temp,SentencePointer);
				if (strlen(Word)>100)
				{
					printf("Sentence pair %d exist a long word\n",CountLine);
				}

				if(strcmp(Word,"NULL")==0)
				{
					SentencePointer=Align+count+2;
					temp=0;
					continue;
				}

				if (temp==0)
				{
					SentencePointer=Align+count+2;
					CountProcess++;
					continue;
				}

				while(1)
				{
					if (temp>TargetSentenceLength)
					{
						printf("input target data exist errors!\n");
						return 1;
					}
					TargetArray[temp-1]=CountProcess+1;
					if (CountProcess>SourceSentenceLength)
					{
						printf("data error at line: %d!\n",CountLine);
						return 0;
					}
					if(strcmp(SentencePointer,"}")==0)
					{
						break;
					}
					temp=0;
					sscanf(SentencePointer,"%d %[ 0-9}]",&temp,SentencePointer);
				}

				SentencePointer=Align+count+2;
				CountProcess++;
			}
		}




		//grow-diag-final

		int *Martix=new int[SourceSentenceLength*TargetSentenceLength+1];
		memset(Martix,0,SourceSentenceLength*TargetSentenceLength*sizeof(int));

		//store single alignment to Matrix
		for (int i=0;i<SourceSentenceLength;i++)
		{
			if (SourceArray[i]!=0)
			{
				Martix[TargetSentenceLength*i+SourceArray[i]-1]++;
			}

		}

		int *SourceSelected=new int[SourceSentenceLength+1];
		int *TargetSelected=new int[TargetSentenceLength+1];

		memset(SourceSelected,0,SourceSentenceLength*sizeof(int));
		memset(TargetSelected,0,TargetSentenceLength*sizeof(int));

		for (int i=0;i<TargetSentenceLength;i++)
		{
			if (TargetArray[i]!=0)
			{
				Martix[TargetSentenceLength*(TargetArray[i]-1)+i]++;
				if (Martix[TargetSentenceLength*(TargetArray[i]-1)+i]==2)
				{
					TargetSelected[i]=1;
					SourceSelected[TargetArray[i]-1]=1;
				}
			}

		}


		int IfNewPoint=1;
		while(IfNewPoint--)
		{
			for (int i=0;i<SourceSentenceLength*TargetSentenceLength;i++)
			{
				if (Martix[i]==2)
				{
					//grow

					//above
					if ((i-TargetSentenceLength)>=0&&Martix[i-TargetSentenceLength]==1)
					{
						if (SourceSelected[(i-TargetSentenceLength)/TargetSentenceLength]==0||TargetSelected[(i-TargetSentenceLength)%TargetSentenceLength]==0)
						{
							Martix[i-TargetSentenceLength]++;
							IfNewPoint=1;
							SourceSelected[(i-TargetSentenceLength)/TargetSentenceLength]=1;
							TargetSelected[(i-TargetSentenceLength)%TargetSentenceLength]=1;
						}
					}

					//before
					if ((i-1)>=0&&i%TargetSentenceLength!=0&&Martix[i-1]==1)
					{
						if (SourceSelected[(i-1)/TargetSentenceLength]==0||TargetSelected[(i-1)%TargetSentenceLength]==0)
						{
							Martix[i-1]++;
							IfNewPoint=1;
							SourceSelected[(i-1)/TargetSentenceLength]=1;
							TargetSelected[(i-1)%TargetSentenceLength]=1;
						}

					}

					//after
					if ((i+1)%TargetSentenceLength!=0&&(i+1)<(SourceSentenceLength*TargetSentenceLength)&&Martix[i+1]==1&&(i+1)/TargetSentenceLength<TargetSentenceLength&&(i+1)%TargetSentenceLength<TargetSentenceLength)
					{
						if (SourceSelected[(i+1)/TargetSentenceLength]==0||TargetSelected[(i+1)%TargetSentenceLength]==0)
						{
							Martix[i+1]++;
							IfNewPoint=1;
							SourceSelected[(i+1)/TargetSentenceLength]=1;
							TargetSelected[(i+1)%TargetSentenceLength]=1;
						}

					}

					//below
					if ((i+TargetSentenceLength)<SourceSentenceLength*TargetSentenceLength&&Martix[i+TargetSentenceLength]==1&&(i+TargetSentenceLength)/TargetSentenceLength<TargetSentenceLength&&(i+TargetSentenceLength)%TargetSentenceLength<TargetSentenceLength)
					{
						if (SourceSelected[(i+TargetSentenceLength)/TargetSentenceLength]==0||TargetSelected[(i+TargetSentenceLength)%TargetSentenceLength]==0)
						{
							Martix[i+TargetSentenceLength]++;
							IfNewPoint=1;
							SourceSelected[(i+TargetSentenceLength)/TargetSentenceLength]=1;
							TargetSelected[(i+TargetSentenceLength)%TargetSentenceLength]=1;
						}
					}





					//diag

					//left above
					if ((i-TargetSentenceLength-1)>=0&&i%TargetSentenceLength!=0&&Martix[i-TargetSentenceLength-1]==1)
					{
						if (SourceSelected[(i-TargetSentenceLength-1)/TargetSentenceLength]==0||TargetSelected[(i-TargetSentenceLength-1)%TargetSentenceLength]==0)
						{
							Martix[i-TargetSentenceLength-1]++;
							IfNewPoint=1;
							SourceSelected[(i-TargetSentenceLength-1)/TargetSentenceLength]=1;
							TargetSelected[(i-TargetSentenceLength-1)%TargetSentenceLength]=1;
						}
					}

					//right above
					if ((i-TargetSentenceLength+1)>=0&&(i+1)%TargetSentenceLength!=0&&Martix[i-TargetSentenceLength+1]==1)
					{
						if (SourceSelected[(i-TargetSentenceLength+1)/TargetSentenceLength]==0||TargetSelected[(i-TargetSentenceLength+1)%TargetSentenceLength]==0)
						{
							Martix[i-TargetSentenceLength+1]++;
							IfNewPoint=1;
							SourceSelected[(i-TargetSentenceLength+1)/TargetSentenceLength]=1;
							TargetSelected[(i-TargetSentenceLength+1)%TargetSentenceLength]=1;
						}
					}

					//left below
					if ((i+TargetSentenceLength-1)>=0&&(i+TargetSentenceLength-1)<SourceSentenceLength*TargetSentenceLength&&i%TargetSentenceLength!=0&&Martix[i+TargetSentenceLength-1]==1)
					{
						if (SourceSelected[(i+TargetSentenceLength-1)/TargetSentenceLength]==0||TargetSelected[(i+TargetSentenceLength-1)%TargetSentenceLength]==0)
						{
							Martix[i+TargetSentenceLength-1]++;
							IfNewPoint=1;
							SourceSelected[(i+TargetSentenceLength-1)/TargetSentenceLength]=1;
							TargetSelected[(i+TargetSentenceLength-1)%TargetSentenceLength]=1;
						}
					}

					//right below
					if ((i+TargetSentenceLength+1)<SourceSentenceLength*TargetSentenceLength&&(i+1)%TargetSentenceLength!=0&&Martix[i+TargetSentenceLength+1]==1)
					{
						if (SourceSelected[(i+TargetSentenceLength+1)/TargetSentenceLength]==0||TargetSelected[(i+TargetSentenceLength+1)%TargetSentenceLength]==0)
						{
							Martix[i+TargetSentenceLength+1]++;
							IfNewPoint=1;
							SourceSelected[(i+TargetSentenceLength+1)/TargetSentenceLength]=1;
							TargetSelected[(i+TargetSentenceLength+1)%TargetSentenceLength]=1;
						}
					}




				}
			}
		}

		//final 
		for (int i=0;i<SourceSentenceLength*TargetSentenceLength;i++)
		{
			if (Martix[i]==1)
			{
				//final Source
#ifndef GROW_DIAG_FINAL_AND
				if ((SourceSelected[i/TargetSentenceLength]==0||TargetSelected[i%TargetSentenceLength]==0)&&SourceArray[i/TargetSentenceLength]==(i%TargetSentenceLength+1))
#else
				if ((SourceSelected[i/TargetSentenceLength]==0&&TargetSelected[i%TargetSentenceLength]==0)&&SourceArray[i/TargetSentenceLength]==(i%TargetSentenceLength+1))
#endif
				{
					Martix[i]++;
					SourceSelected[i/TargetSentenceLength]=1;
					TargetSelected[i%TargetSentenceLength]=1;
				}

			}
		}

		for (int i=0;i<SourceSentenceLength*TargetSentenceLength;i++)
		{
			if (Martix[i]==1)
			{
				//final Target
#ifndef GROW_DIAG_FINAL_AND
				if ((SourceSelected[i/TargetSentenceLength]==0||TargetSelected[i%TargetSentenceLength]==0)&&TargetArray[i%TargetSentenceLength]==(i/TargetSentenceLength+1))
#else
				if ((SourceSelected[i/TargetSentenceLength]==0&&TargetSelected[i%TargetSentenceLength]==0)&&TargetArray[i%TargetSentenceLength]==(i/TargetSentenceLength+1))
#endif
				{
					Martix[i]++;
					SourceSelected[i/TargetSentenceLength]=1;
					TargetSelected[i%TargetSentenceLength]=1;
				}

			}
		}

		bool IfFirstPair=1;
		for (int i=0;i<SourceSentenceLength*TargetSentenceLength;i++)
		{
			if (Martix[i]==2)
			{
				if (IfFirstPair)
				{
					fprintf(Alignment,"%d-%d",i%TargetSentenceLength,i/TargetSentenceLength);
					IfFirstPair=0;
				}
				else
				{
					fprintf(Alignment," %d-%d",i%TargetSentenceLength,i/TargetSentenceLength);
				}

			}
		}
		fprintf(Alignment,"\n");



		delete(SourceArray);
		delete(TargetArray);
		delete(SourceSelected);
		delete[] TargetSelected;
		delete(Martix);

	}

	printf("\nTotal Line: %d \n",CountLine);

	fclose(c2e);
	fclose(e2c);
	fclose(Alignment);

	return 0;
}

