#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>		// for pow

#define BINARY_LENGTH					8
#define SIX								6

char base64_table[64]={
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

int dec_to_binary(unsigned int num, unsigned int **output)
{
	int bytes=0;
	int i=0, j=0;
	int length=BINARY_LENGTH;
	int *temp_output=NULL;
	
	temp_output=(int*)calloc(length, sizeof(int));
	
	int remnant=0;
	while( num!=0 )
	{
		remnant=num%2;
		temp_output[bytes++]=remnant;
		if( bytes>length )
		{
			length*=2;
			temp_output=(int*)realloc(temp_output, sizeof(int)*length);
		}
		num/=2;
	}
	for( i=bytes; i<length; i++ )
	{
		temp_output[i]=0;
	}
	*output=(int*)calloc(length, sizeof(int));
	for( i=length-1, j=0; i>=0; i-- )
	{
		(*output)[j++]=temp_output[i];
	}
	
	return length;
}

int binary_to_dec(unsigned long num)
{
	int steps=0, remnant=0, index=0;
	int decimal=0;
	
	while( num!=0 )
	{
		remnant=num%10;
		num/=10;
		decimal+=remnant*pow(2, index);
		index++;
		steps++;
	}
	
	return decimal;
}

int text_to_binary(unsigned char *input, unsigned char **output, size_t length)
{
	int bytes=0, filled=0;
	int i=0, j=0;
	int buffer=0;
	
	unsigned int *temp_input=NULL;

	// wont work since image files have characters such as 0xff(end of file)
	// strlen stops counting when it encounters a end of file
//	buffer=strlen(input)*BINARY_LENGTH;
	buffer=length*BINARY_LENGTH;
	*output=(unsigned char*)malloc(sizeof(char)*buffer);
	for( i=0; i<length; i++ )
	{
		bytes=dec_to_binary(input[i], &temp_input);
		for( j=0; j<bytes; j++ )
		{
			(*output)[filled++]=temp_input[j]+'0';
		}
	}
	(*output)[filled]='\0';
	
	return filled;
}

// does conversion to 6-bit binary and also converts them to decimal
int base64_encode(unsigned char *input, unsigned char **output, size_t length)
{
	int bytes=0, filled=0;
	int i=0, j=0;
	int buffer=0, divided=0;
	char **temp_buf=NULL;
	int integered=0, *decimaled=NULL;				// array for all decimal values (dynamically created)
	int extra=0;
	unsigned char *temp_output=NULL;
	
	text_to_binary(input, &temp_output, length);
	buffer=strlen(temp_output);
	divided=buffer%SIX==0 ? buffer/SIX : (buffer/SIX)+1;
	
	temp_buf=(char**)malloc(sizeof(char*)*divided);
	
	// allocate data for temporary buffer
	for( i=0; i<divided; i++ )
	{
		*(temp_buf+i)=(char*)malloc(sizeof(char)*SIX);
	}
	
	// divide into 6 bit array
	for( i=0; i<divided; i++ )
	{
		for( j=0; j<SIX; j++ )
		{
			temp_buf[i][j]=temp_output[filled++];
			while( filled>buffer )
			{
				temp_buf[i][j]='0';
				extra++;
				buffer++;
			}
		}
	}
	
	decimaled=(int*)malloc(sizeof(int)*divided);
	
	// convert each 6 bit string to integer
	for( i=0; i<divided; i++ )
	{
		integered=atoi(temp_buf[i]);
		decimaled[i]=binary_to_dec(integered);
	}
	
	// alloc mem for output which is equal "divided" bytes
	// check for extra bytes added so as to add the "=" accordingly
	// if 2 bytes added then add one to "divided" and if 4 bytes added then add two to "divided", if no extra bytes then 0
	divided+=extra==2 ? 2 : (extra==4 ? 3 : 1);
	*output=(char*)malloc(sizeof(char)*divided);
	for( i=0, bytes=0; i<divided-(extra/2)-1; i++, bytes++ )
	{
		(*output)[bytes]=base64_table[decimaled[i]];
	}
	
	// fill in the equal sign(s) at the end
	if( extra>0 )
	{
		for( i=bytes; i<divided-1; i++, bytes++ )
		{
			(*output)[i]='=';
		}
	}
	(*output)[i]='\0';
	
	return bytes;
}

int main()
{
	FILE *fp=NULL;
	unsigned char *buffer=NULL, *output=NULL;
	size_t len=0, result=0;
	
	fp=fopen("small.gif", "rb");
	if( fp==NULL ) printf("Error opening file: %d\n", GetLastError());
	
	fseek(fp, 0, SEEK_END);
	len=ftell(fp);
	rewind(fp);
	
	buffer=(char*)malloc(sizeof(char)*(len+1));
	if( !buffer ) printf("No buffer alloc'd\n");
	
	result=fread(buffer, 1, len, fp);
	if( result!=len ) printf("Error reading file: %d\n", GetLastError());
	fclose(fp);
	
	base64_encode(buffer, &output, result);
	printf("%s", output);
	
	return 0;
}

