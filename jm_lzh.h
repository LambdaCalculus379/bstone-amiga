#ifndef _JM_LZH_H_
#define _JM_LZH_H_

extern void (*LZH_CompressDisplayVector)(uint32_t, uint32_t);
extern void (*LZH_DecompressDisplayVector)(uint32_t, uint32_t);



//===========================================================================
//
//											PROTOTYPES
//
//===========================================================================


boolean LZH_Startup(void);
void LZH_Shutdown(void);
int32_t LZH_Compress(void far *infile, void far *outfile,uint32_t DataLength,uint16_t PtrTypes);
int32_t LZH_Decompress(void far *infile, void far *outfile, uint32_t OrginalLength, uint32_t CompressLength, uint16_t PtrTypes);

#endif
