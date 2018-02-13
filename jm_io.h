#ifndef _JM_IO_H_
#define _JM_IO_H_

//-------------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------------
boolean IO_FarRead (int handle, byte far *dest, int32_t length);
boolean IO_FarWrite (int handle, byte far *source, int32_t length);
boolean IO_ReadFile (char *filename, memptr *ptr);
boolean IO_WriteFile(char *filename, void far *ptr, int32_t length);
int32_t IO_LoadFile (char *filename, memptr *dst);
void IO_CopyFile(char *sFilename, char *dFilename);
void IO_CopyHandle(int sHandle, int dHandle, int32_t num_bytes);

#endif
