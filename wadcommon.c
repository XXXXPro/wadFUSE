// ====================================================
// Common routines for WAD files
// 
// Written by 4X_Pro <me@4xpro.ru>, 2019
// License: MIT
// ====================================================


#include "wadcommon.h"

struct WAD_header WAD_get_header(FILE* fh) {
  struct WAD_header result;
  fseek(fh,0,SEEK_SET);
  fread(&result,sizeof(struct WAD_header),1,fh);
  return result;
}

int check_WAD(struct WAD_header wad) {
  return (wad.signature[0]=='I' || wad.signature[0]=='P') && wad.signature[1]=='W' && wad.signature[2]=='A' && wad.signature[3]=='D';
}

int WAD_get_entries(FILE* fh, struct WAD_header wad, struct WAD_entry* result) {
  if (result!=NULL) {
    fseek(fh,wad.dir_offset,SEEK_SET);  
    return fread(result,sizeof(struct WAD_entry),wad.entries,fh);
  }
  return -1;
}

int WAD_read_entry(FILE* fh, struct WAD_entry wad_entry, void* buffer, size_t size, off_t offset) {
  if (buffer!=NULL) {
    fseek(fh,offset+wad_entry.offset,SEEK_SET);
    return fread(buffer,1,size,fh);
  }
  else return -1;
}

int WAD_find_entry(const char* name, struct WAD_header wad, struct WAD_entry wad_entry[]) {
  int result=-1;
  for (int i=0; i<wad.entries && result==-1; i++) {
    int found=1;
    for (int j=0; j<WAD_ENTRY_NAME_LEN && wad_entry[i].name[j]!=0 && found; j++) if (wad_entry[i].name[j]!=name[j]) found=0;
    if (found) result=i;
  }
  return result;
}
