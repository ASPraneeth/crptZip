#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "crptzip.h"

filenode* create_file_node(const char* filename)
{
	FILE *fp=fopen(filename,"rb");
	if(fp==NULL)
	{
		printf("Error opening the file %s\n",filename);
		return NULL;
	}
	//**Create New Node
	filenode *new=malloc(sizeof(filenode));
	new->next=NULL;
	new->is_compressed=0;
	strcpy(new->filename,filename);
	new->length=strlen(filename);
	//**Copy data into filenode
	fseek(fp,0,SEEK_END)!=0; //--------** To get the size of data
	new->original_size=ftell(fp);
	fseek(fp,0,SEEK_SET)!=0;
	new->data =(unsigned char *) malloc(new->original_size);
	fread(new->data,1,new->original_size,fp);//-------* Read data
	fclose(fp);
	return new;
}

void append_file(filenode** head, filenode * node)
{
	filenode *temp=*head;
	if(temp==NULL)
	{
		*head=node;
		return;
	}
	else
	{
		while(temp->next!=NULL)
		{
			temp=temp->next;
		}
		temp->next=node;
		node->next=NULL;
		return;
	}
}

filenode* find_file(filenode *head, const char* filename)
{
	filenode * temp=head;
	while(temp!=NULL)
	{
		if(strcmp(temp->filename,filename)==0)
		{
			return temp;
		}
		temp=temp->next;
	}
	return NULL;
}

void free_filenode(filenode * head)
{
	filenode* temp=head;
	while(temp!=NULL)
	{
		filenode* flag=temp->next;
		free(temp->data);
		free(temp);
		temp=flag;
	}
	return;
}

indexnode* create_index_node(const char* filename, long orig, long comp, int is_comp,long offset)
{
	//**Create new indexnode
	indexnode* new=malloc(sizeof(indexnode));
	strcpy(new->filename,filename);
	new->original_size=orig;
	new->compressed_size=comp;
	new->is_compressed=is_comp;
	new->offset=offset;
	new->prev=NULL;
	new->next=NULL;
	return new;
}

void append_index(indexnode** head, indexnode* node)
{
	indexnode* temp=*head;
	if(temp==NULL)
	{
		*head=node;
		return;
	}
	else
	{
		while(temp->next!=NULL)
			temp=temp->next;
		temp->next=node;
		node->prev=temp;
		node->next=NULL;
	}
	return;
}

indexnode* find_index(indexnode* head, const char* filename)
{
	indexnode* temp=head;
	while(temp!=NULL)
	{
		if(strcmp(filename, temp->filename)==0)
		{
			return temp;
		}
		temp=temp->next;
	}
	return NULL;
}

void index_print(indexnode* head)
{
	indexnode* temp=head;
	int nofiles=0;
	long orgsize=0;
	long storedsize=0;
	printf("   %-40sOrig Size\t       Stored Size         Ratio\tCompress\n","Filename");
	int i=0;
	while(temp!=NULL)
	{
		float ratio;

		if(temp->original_size > 0)
			ratio=((temp->original_size  - temp->compressed_size )*1.0)/temp->original_size;
		else 
			ratio=0;

		ratio*=100;
		char flag[5];
		if(temp->is_compressed==1)
			strcpy(flag,"Yes");
		else 
			strcpy(flag,"No");
		printf("%-2d %-40s%-12ldbytes   %-12ldbytes   %.2f\%% saved\t%s\n",i+1,temp->filename,temp->original_size,temp->compressed_size,ratio,flag);
		nofiles++;
		orgsize+=temp->original_size;
		storedsize+=temp->compressed_size;
		temp=temp->next;
		i++;
	}
	printf("----------------------------------------------------------------------\n");
	printf("Total: %d file(s) | %ld bytes original | %ld bytes stored\n",nofiles,orgsize,storedsize);
	printf("----------------------------------------------------------------------\n");

	return;
}

void free_indexnode(indexnode* head)
{
	indexnode* temp=head;
	while(temp!=NULL)
	{
		indexnode* flag=temp->next;
		free(temp);
		temp=flag;
	}
	return;
}

