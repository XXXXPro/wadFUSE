// ============================================================================================
// FUSE module for mounting old good Doom WAD files as user-space filesystem (read-only)
// 
// Written by 4X_Pro <me@4xpro.ru>, 2019, just for fun and some FUSE modules writing excercise
// License: MIT
// 
// Compilation: gcc -Wall wadfuse.c wadcommon.c `pkg-config fuse --cflags --libs` -o wadfuse
// Usage: ./wadfuse <WADfile> <mountpoint> 
// Dependencies: libfuse-dev package
// =============================================================================================

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "wadcommon.h"
#include <limits.h>

static struct WAD_header wad_header; // Cache for WAD header
static struct WAD_entry* wad_entries; // Pointer to cache of WAD directory
static FILE* wad_fh; // WAD file handle
static char wad_name[PATH_MAX]=""; // where file name will be stored
static struct stat wad_st; // for WAD stats

static int wad_getattr(const char *path, struct stat *st) {
  st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = wad_st.st_atime;
	st->st_mtime = wad_st.st_mtime;
	if ( strcmp( path, "/" ) == 0 ) { // if root directory requested
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2;
	}
	else
	{
    int position = WAD_find_entry(path+1,wad_header,wad_entries); // path[0] contatins / symbol, so name of entry starts from path[1]
    if (position==-1) return -ENOENT; // if no entry found, returning error
    else {
      st->st_mode = S_IFREG | 0644;
		  st->st_nlink = 1;
		  st->st_size = wad_entries[position].length; // returning correct size of entry
    }
	}
		
	return 0;  
}

static int wad_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
  char name_buffer[WAD_ENTRY_NAME_LEN+1];
  name_buffer[WAD_ENTRY_NAME_LEN]=0;
   
  if (offset<2) { 	
	  if (offset==0) filler( buffer, ".", NULL, 0 ); // Current Directory
	  filler( buffer, "..", NULL, 0 ); // Parent Directory
    offset=2;
  }
	
	if ( strcmp( path, "/" ) == 0 ) // If the user is trying to show the files/directories of the root directory show the following
	{ 
    for (int i=offset; i<wad_header.entries+2; i++) { 
      memcpy(name_buffer,wad_entries[i-2].name,8); // Copying entry name to buffer with zero byte at the end
		  filler( buffer, name_buffer, NULL, 0 );		
    }
	}
  else return -ENOTDIR; // no other dirs in WAD possible except the root
	
	return 0;
}

static int wad_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {  
  int position = WAD_find_entry(path+1,wad_header,wad_entries);
  int read=0;
  if (position==-1) return -ENOENT;
  else {
    if (size>wad_entries[position].length-offset) size=wad_entries[position].length-offset;
    return WAD_read_entry(wad_fh,wad_entries[position],buffer,size,offset);
  }
  sprintf(buffer,"Path: %s, read bytes: %d",path+1, read);  
  return strlen(buffer);
}

static struct fuse_operations operations = {
  .getattr	= wad_getattr,
  .readdir	= wad_readdir,
  .read	= wad_read,
};

static int wadfs_opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs) {
  if (key == FUSE_OPT_KEY_NONOPT && strcmp(wad_name,"")==0) { // Getting WAD file name as first argument
    strcpy(wad_name,arg);
    return 0;
  }
  return 1;
}

int main(int argc, char *argv[]) {
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  fuse_opt_parse(&args, NULL, NULL, wadfs_opt_proc);
  if (argc<3) {
    printf("Usage:\r\n  wadfuse file mountpoint\r\nWhere:\r\nfile — path to WAD file\r\nmountpoint — directory where to mount WAD filesystem\r\n\r\n");    
    return 0;
  }
  stat(wad_name,&wad_st); // Getting stat of WAD before opening to preserve last access time
  wad_fh = fopen(wad_name,"r");

  // some error checking
  if (wad_fh==NULL) {
    fprintf(stderr,"Unable to open WAD file %s!\r\n",wad_name);
    exit(1);
  }
  wad_header = WAD_get_header(wad_fh);
  if (!check_WAD(wad_header)) {
    fprintf(stderr,"File %s is not recognized as WAD file!\r\n",argv[1]);
    fclose(wad_fh);
    exit(2);    
  }
  wad_entries = malloc(sizeof(struct WAD_entry)*wad_header.entries);
  if (wad_entries==NULL) {
    fprintf(stderr,"Unable to get memory for WAD directory!\r\n");
    fclose(wad_fh);
    exit(3);        
  }

  // if eveything is ok, loading entries and start FUSE
  WAD_get_entries(wad_fh,wad_header,wad_entries);
  int result=fuse_main(args.argc, args.argv, &operations, NULL);
  // some cleanup
  fclose(wad_fh);
  free(wad_entries);
  return result;
}
