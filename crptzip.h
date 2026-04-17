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

int Encrypt_files(const char* archive_path, const char* password, char** files,int num_files);
int Extract_files(const char* archive_path, const char* password, const char* output_folder);
int Extract_one_file(const char* archive_path,const char* password, const char* output_folder,char* filename);
void list_files(const char* archive_file);
void Inspect_files(const char* archive_file);
