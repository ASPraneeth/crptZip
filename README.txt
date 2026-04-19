================================================================================
                              CRPTZ Archiver
================================================================================

What is this project about?
--------------------------------
CRPTZ is a simple file archiver that compresses and encrypts multiple files
into a single archive with the extension .crptz. It uses a basic Run-Length
Encoding (RLE) algorithm for compression and a lightweight XOR + rotation
cipher for encryption.

Features of this Project
--------------------------------
- Creates an encrypted archive (.crptz) from one or more input files.
- Compresses files when the compressed version is smaller than the original.
  --Compression uses basic RLE 
  	aaaa is compressed to 4a .
  	aaabbb is compressed to 3a3b.
  	aabb will not be compressed. Stored as it is.
- Verifies archive corruption using XOR checksum with bit rotation.
- Password is never stored directly — stored as a djb2 hash for security.
- Extracts all files or a specific file from an archive to an output folder.
- Verifies archive corruption using a checksum and password hash.
- Lists the contents of an archive.
- Inspects detailed metadata (original size, stored size, compression ratio of each File).
- Deletes a file from an existing archive.
- Memory leak free — verified with valgrind.

Compilation and execution of this project
----------------------------------------
The project includes a Makefile.

Requirements:
- GCC (or any C compiler)
- Standard C library

Compilation:
    $ make -f Makefile

This will produce an executable named 'crptz' (or 'crptz.exe' on Windows).

To clean object files and executable:
    $ make -f Makefile clean

File structure of this repository
-------------------------------------
    crptzip/
    ├── main.c                 
    ├── crptzip.c              
    ├── crptzip.h              
    ├── Makefile               
    ├── README.txt             # This file
    ├── test_files/            # Sample files for testing
    │   ├── txt_files/
    │   ├── jpeg_files/
    │   ├── pdf_files/
    │   ├── video_files/
    │   └── audio_files/
    ├── new/                   # Default output folder for extracted files (empty)
    └── crptz_files/           # Pre-made encrypted archives for testing 
    				USE "Test123" as PASSWORD


Functionalities supported
----------------------------
The program supports the following operations:

Command           Description
----------------------------------------------------------------------------
E                 Encrypt files into a new .crptz archive.
D                 Decrypt and extract file(s) from an archive.
list              Display the names of all files inside the archive.
inspect           Show detailed information about each file in the archive.
delete            Remove a specific file from the archive.

Input format and conditions
------------------------------
Usage

./crptz <command> [arguments...]

Command Overview

Command   | Syntax
----------|------------------------------------------------------------
E         | ./crptz E <archive_name> <password> <file1> [file2...]
D         | ./crptz D <archive_name> <password> <output_folder/>
          | or extract one file: ./crptz D <archive_name> <password> <output_folder/> <filename>
list      | ./crptz list <archive_name>
inspect   | ./crptz inspect <archive_name>
delete    | ./crptz delete <archive_name> <password> <filename>

Note:
	**** Password must contain at least one Capital letter and at least one digit.
	**** output_folder name must be less than 250 characters.
	**** File name is stored as complete file path w.r.t to current directory (Ex: test_files/txt_files/1mb.txt).
	**** filenames (including paths) must be less than 250 Characters.
	**** While using delete or Decryption for single file use filename as filepath w.r.t current directory.
       		Ex:  test_files/txt_files/1mb.txt  
       		-- Whole name must be entered not just "1mb.txt" as Multitple folders may contain different files with same names.
       	**** Duplicate filenames are skipped during Encryption (only first occurrence is stored). 
       	**** If archive is corrupted, program will detect and report it without extracting.
       	**** Archive corruption is verified automatically using checksum comparison.

Examples
-----------
All examples assume the executable is named 'crptz' and is in the current directory.

1 Create an archive
  ------------------
  	Command:
	$  ./crptz E ./crptz_files/backup.crptz MyPass123 test_files/txt_files/1mb.txt test_files/jpeg_files/image1.jpg
	
	Output:
	----------------------------------
	Added 2 files to ./crptz_files/backup.crptz
	----------------------------------
	1  test_files/txt_files/1mb.txt             : 1048593 bytes  ->  1048593 bytes --not compressed
	2  test_files/jpeg_files/image1.jpg         : 8934440 bytes  ->  8934440 bytes --not compressed
	----------------------------------------
	Total	: 9983033 bytes -> 9983033 bytes
	----------------------------------------
	Successfully Encrypted

2 List contents of the archive
  -----------------------------
  	Command:
	$  ./crptz list ./crptz_files/backup.crptz
	
	Output:
	--------------------------------------
	File: ./crptz_files/backup.crptz	|	2 file(s)
	--------------------------------------
	1   test_files/txt_files/1mb.txt             
	2   test_files/jpeg_files/image1.jpg         
	--------------------------------------

3 Inspect detailed information
  -----------------------------
  	Command:
	$  ./crptz inspect ./crptz_files/backup.crptz
	
	Output:
	-----------------------------------
	File: ./crptz_files/backup.crptz	|	2 file(s)
	-----------------------------------
	   Filename                                Orig Size	       Stored Size         Ratio	Compress
	1  test_files/txt_files/1mb.txt            1048593     bytes   1048593     bytes   0.00% saved	No
	2  test_files/jpeg_files/image1.jpg        8934440     bytes   8934440     bytes   0.00% saved	No
	----------------------------------------------------------------------
	Total: 2 file(s) | 9983033 bytes original | 9983033 bytes stored
	----------------------------------------------------------------------

4 Extract all files to a folder
  ------------------------------
  	Command:
	$  ./crptz D ./crptz_files/backup.crptz MyPass123 ./new/
	
	Output:
	--------------------------------------
	File: ./crptz_files/backup.crptz	|	2 file(s)
	--------------------------------------
	1   test_files/txt_files/1mb.txt             
	2   test_files/jpeg_files/image1.jpg         
	--------------------------------------
	Successfully Decrypted

5 Extract a single file
  ----------------------
  	Command:
	$  ./crptz D ./crptz_files/backup.crptz MyPass123 ./new/ test_files/txt_files/1mb.txt
	
	Output:
	--------------------------------------
	File: ./crptz_files/backup.crptz	|	2 file(s)
	--------------------------------------
	1.  test_files/txt_files/1mb.txt 
	--------------------------------------
	Successfully Decrypted

6 Delete a file from the archive
  ------------------------------- 
  	Command:
	$  ./crptz delete ./crptz_files/backup.crptz MyPass123 test_files/jpeg_files/image1.jpg
	
	Output:
	------------------------------------------------
	   Filename                                Orig Size	       Stored Size
	------------------------------------------------
	1  test_files/jpeg_files/image1.jpg        8934440     bytes   8934440     bytes
	------------------------------------------------
	File Removed
	------------------------------------------------
	
7 Corrupted Archive Detection
  ----------------------------
    Manually corrupt one byte in archive,
    then try to decrypt:
    
    Command:
    $  ./crptz D ./crptz_files/backup.crptz Test123 ./new/
    
    Output:
    Checksum mismatch — Archive corrupted
    
8 Wrong Password Detection
  -------------------------
    Command:
    $  ./crptz D ./crptz_files/backup.crptz WrongPass1 ./new/
    
    Output:
    Wrong password

================================================================================
                        End of README
================================================================================
