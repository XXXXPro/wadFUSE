#ifndef WADCOMMON_h

#define WADCOMMON_h
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define WAD_ENTRY_NAME_LEN 8

struct __attribute__((packed)) WAD_header {
  char signature[4];
  int32_t entries;
  int32_t dir_offset;
};

struct __attribute__((packed)) WAD_entry {
  int32_t offset;
  int32_t length;
  char name[WAD_ENTRY_NAME_LEN];  
};

extern struct WAD_header WAD_get_header(FILE* fh);

extern int check_WAD(struct WAD_header wad);

extern int WAD_get_entries(FILE* fh, struct WAD_header wad, struct WAD_entry* result);

extern int WAD_read_entry(FILE* fh, struct WAD_entry wad_entry, void* buffer, size_t size, off_t offset);

extern int WAD_find_entry(const char* name, struct WAD_header wad, struct WAD_entry wad_entry[]);
#endif
