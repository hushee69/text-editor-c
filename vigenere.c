/*
 * Authors : Demba Cisse
 *			Harry Jandu
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#define IS_SPACE_CHAR(s) ( s == ' ' || (int)s == 32 )
#define IS_ALPHA(s) ( ( s >= 97 && s <= 122 ) || ( s >= 65 && s <= 90 ) )
#define IS_LOWER_CASE(s) ( s >= 97 && s <= 122)
#define TO_UPPER_CASE(s) (char)((int)s - 32)

#define MOD_Z						155
#define SUB_A						65

#define BUFFER_ALLOC				8

enum vigenere_options
{
	DECIPHER = 0, CIPHER
};

void die_error(const char *err_msg)
{
	perror(err_msg);
	
	exit(-1);
}

int string_to_int(char *str, int *converted)
{
	int success = 0, temp = 0;
	
	if( *str >= '0' && *str <= '9' )
	{
		*converted = 0;
		
		success = 1;
		do
		{
			temp = (*str) - '0';
			*converted = ((*converted) * 10) + temp;
			*str++;
		}while( *str >= '0' && *str <= '9' );
	}
	else
	{
		success = 0;
	}
	
	return success;
}

void string_to_upper(char *str, size_t len)
{
	int k = 0;
	size_t k_len = len;
	
	for( k = 0; k < k_len; ++k )
	{
		if( !IS_ALPHA(str[k]) )
		{
			die_error("key must be alphabets only");
		}
		if( IS_LOWER_CASE(str[k]) )
		{
			str[k] = TO_UPPER_CASE(str[k]);
		}
	}
	
	return;
}

void v_cipher(char *plain_text, char *key, char **cipher)
{
	int pt = 0, cp = 0, k = 0;				// plain_text, cipher index
	int p_text_len = strlen(plain_text);
	int k_len = strlen(key);
	int ciphered_dec = 0;
	*cipher = (char*)malloc(sizeof(char)*(p_text_len+1));

	for( pt = 0; pt < p_text_len; ++pt )
	{
		if( !IS_SPACE_CHAR(plain_text[pt]) && IS_ALPHA(plain_text[pt]) )
		{
			if( k >= k_len )
			{
				k=0;
			}
			if( IS_LOWER_CASE(plain_text[pt]) )
			{
				plain_text[pt] = TO_UPPER_CASE(plain_text[pt]);
			}
			ciphered_dec = (int)plain_text[pt] + (int)key[k];
			if( ciphered_dec > MOD_Z )
			{
				ciphered_dec = (ciphered_dec - 1) % MOD_Z;
				(*cipher)[cp] = (char)(SUB_A+ciphered_dec);
				cp++;
			}
			else if( ciphered_dec == MOD_Z )
			{
				(*cipher)[cp] = (char)(ciphered_dec - SUB_A);
				cp++;
			}
			else
			{
				ciphered_dec = ciphered_dec % MOD_Z;
				(*cipher)[cp] = (char)(ciphered_dec - SUB_A);
				cp++;
			}
			k++;
		}
		else
		{
			(*cipher)[cp] = plain_text[pt];;
			cp++;
		}
	}
	(*cipher)[cp]='\0';

	return;
}

void v_decipher(char *ciphered_text, char *key, char **decipher)
{
	int ct = 0, dp = 0, k = 0;				// ciphered_text, decipher
	int c_text_len = strlen(ciphered_text);
	int k_len = strlen(key);
	int deciphered_dec = 0;
	*decipher=(char*)malloc(sizeof(char)*(c_text_len+1));

	for( ct = 0; ct < c_text_len; ++ct )
	{
		if( !IS_SPACE_CHAR(ciphered_text[ct]) && IS_ALPHA(ciphered_text[ct]) )
		{
			if( k >= k_len )
			{
				k = 0;
			}
			deciphered_dec = (int)ciphered_text[ct] - (int)key[k];
			if( deciphered_dec < 0 )
			{
				deciphered_dec = MOD_Z + (deciphered_dec + 1);
				(*decipher)[dp] = (char)(deciphered_dec - SUB_A);
				dp++;
			}
			else if( deciphered_dec == 0 )
			{
				(*decipher)[dp] = (char)(deciphered_dec + SUB_A);
				dp++;
			}
			else
			{
				deciphered_dec = deciphered_dec % MOD_Z;
				(*decipher)[dp] = (char)(deciphered_dec + SUB_A);
				dp++;
			}
			k++;
		}
		else
		{
			(*decipher)[dp] = ciphered_text[ct];
			dp++;
		}
	}
	(*decipher)[dp]='\0';

	return;
}

int get_delimited_input(char **data, int *data_len, int delim, FILE *fp)
{
	int bytes_written = 0, bytes_read = 0, resize = BUFFER_ALLOC, flag = 1;
	
	char *buffer = *data;
	char c;
	
	if( ( buffer == NULL ) && ( data_len == NULL ) )
	{
		flag = 0;
		buffer = (char*)malloc(sizeof(char)*BUFFER_ALLOC);
	}
	
	do
	{
		if( bytes_written >= ( resize - 1) )
		{
			resize = (resize + bytes_written + BUFFER_ALLOC);
			//printf("resizing with %d bytes\n", resize);
			char *temp = (char*)realloc((buffer), resize);
			if( temp == NULL )
			{
				die_error("get_input realloc");
			}
			buffer = temp;
		}
		
		if( flag == 1 && bytes_written >= (*data_len - 1) )
		{
			break;
		}
		
		bytes_read = fread(&c, sizeof(char), 1, fp);
		buffer[bytes_written] = c;
		
		bytes_written = bytes_written + bytes_read;
	}while( (int)c != delim && (int)c != EOF && bytes_read > 0 );
	buffer[bytes_written] = '\0';
	
	*data = buffer;
	
	return bytes_written;
}

void free_memory(void **mem)
{
	if( (*mem) != NULL )
	{
		free(*mem);
		*mem = NULL;
	}
	
	return;
}

int main(int argc, char **argv)
{
	// -c => crypter
	// -d => decrypter
	// -k => fichier avec cle
	// -f => fichier a crypter / decrypter
	char *editor_options = "cdk:f:";
	char gc;
	
	// flags pour les arguments passee au parametres comme -f -m
	int crypt = 0, decrypt = 0;
	char *file_arg = NULL, *key_arg = NULL;
	
	while( ( gc	= getopt(argc, argv, editor_options) ) != -1 )
	{
		switch( gc )
		{
			case 'c':
				crypt = 1;
				break;
			case 'd':
				decrypt = 1;
				break;
			case 'k':
				key_arg = optarg;
				break;
			case 'f':
				file_arg = optarg;
				break;
			case '?':
				if( optopt == 'f' || optopt == 'k' )
					fprintf(stderr, "option -%c requires an argument\n", optopt);
				else if( isprint(optopt) )
					fprintf(stderr, "unknown option `-%c`\n", optopt);
				else
					fprintf(stderr, "unknown option character `\\x%x`\n", optopt);
				exit(-1);
			default:
				abort();
		}
	}
	
	if( crypt == 1 )
	{
		clock_t start, end;
		double time_taken;
		
		FILE *key = NULL, *file_path = NULL;
		
		char *key_string = NULL;
		size_t key_len = 0;
		
		if( !key_arg )
		{
			fprintf(stderr, "key file has to be specified\n");
			exit(-1);
		}
		key = fopen(key_arg, "r");
		if( !key )
		{
			die_error("fopen");
		}
		
		start = clock();
		
		get_delimited_input(&key_string, 0, (int)'\n', key);
		key_len = strlen(key_string);
		key_string[key_len - 1] = '\0';
		string_to_upper(key_string, key_len - 1);
		
		if( !file_arg )
		{
			file_path = stdin;
		}
		else
		{
			file_path = fopen(file_arg, "r");
			if( file_path == NULL )
			{
				die_error("fopen");
			}
		}
		
		int bytes_read = 0, total = 0;
		char *plain_text = NULL, *ciphered = NULL;
		do
		{
			bytes_read = get_delimited_input(&plain_text, 0, (int)'\n', stdin);
			total += bytes_read;
			
			v_cipher(plain_text, key_string, &ciphered);
			
			fprintf(stdout, "%s", ciphered);
			
			free_memory((void**)&plain_text);
			free_memory((void**)&ciphered);
		}while( bytes_read > 0 );
		
		end = clock();
		
		time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
		
		fprintf(stderr, "time_taken by program to cipher %d bytes is %lfs\n", total, time_taken);
		
		fclose(key);
		fclose(file_path);
		free_memory((void**)&key_string);
		
		exit(0);
	}
	
	if( decrypt == 1 )
	{
		clock_t start, end;
		double time_taken;
		
		FILE *key = NULL, *file_path = NULL;
		
		char *key_string = NULL;
		size_t key_len = 0;
		
		if( !key_arg )
		{
			fprintf(stderr, "key file has to be specified\n");
			exit(-1);
		}
		
		key = fopen(key_arg, "r");
		if( !key )
		{
			die_error("fopen");
		}
		
		start = clock();
		
		get_delimited_input(&key_string, 0, (int)'\n', key);
		key_len = strlen(key_string);
		key_string[key_len - 1] = '\0';
		string_to_upper(key_string, key_len - 1);
		
		if( !file_arg )
		{
			file_path = stdin;
		}
		else
		{
			file_path = fopen(file_arg, "r");
			if( file_path == NULL )
			{
				die_error("fopen");
			}
		}
		
		int bytes_read = 0, total = 0;
		char *ciphered_text = NULL, *deciphered = NULL;
		do
		{
			bytes_read = get_delimited_input(&ciphered_text, 0, (int)'\n', stdin);
			total += bytes_read;
			
			v_decipher(ciphered_text, key_string, &deciphered);
			
			fprintf(stdout, "%s", deciphered);
			
			free_memory((void**)&ciphered_text);
			free_memory((void**)&deciphered);
		}while( bytes_read > 0 );
		
		end = clock();
		
		time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
		
		fprintf(stderr, "time_taken by program to decipher %d bytes is %lfs\n", total, time_taken);
		
		fclose(key);
		fclose(file_path);
		free_memory((void**)&key_string);
		
		exit(0);
	}
	
	return 0;
}
