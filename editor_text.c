/*
 * Authors : Demba Cisse
 *			Harry Jandu
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#define BUFFER_ALLOC			8
#define SAVE_FILE				"$--save"

void die_error(const char *err_msg)
{
	perror(err_msg);
	
	exit(-1);
}

void free_memory(void **mem)
{
	if( (*mem) )
	{
		free(*mem);
		*mem = NULL;
	}
	
	return;
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

int get_delimited_input(char **data, int *data_len, int delim, FILE *fp)
{
	int bytes_written = 0, bytes_read = 0, resize = BUFFER_ALLOC, flag = 1;
	
	char *buffer = *data, *temp = NULL;
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
			temp = (char*)realloc((buffer), resize);
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

char *random_name()
{
	char *rname = (char*)calloc(BUFFER_ALLOC + 1, sizeof(char));
	int i = 0;
	
	for( i = 0; i < BUFFER_ALLOC; ++i )
	{
		rname[i] = (char)((rand() % 26) + 65);
	}
	
	return rname;
}

int affiche_file_content(FILE *fp, int *line_numbers)
{
	int init_buff = BUFFER_ALLOC;
	char *buf = NULL;
	
	int bytes_read = 0, total = 0;
	
	if( fp != NULL )
	{
		if( line_numbers )
		{
			*line_numbers = 1;
		}
		while( ( bytes_read = get_delimited_input(&buf, 0, (int)'\n', fp) ) > 0 )
		{
			total += bytes_read;
			
			if( line_numbers )
			{
				printf("%d ] ", *line_numbers);
				*line_numbers = *line_numbers + 1;
			}
			
			fprintf(stdout, "%s", buf);
			
			free_memory((void**)&buf);
		}
		
		if( buf )
		{
			free_memory((void**)&buf);
		}
		
		fclose(fp);
	}
		
	return total;
}

int libre_editor(FILE *fp)
{
	int init_buff = BUFFER_ALLOC;
	char *buf = NULL, *content = calloc(init_buff, sizeof(char));
	char save[30] = {0}, flag = 1;
	
	int bytes_read = 0, total = 0, line_numbers = 1;
	
	if( fp == NULL )
	{
		fp = stdin;
	}
	else
	{
		affiche_file_content(fp, &line_numbers);
	}
	
	do
	{
		bytes_read = get_delimited_input(&buf, 0, (int)'\n', stdin);
		total += bytes_read;
		
		content = (char*)realloc(content, sizeof(char)*(total + 1));
		strncat(content, buf, bytes_read);
		content[total] = '\0';
		
		printf("%d ] ", line_numbers++);
		
		fprintf(stdout, "%s", buf);
		
		if( bytes_read >= 8 )
		{
			memset(save, 0, sizeof(save));
			strncpy(save, buf, 29);
			if( strncmp(save, SAVE_FILE, 7) == 0 )
			{
				FILE *save_file = NULL;
				flag = 0;
				
				if( strlen(save) < 9 )
				{
					char *fname = random_name();
					save_file = fopen(fname, "a+");
					total = total - strlen(fname);
					
					fprintf(stdout, "saving to %s\n", fname);
				}
				else
				{
					save[strlen(save)-1] = '\0';
					save_file = fopen(save + 8, "a+");
					total = total - strlen(save) - 1;		// pour le '\n' on fait -1
					
					fprintf(stdout, "saving to %s", (save + 8));
				}
				
				fwrite(content, sizeof(char), total, save_file);
				
				fclose(save_file);
			}
		}
		
		free_memory((void**)&buf);
	} while( bytes_read > 0 && flag != 0 );
	
	printf("\n");
	
	free_memory((void**)&content);
	
	return total;
}

int delete_line(FILE *fp, int row)
{
	int success = 0;
	if( row < 1 )
	{
		fprintf(stderr, "row or column can't be less than 1\n");
		exit(-1);
	}
	
	int bytes_read = 0, line_num = 1, line_found = 0, total = 0;
	char *buf = NULL;
	
	if( fp != NULL )
	{
		do
		{
			bytes_read = get_delimited_input(&buf, 0, (int)'\n', fp);
			total += bytes_read;
			
			if( line_num == row )
			{
				line_found = 1;
				break;
			}
			line_num++;
			
			free_memory((void**)&buf);
		}while( bytes_read > 0 );
		
		if( buf && line_found )
		{
			fprintf(stderr, "%d ] %s", line_num, buf);
			char c = '\x8';
			
			total = total - strlen(buf);
			fseek(fp, total, 0);
			
			int i = 0;
			for( i = 0; i < strlen(buf); ++i )
			{
				fwrite(&c, sizeof(char), 1, fp);
			}
			
			free_memory((void**)&buf);
		}
		else
		{
			fprintf(stderr, "line not found\n");
		}
		
		fclose(fp);
	}
	
	return success;
}

int modify_char(FILE *fp, int row, int col)
{
	int success = 0;
	if( row < 1 || col < 1 )
	{
		fprintf(stderr, "row or column can't be less than 1\n");
		exit(-1);
	}
	
	int bytes_read = 0, line_num = 1, line_found = 0, total = 0;
	char *buf = NULL;
	
	if( fp != NULL )
	{
		do
		{
			bytes_read = get_delimited_input(&buf, 0, (int)'\n', fp);
			total += bytes_read;
			
			if( line_num == row )
			{
				line_found = 1;
				break;
			}
			line_num++;
			
			free_memory((void**)&buf);
		}while( bytes_read > 0 );
		
		if( buf && line_found )
		{
			fprintf(stderr, "%d ] %s", line_num, buf);
			
			total = total - strlen(buf);
			fseek(fp, total, 0);
			
			int i = 0;
			for( i = 0; i < strlen(buf); ++i )
			{
				if( i == ( col - 1 ) )
				{
					total = total + i;
					fseek(fp, total, SEEK_SET);
					
					int input_len = BUFFER_ALLOC - 1;
					char c;
					char *input = (char*)calloc(input_len + 1, sizeof(char));
					
					fread(&c, sizeof(char), 1, fp);
					fseek(fp, total, SEEK_SET);
					
					fprintf(stderr, "enter the character you would like to replace `%c` with : ", c);
					get_delimited_input(&input, &input_len, (int)'\n', stdin);
					
					fprintf(stderr, "you've entered %c\n", input[0]);
					
					fwrite(&input[0], sizeof(char), 1, fp);
					free_memory((void**)&input);
					
					success = 1;
					
					break;
				}
			}
			if( !success )
			{
				fprintf(stderr, "column not found\n");
			}
			
			free_memory((void**)&buf);
		}
		else
		{
			fprintf(stderr, "line not found\n");
		}
		
		fclose(fp);
	}
	
	return success;
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	
	int index;
	
	// -a => affiche contenu
	// -e => libre_edit
	// -m => modifier
	// -f => fichier a modifier
	// -l => ligne a modifier
	// -c => colonne a modifier
	// -d => delete line
	char *editor_options = "aemf:l:c:d";
	char gc;
	
	// flags pour les arguments passee au parametres comme -f -m
	int free_edit = 0;
	int modify_flag = 0, delete_flag = 0, affiche_flag = 0;
	char *file_arg = NULL, *row_arg = NULL, *column_arg = NULL;
	
	while( ( gc	= getopt(argc, argv, editor_options) ) != -1 )
	{
		switch( gc )
		{
			case 'a':
				affiche_flag = 1;
				break;
			case 'e':
				free_edit = 1;
				break;
			case 'd':
				delete_flag = 1;
				break;
			case 'm':
				modify_flag = 1;
				break;
			case 'f':
				file_arg = optarg;
				break;
			case 'l':
				row_arg = optarg;
				break;
			case 'c':
				column_arg = optarg;
				break;
			case '?':
				if( optopt == 'f' || optopt == 'l' || optopt == 'c' )
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
	
	if( affiche_flag == 1 )
	{
		FILE *fp = NULL;
		if( file_arg != NULL )
		{
			fp = fopen(file_arg, "r");
			if( fp == NULL )
			{
				fprintf(stderr, "%s file not found\n", file_arg);
			}
		}
		else
		{
			fprintf(stderr, "-a requires file, use -f operator to specify file\n");
			exit(-1);
		}
			
		affiche_file_content(fp, NULL);
		
		exit(0);
	}
	
	if( free_edit == 1 )
	{
		fprintf(stderr, "WELCOME TO LIBRE EDIT - $--save [filename.txt] to save file\n");
		
		FILE *fp = NULL;
		if( file_arg != NULL )
		{
			fp = fopen(file_arg, "r");
			if( fp == NULL )
			{
				fprintf(stderr, "%s file not found\n", file_arg);
			}
		}
		
		long int ecrit = libre_editor(fp);
		
		fprintf(stderr, "sauvegardee %lu bytes de donnees\n", ecrit);
		
		exit(0);
	}
	
	if( delete_flag == 1 )
	{
		FILE *fp = NULL;
		int row = 1;
		
		if( file_arg == NULL )
		{
			fprintf(stderr, "-m requires file, use -f operator to specify file\n");
			exit(-1);
		}
		
		fp = fopen(file_arg, "r");
		if( fp == NULL )
		{
			die_error("main modify_flag");
		}
		fclose(fp);
		
		fp = fopen(file_arg, "r+");
		if( fp == NULL )
		{
			die_error("main modify flag");
		}
		
		if( row_arg != NULL )
		{
			string_to_int(row_arg, &row);
		}
		
		delete_line(fp, row);
		
		exit(0);
	}
	
	if( modify_flag == 1 )
	{
		FILE *fp = NULL;
		int row = 1, column = 1;
		
		if( file_arg == NULL )
		{
			fprintf(stderr, "-m requires file, use -f operator to specify file\n");
			exit(-1);
		}
		
		fp = fopen(file_arg, "r");
		if( fp == NULL )
		{
			die_error("main modify_flag");
		}
		fclose(fp);
		
		fp = fopen(file_arg, "r+");
		if( fp == NULL )
		{
			die_error("main modify flag");
		}
		
		if( row_arg != NULL )
		{
			string_to_int(row_arg, &row);
		}
		
		if( column_arg != NULL )
		{
			string_to_int(column_arg, &column);
		}
		
		printf("file = %s, row = %d, column = %d\n", file_arg, row, column);
		
		modify_char(fp, row, column);
		
		exit(0);
	}
	
	return 0;
}

