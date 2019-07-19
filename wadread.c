// ====================================================
// Simple WAD extraction utility, used for testing of wadfuse
// 
// Written by 4X_Pro <me@4xpro.ru>, 2019
// License: MIT
// 
// Compilation: gcc -Wall wadread.c wadcommon.c -o wadread
// Usage: ./wadread <WAD file> <entry name> 
// ====================================================


#include "wadcommon.h"
#include <string.h>

void list_WAD_entries(struct WAD_header wad, struct WAD_entry entries[]) {  
  char buffer[WAD_ENTRY_NAME_LEN+1];
  buffer[WAD_ENTRY_NAME_LEN]=0;
  for (int i=0; i<wad.entries; i++) {
    memcpy(buffer,entries[i].name,WAD_ENTRY_NAME_LEN);
    printf("%s - %d\r\n",buffer,entries[i].length);
  }
}

int main(int argc, char** argv) {
  FILE* wad_fh;
  struct WAD_header wad;
  struct WAD_entry* entries;
  
  if (argc==1) {
    printf("Usage:\r\n  wadread file [entry]\r\nWhere:\r\nfile — path to WAD file\r\nentry — name of entry in WAD file to extract.\r\nIf entry is omitted, list of entries will be printed\r\n\r\n");    
  }
  wad_fh = fopen(argv[1],"r");
  if (wad_fh==NULL) {
    fprintf(stderr,"Unable to open WAD file %s!\r\n",argv[1]);
    exit(1);
  }
  wad = WAD_get_header(wad_fh);
  if (!check_WAD(wad)) {
    fprintf(stderr,"File %s is not recognized as WAD file!\r\n",argv[1]);
    fclose(wad_fh);
    exit(2);    
  }
  entries = malloc(sizeof(struct WAD_entry)*wad.entries);
  if (entries==NULL) {
    fprintf(stderr,"Unable to get memory for WAD directory!\r\n");
  }
  else {
    WAD_get_entries(wad_fh,wad,entries);
    if (argc==2) list_WAD_entries(wad,entries);
    else {
      int position = WAD_find_entry(argv[2],wad,entries);
      if (position<0) {
        fprintf(stderr,"Entry %s not found in WAD %s!\r\n",argv[2],argv[1]);
        if (entries!=NULL) free(entries);
        fclose(wad_fh);        
        exit(3);
      }
      else {
        void* buffer = malloc(entries[position].length);
        // fprintf(stderr,"Entry found! position: %d, offset: %d, length: %d\r\n",position,entries[position].offset,entries[position].length);
        if (WAD_read_entry(wad_fh,entries[position],buffer,entries[position].length,0)) fwrite(buffer,entries[position].length,1,stdout);
        free(buffer);        
      }
    }
  }
  if (entries!=NULL) free(entries);
  fclose(wad_fh);
}