unsigned char* compress_data(unsigned char* data, long original_size, long* compressed_size)
{
	//***Example : aaaa is compressed to 4a  abcd is compressed into 1a1b1c1d
	unsigned char* out=malloc(2*(original_size));
	long size=0;
	long count=1;
	unsigned char prev;
	for(long i=0;i<original_size;i++)
	{
		if(i==0)
		{
			prev=data[i];
			continue;
		}
		if(prev==data[i] && count<255)
			count++;
		else
		{
			out[size++]=count;
			out[size++]=prev;
			prev=data[i];
			count=1;
		}
	}
	out[size++]=count;
	out[size++]=prev;
	//Copy the data into another malloc (Smaller)
	unsigned char* compressed_data=malloc(size);
	memcpy(compressed_data,out,size);
	free(out);
	*compressed_size=size;
	return compressed_data;
}

unsigned char* decompress_data(const unsigned char* data, long compressed_size,long original_size)
{
	unsigned char* org=malloc(original_size);
	long size=0;
	for(long i=0;i<compressed_size-1;i+=2)
	{
		for(long j=0;j<data[i];j++)
		{
			org[size++]=data[i+1];
		}
	}
	return org;
}

void encrpt_data(unsigned char* data, long size, const char* password)
{
	for(long j=0;j<size;j++)
	{
		int length=strlen(password);
		for(int i=0;i<length;i++)
		{
			data[j]=data[j]^ password[i];
		}
		data[j]=(data[j] << 2) | (data[j] >> 6);
	}
	return;
}

void decrpt_data(unsigned char* data, long size, const char * password)
{
	for(long j=0;j<size;j++)
	{
		int length=strlen(password);
		data[j]=(data[j] >> 2) | (data[j] << 6);
		for(int i=0;i<length;i++)
		{
			data[j]=data[j] ^ password[i];
		}
	}
	return;
}

unsigned int hash_password(const char* password)
{
	unsigned int hash=5381;
	int length=strlen(password);
	for(int i=0;i<length;i++)
	{
		hash=((hash << 5) + hash) + password[i];
	}
	return hash;
}

int validate_password(const char* password)
{
	int length=strlen(password);
	if(length<6)
		return 0;
	int upper=0;
	int digit=0;
	for(int i=0;i<length;i++)
	{
		if(upper==1 && digit ==1)
		{
			break;
		}
		if(password[i] >= 'A' && password[i] <= 'Z')
			upper=1;
		else if(password[i] >= '0' && password[i] <= '9')
			digit=1;
	}
	if(upper==1 && digit ==1)
		return 1;
	return 0;
}

unsigned int compute_checksum(const unsigned char* data, long size)
{
	unsigned int checksum=0;
	for(long i=0;i<size;i++)
	{
		checksum=checksum ^ data[i];
	}
	//since checksum is unsigned int size is 32;
	checksum= (checksum << 1) | (checksum >> 31);
	return checksum;
}

int verify_checksum(const unsigned char* data, long size, unsigned int expected)
{
	unsigned int actual=compute_checksum(data,size);
	if(actual==expected)
		return 1;
	else
		return 0;
}

indexnode* write_into_archive(const char* archive_file,filenode* file_header,archiveheader header)
{
	FILE *fp=fopen(archive_file,"wb");
	if(fp==NULL)
	{
		printf("Error opening file %s\n",archive_file);
		free_filenode(file_header);
		exit(1);
	}
	fwrite(&header,sizeof(archiveheader),1,fp);
	filenode *temp=file_header;
	indexnode* index_header=NULL;
	for(int i=0;temp!=NULL;i++)
	{
		fwrite(&(temp->length),sizeof(int),1,fp);
		fwrite(temp->filename,1,temp->length,fp);
		fwrite(&(temp->original_size),sizeof(long),1,fp);
		fwrite(&(temp->compressed_size),sizeof(long),1,fp);
		fwrite(&(temp->is_compressed),sizeof(int),1,fp);
		long offset=ftell(fp);
		offset+=sizeof(long);
		fwrite(&(offset),sizeof(long),1,fp);
		fwrite(temp->data,1,temp->compressed_size,fp);

		//copying details into indexnode
		indexnode* file_index= create_index_node(temp->filename,temp->original_size,temp->compressed_size,temp->is_compressed,offset);
		append_index(&index_header,file_index);
		temp=temp->next;
	}
	fclose(fp);
	return index_header;
}

