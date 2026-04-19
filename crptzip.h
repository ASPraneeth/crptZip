typedef struct FileNode
{
	int length;
	char filename[256];
	long original_size;
	long compressed_size;
	int is_compressed;
	unsigned char * data;
	struct FileNode *next;
}filenode;
typedef struct IndexNode 
{
	char filename[256];
	long original_size;
	long compressed_size;
	int is_compressed;
	long offset;
	struct IndexNode *prev;
	struct IndexNode *next;
}indexnode;

typedef struct ArchiveHeader
{
	char file_type[6];
	int num_files;
	unsigned int checksum;
	unsigned int hashpassword;
}archiveheader;

int Encrypt_files(const char*, const char*, char** ,int );
int Extract_files(const char*, const char* , const char*);
int Extract_one_file(const char*,const char*, const char*,char*);
void list_files(const char*);
void Inspect_files(const char*);
void Remove_file(const char*,const char*,const char*);
