#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "crptzip.h"

void print_usage()
{
	printf("Usage:\n");
	printf("./crptz E <New name for encrpted file> <Create Password> <file1> [file2..]\n");
	printf("./crptz D <Encrpted File Name> <Password> <output_folder/> <Optional file to decrpt only one file>\n");
	printf("./crptz list <Encrpted File Name>\n");
	printf("./crptz inspect <Encrpted File Name>\n");
	printf("./crptz delete <Encrpted File Name> <Password> <file>\n");
	exit(1);
}

int main(int argc,char* argv[])
{
	if(argc<3)
	{
		print_usage();
	}
	char* command=argv[1];
	if(strcmp(command,"E")==0)
	{
		if(argc<5)
			print_usage();
		int nofiles=argc-4;
		char* password=argv[3];
		char* archive_file=argv[2];
		for(int i=0;i<nofiles;i++)
		{
			if(strlen(argv[4+i])>255)
			{
				printf("Length of file name must be less than 256\n");
				return 1;
			}
		}
		//printf("%s %s %d %s\n",archive_file,password,nofiles,argv[4]);
		if(Encrypt_files(archive_file,password,&argv[4],nofiles)==1)
		{
			printf("Successfully Encrypted\n");
		}	
		else 
			printf("Encrption Failed\n");
	}
	else if(strcmp(command,"D")==0)
	{
		if(argc<4)
			print_usage();
		char* archive_file=argv[2];
		char* password=argv[3];
		if(strlen(argv[4])>254)
		{
			printf("output_folder filename length must be less than 255\n");
			return 1;
		}
		char output_folder[256];
		strcpy(output_folder,argv[4]);
		int l=strlen(output_folder);
		if(output_folder[l-1]!='/') 
		{
			output_folder[l]='/';
			output_folder[l+1]='\0';
		}

		if(argc==5)
		{
			if(Extract_files(archive_file,password,output_folder)==1)
				printf("Successfully Decrypted\n");
			else
				printf("Decryption Failed\n");
		}
		else 
		{
			char* filename=argv[5];
			if(Extract_one_file(archive_file,password,output_folder,filename)==1)
				printf("Successfully Decrypted\n");
			else
				printf("Decryption Failed\n");

		}
	}
	else if(strcmp(command,"list")==0)
	{
		list_files(argv[2]);
	}
	else if(strcmp(command,"inspect")==0)
	{
		Inspect_files(argv[2]);
	}
	else if(strcmp(command,"delete")==0)
	{
		if(argc<5)
			print_usage();
		char* archive_file=argv[2];
		char* password=argv[3];
		char* filename=argv[4];
		Remove_file(archive_file,password,filename);
	}
	else
		print_usage();
	return 0;
}