int Encrypt_files(const char* archive_file, const char* password, char** files,int num_files)
{
	//---Checking whether the password is valid or not
	if(validate_password(password)==0)
	{
		printf("Password Must contain atleast 6 characters, 1 capital letter and one digit\n");
		exit(1);
	}

	//---Header to store the metadata about the crptz file
	archiveheader header;
	header.num_files=num_files;
	strcpy(header.file_type,"CRPTZ\0");
	header.hashpassword=hash_password(password);
	unsigned int checksum=0;
	int extrafiles=0;	
	//---Extract the file content into filenodes
	filenode *file_header=NULL; 
	for(int i=0;i<num_files;i++)
	{
		//---Copying, Compressing and Encryption of files
		if(find_file(file_header,files[i])!=NULL)
		{
			extrafiles++;
			continue;  //---skips repeated files
		}
		filenode * file=create_file_node(files[i]);
		if(file==NULL)
		{
			free_filenode(file_header);
			exit(1);
		}
		long size=file->original_size;
		if(file->original_size > 0)
		{
			unsigned char* cdata=compress_data(file->data,file->original_size,&(file->compressed_size));
			if(file->original_size > file->compressed_size)
			{
				file->is_compressed=1;
				free(file->data);
				file->data=cdata;
				size=file->compressed_size;
			}
			else
			{
				free(cdata);
			}
		}
		file->compressed_size=size;
		encrpt_data(file->data,size,password);
		append_file(&file_header,file);
		checksum+=compute_checksum(file->data,size);
	}
	header.num_files-=extrafiles;
	num_files-=extrafiles;
	header.checksum=checksum;

	//---Writing header and files into archive_header
	indexnode* index_header=write_into_archive(archive_file,file_header,header);

	//---Print Summary
	printf("----------------------------------\n");
	printf("Added %d files to %s\n",num_files,archive_file);
	printf("----------------------------------\n");
	indexnode* temp2=index_header;
	long total1=0;
	long total2=0;
	for(int i=0;i<num_files;i++)
	{
		total1+=temp2->original_size;
		total2+=temp2->compressed_size;

		printf("%-2d %-40s : %ld bytes  ->  %ld bytes",i+1,temp2->filename,temp2->original_size,temp2->compressed_size);
		if(temp2->is_compressed==1)
			printf(" -- compressed\n");
		else
			printf(" --not compressed\n");
		temp2=temp2->next;
	}
	printf("----------------------------------------\n");
	printf("Total\t: %ld bytes -> %ld bytes\n",total1,total2);
	printf("----------------------------------------\n");

	//---Free file_header & index_header
	free_filenode(file_header);
	free_indexnode(index_header);
	return 1;
}

archiveheader extract_details(const char* archive_file,indexnode** index_out_header)
{
	//---First get details in header
	archiveheader header;
	FILE *fp=fopen(archive_file,"rb");
	if(fp==NULL)
	{
		printf("Error Opening the file %s\n",archive_file);
		exit(1);
	}
	fread(&header,sizeof(archiveheader),1,fp);
	unsigned int checksum=0;
	if(strcmp(header.file_type,"CRPTZ")!=0)
	{
		printf("Only CRPTZ files are supported\n");
		fclose(fp);
		exit(1);
	}
	//---Store index information in indexnode
	indexnode* index_header=NULL;
	for(int i=0;i<header.num_files;i++)
	{
		int length;
		char filename[256];
		long original_size;
		long compressed_size;
		int is_compressed;
		long offset;
		fread(&length,sizeof(int),1,fp);
		if(length>255)
		{
			printf("File Currupted\n");
			free_indexnode(index_header);
			fclose(fp);
			exit(1);
		}
		fread(filename,1,length,fp);
		filename[length]='\0';
		fread(&original_size,sizeof(long),1,fp);
		fread(&compressed_size,sizeof(long),1,fp);
		fread(&is_compressed,sizeof(int),1,fp);
		fread(&offset,sizeof(long),1,fp);
		unsigned char* data=malloc(compressed_size);
		fread(data,1,compressed_size,fp);
		checksum+=compute_checksum(data,compressed_size);
		free(data);
		indexnode* file_index=create_index_node(filename,original_size,compressed_size,is_compressed,offset);
		append_index(&index_header,file_index);
	}
	if(checksum!=header.checksum)
	{
		printf("Checksum mismatch - Archive corrupted\n");
		fclose(fp);
		free_indexnode(index_header);
		exit(1);
	}
	*index_out_header=index_header;
	fclose(fp);
	return header;
}

