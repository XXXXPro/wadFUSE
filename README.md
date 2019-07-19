# wadFUSE
FUSE module for mounting old good Doom WAD files as user-space filesystem (read-only). Files from Hexen, Heretic or Birthright should work too, but haven't tested. Written just for fun and some FUSE modules writing excercise.

### Compilation: 
gcc -Wall wadfuse.c wadcommon.c \`pkg-config fuse --cflags --libs\` -o wadfuse

### Usage: 
./wadfuse \<WADfile\> \<mountpoint\> 

WADfile is path to IWAD or PWAD file, mountpoint is empty writable by current user directory.
### Dependencies: 
libfuse-dev package
