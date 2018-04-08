#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFFER_ALLOC			120

char *random_name()
{
	char *rname = (char*)calloc(BUFFER_ALLOC + 1, sizeof(char));
	int i = 0;
	
	for( i = 0; i < BUFFER_ALLOC - 1; ++i )
	{
		rname[i] = (char)((rand() % 26) + 65);
	}
	rname[i] = '\n';
	
	return rname;
}

void write_to_file(FILE *fp)
{
	long i = 0;
	char *text = NULL;
	
	for( i = 0; i < 100000; ++i )
	{
		text = random_name();
		fwrite(text, strlen(text), sizeof(char), fp);
	}
	
	return;
}

int main()
{
	srand(time(NULL));
	
	FILE *fp = fopen("smallfile.txt", "a+");
	
	write_to_file(fp);
	
	fclose(fp);
	
	return 0;
}