int Extract_files(const char* archive_file, const char* password, const char* output_folder)
{
	indexnode *index_header=NULL;
	archiveheader header=extract_details(archive_file,&index_header);
	unsigned int hash=hash_password(password);
	if(hash!=header.hashpassword)
	{
		printf("Wrong Password\n");
		free_indexnode(index_header);
		exit(1);
	}
	FILE *fp=fopen(archive_file,"rb");

	//---Decrypt and Regenerate files from archive_file
	indexnode* temp=index_header;
	for(int i=0;i<header.num_files;i++)
	{
		fseek(fp,temp->offset,SEEK_SET)!=0;
		unsigned char *data=malloc(temp->compressed_size);
		fread(data,1,temp->compressed_size,fp);
		decrpt_data(data,temp->compressed_size,password);
		unsigned char* orig_data=data;
		if(temp->is_compressed==1)
		{
			orig_data=decompress_data(data,temp->compressed_size,temp->original_size);
			free(data);
		}
		//---Write data into files
		char out[520];
		strcpy(out,output_folder);
		char filename[256];
		strcpy(filename,temp->filename);
		char * token=strtok(filename,"/");
		char* name;
		while(token!=NULL)
		{
			name=token;
			token=strtok(NULL,"/");
		}
		strcat(out,name);
		FILE *file=fopen(out,"wb");
		if(file==NULL)
		{
			printf("Folder %s not found\n",output_folder);
			fclose(fp);
			free_indexnode(index_header);
			exit(1);
		}
		fwrite(orig_data,1,temp->original_size,file);
		free(orig_data);
		fclose(file);
		temp=temp->next;
	}
	//---Print Summary
	list_files(archive_file);
	fclose(fp);
	//---Free indexnodes
	free_indexnode(index_header);
	return 1;
}

int Extract_one_file(const char* archive_file,const char* password,const char* output_folder,char* filename)
{
	indexnode* index_header=NULL;
	archiveheader header=extract_details(archive_file,&index_header);
	unsigned int hash=hash_password(password);
	if(hash!=header.hashpassword)
	{
		printf("Wrong Password\n");
		free_indexnode(index_header);
		exit(1);
	}
	indexnode* temp=index_header;
	while(temp!=NULL)
	{
		if(strcmp(temp->filename,filename)==0)
		{

			FILE *fp=fopen(archive_file,"rb");
			if(fp==NULL)
			{
				free_indexnode(index_header);
				printf("Error Opening File %s\n",archive_file);
				exit(1);
			}
			fseek(fp,temp->offset,SEEK_SET)!=0;
			unsigned char* data=malloc(temp->compressed_size);
			fread(data,1,temp->compressed_size,fp);
			decrpt_data(data,temp->compressed_size,password);
			unsigned char* original_data=data;
			if(temp->is_compressed==1)
			{
				original_data=decompress_data(data,temp->compressed_size,temp->original_size);
				free(data);
			}
			char out[520];
			strcpy(out,output_folder);
			char* file;
			char* token=strtok(filename,"/");
			file=token;
			while(token!=NULL)
			{
				file=token;
				token=strtok(NULL,"/");
			}
			strcat(out,file);
			FILE *fp2=fopen(out,"wb");
			if(fp2==NULL)
			{
				printf("Folder %s not found\n",output_folder);
				fclose(fp);
				free_indexnode(index_header);
				exit(1);
			}
			fwrite(original_data,1,temp->original_size,fp2);
			free(original_data);
			fclose(fp);
			fclose(fp2);
			//---Print Summary
			printf("--------------------------------------\n");
			printf("File: %s\t|\t%d file(s)\n",archive_file,header.num_files);
			printf("--------------------------------------\n");
			printf("%d.  %s \n",1,temp->filename);
			printf("--------------------------------------\n");
			//---Free indexnode
			free_indexnode(index_header);
			return 1;
		}
		temp=temp->next;
	}
	printf("File not found in %s\n",archive_file);
	//---Free indexnode
	free_indexnode(index_header);
	return 0;
}

void list_files(const char* archive_file)
{
	indexnode* index_header=NULL;
	archiveheader header=extract_details(archive_file,&index_header);
	printf("--------------------------------------\n");
	printf("File: %s\t|\t%d file(s)\n",archive_file,header.num_files);
	printf("--------------------------------------\n");
	indexnode* temp=index_header;
	int i=1;
	while(temp!=NULL)
	{
		printf("%-2d  %-40s \n",i,temp->filename);
		i++;
		temp=temp->next;
	}
	printf("--------------------------------------\n");
	//---Free indexnode
	free_indexnode(index_header);
	return;
}

void Inspect_files(const char* archive_file)
{
	indexnode* index_header=NULL;
	archiveheader header=extract_details(archive_file,&index_header);
	printf("-----------------------------------\n");
	printf("File: %s\t|\t%d file(s)\n",archive_file,header.num_files);
	printf("-----------------------------------\n");
	index_print(index_header);
	//---Free indexnode
	free_indexnode(index_header);
}

void Remove_file(const char* archive_file, const char* password, const char* filename)
{
	indexnode* index_header=NULL;
	archiveheader header=extract_details(archive_file,&index_header);
	unsigned int hash=hash_password(password);
	if(hash!=header.hashpassword)
	{
		printf("Wrong Password\n");
		exit(1);
	}
	//---Check whether the file is present in archvive_header or not
	if(find_index(index_header,filename)==NULL)
	{
		free_indexnode(index_header);
		printf("File not found in %s\n",archive_file);
		return;
	}
	filenode* files_header=NULL;
	indexnode* temp=index_header;
	unsigned int checksum;
	while(temp!=NULL)
	{
		filenode* file=malloc(sizeof(filenode));
		strcpy(file->filename,temp->filename);
		file->length=strlen(temp->filename);
		file->original_size=temp->original_size;
		file->compressed_size=temp->compressed_size;
		file->is_compressed=temp->is_compressed;
		file->next=NULL;
		FILE *fp=fopen(archive_file,"rb");
		if(fp==NULL)
		{
			printf("Error Opening File %s\n",archive_file);
			free_indexnode(index_header);
			free_filenode(files_header);
			exit(1);
		}
		fseek(fp,temp->offset,SEEK_SET);
		file->data=(unsigned char*)malloc(file->compressed_size);
		fread(file->data,1,file->compressed_size,fp);
		if(strcmp(file->filename,filename)==0)
		{
			checksum=compute_checksum(file->data,file->compressed_size);
			free(file->data);
			free(file);
			fclose(fp);
			temp=temp->next;
			continue;
		}
		fclose(fp);
		append_file(&files_header,file);
		temp=temp->next;
	}

	//---update archiveheader
	header.num_files--;
	header.checksum-=checksum;

	//---write new content to archive_file
	indexnode* flag=write_into_archive(archive_file,files_header,header);
	free_indexnode(flag);
	//---Summary
	indexnode *node=find_index(index_header,filename);
	printf("------------------------------------------------\n");
	printf("   %-40sOrig Size\t       Stored Size\n","Filename");
	printf("------------------------------------------------\n");
	printf("%-2d %-40s%-12ldbytes   %-12ldbytes\n",1,node->filename,node->original_size,node->compressed_size);
	printf("------------------------------------------------\n");
	printf("File Removed\n");
	printf("------------------------------------------------\n");
	
	//---Free memory
	free_indexnode(index_header);
	free_filenode(files_header);
	return ;
}
