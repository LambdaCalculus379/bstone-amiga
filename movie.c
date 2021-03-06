//===========================================================================
//
//
//
//
//===========================================================================

//#include <io.h>
//#include <alloc.h>
//#include <mem.h>
#include <stdio.h>
#include <string.h>
//#include <dos.h>
#include <fcntl.h>

#include "3d_def.h"
#include "jm_io.h"
#include "an_codes.h"

//===========================================================================
//
//
//
//
//===========================================================================


//#define  DRAW_TO_FRONT

//
// Various types for various purposes...
//

typedef enum
{
  	FADE_NONE,
   FADE_SET,
   FADE_IN,
   FADE_OUT,
   FADE_IN_FRAME,
   FADE_OUT_FRAME,
} FADES;


typedef enum
{
   MV_NONE,
	MV_FILL,
   MV_SKIP,
   MV_READ,

} MOVIE_FLAGS;


//===========================================================================
//
//											VARIABLES
//
//===========================================================================


// Movie File variables

int Movie_FHandle;

// Fade Variables

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
int16_t	unused_movie_flags = 0;
#endif
FADES fade_flags, fi_type, fo_type;
byte	fi_rate,fo_rate;

// MOVIE_GetFrame & MOVIE_LoadBuffer variables

memptr MovieBuffer;					// Ptr to Allocated Memory for Buffer
uint32_t BufferLen;			// Len of MovieBuffer (Ammount of RAM allocated)
uint32_t PageLen;				// Len of data loaded into MovieBuffer
char huge * BufferPtr;				// Ptr to next frame in MovieBuffer
char huge * NextPtr;   				// Ptr Ofs to next frame after BufferOfs

boolean MorePagesAvail;				// More Pages avail on disk?

//

MOVIE_FLAGS  movie_flag;
boolean ExitMovie;
boolean EverFaded;
int32_t seek_pos;
int8_t movie_reps;
ControlInfo ci;
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
memptr movie_palette;
#endif


//
// MOVIE Definations for external movies
//
// NOTE: This list is ordered according to mv_???? enum list.
//


MovieStuff_t Movies[] =
{
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
	{{"SANIM."},1,3,0,0,200},				//mv_ending1
	{{"GANIM."},1,7,0,0,200},				//mv_ending2
	{{"IANIM."},1,3,0,0,200},				//mv_intro
	{{"EANIM."},1,6,0,0,200},				//mv_final
#else
	{{"IANIM."},1,3,0,0,200},				//mv_intro
	{{"EANIM."},1,30,0,0,200},				//mv_final
#endif
};



//===========================================================================
//
//										LOCAL PROTO TYPES
//
//===========================================================================

void JM_MemToScreen(void);
void JM_ClearVGAScreen(byte fill);
void FlipPages(void);
boolean CheckFading(void);
boolean CheckPostFade(void);


//===========================================================================
//
//										   FUNCTIONS
//
//===========================================================================


//---------------------------------------------------------------------------
// SetupMovie() - Inits all the internal routines for the Movie Presenter
//
//
//
//---------------------------------------------------------------------------
void SetupMovie(MovieStuff_t *MovieStuff)
{
#ifdef DRAW_TO_FRONT
	bufferofs=displayofs;
#endif

   movie_reps = MovieStuff->rep;
	movie_flag = MV_FILL;
   /*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
   LastScan = unused_movie_flags = 0;										  
#else
   LastScan = 0;										  
#endif
   PageLen = 0;
   MorePagesAvail = true;
	ExitMovie = false;
	EverFaded = false;
	IN_ClearKeysDown();

	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
   movie_palette = MovieStuff->palette;
#endif	
	JM_VGALinearFill(screenloc[0],3*80*208,0);

	VL_FillPalette (0,0,0);
   LastScan = 0;

   // Find out how much memory we have to work with..

#ifdef __AMIGA__
	// this will force the buffer size to 64256
    BufferLen = 65535;
#else
	BufferLen = MM_LargestAvail();
#endif
   BufferLen -= 65535;						// HACK: Save some room for sounds - This is cludgey

   if (BufferLen < 64256)
   	BufferLen = 64256;

	MM_GetPtr(&MovieBuffer,BufferLen);
	PM_CheckMainMem();
}


//---------------------------------------------------------------------------
// void ShutdownMovie(void)
//---------------------------------------------------------------------------
void ShutdownMovie(void)
{
	MM_FreePtr(&MovieBuffer);
   close (Movie_FHandle);
}

//---------------------------------------------------------------------------
// void JM_DrawBlock()
//
// dest_offset = Correct offset value for memory location for Paged/Latch mem
//
// byte_offset = Offset for the image to be drawn - This address is NOT
//					  a calculated Paged/Latch value but a byte offset in
//					  conventional memory.
//
// source		= Source image of graphic to be blasted to latch memory.  This
//					  pic is NOT 'munged'
//
// length		= length of the source image in bytes
//---------------------------------------------------------------------------
void JM_DrawBlock(uint16_t dest_offset,uint16_t byte_offset,char far *source,uint16_t length)
{
	byte numplanes;
   byte mask,plane;
	char huge *dest_ptr;
	char huge *source_ptr;
   char huge *dest;
   char huge *end_ptr;
   uint16_t count,total_len;
   
#ifdef __AMIGA__
	dest = &vga_memory[vl_get_offset(dest_offset, 0, 0) + byte_offset];
	for (count=0;count<length;count++)
	{
		*dest++ = *source++;
	}
#else


   end_ptr = source+length;

   //
   // Byte offset to determine our starting page to write to...
   //

   mask = 1<<(byte_offset & 3);

   //
   // Check to see if we are writting more than 4 bytes (to loop pages...)
   //

   if (length >= 4)
   	numplanes = 4;
   else
   	numplanes = length;

   //
   // Compute our DEST memory location
   //

   dest = MK_FP(0xA000,dest_offset+(byte_offset>>2));

   //
   // Move that memory.
   //

	for (plane = 0; plane<numplanes; plane++)
	{
   	dest_ptr = dest;
	   source_ptr = source+plane;

		VGAMAPMASK(mask);
		mask <<= 1;
		if (mask == 16)
      {
			mask = 1;
         dest++;
      }

		for (count=0;count<length,source_ptr < end_ptr;count+=4,dest_ptr++,source_ptr+=4)
      	*dest_ptr = *source_ptr;
	}
#endif
}



//---------------------------------------------------------------------------
// MOVIE_ShowFrame() - Shows an animation frame
//
// PARAMETERS: pointer to animpic
//---------------------------------------------------------------------------
void MOVIE_ShowFrame (char huge *inpic)
{
   anim_chunk huge *ah;

   if (inpic == NULL)
      return;

   for (;;)
   {
      ah = (anim_chunk huge *)inpic;
#ifdef __AMIGA__
		// TODO: make a copy of the struct?
		ah->opt = SWAP16LE(ah->opt);
		ah->offset = SWAP16LE(ah->offset);
		ah->length = SWAP16LE(ah->length);
#endif

      if (ah->opt == 0)
			break;

      inpic += sizeof(anim_chunk);
		JM_DrawBlock(bufferofs, ah->offset, (char far *)inpic, ah->length);
      inpic += ah->length;
   }
}



//---------------------------------------------------------------------------
// MOVIE_LoadBuffer() - Loads the RAM Buffer full of graphics...
//
// RETURNS:  true  	- MORE Pages avail on disk..
//				 false   - LAST Pages from disk..
//
// PageLen = Length of data loaded into buffer
//
//---------------------------------------------------------------------------
boolean MOVIE_LoadBuffer()
{
   anim_frame blk;
   int32_t chunkstart;
	char huge *frame;
   uint32_t free_space;

   //NextPtr = BufferPtr = frame = MK_FP(MovieBuffer,0);
   NextPtr = BufferPtr = frame = MovieBuffer;
   free_space = BufferLen;

	while (free_space)
   {
   	chunkstart = tell(Movie_FHandle);

	   if (!IO_FarRead(Movie_FHandle, (byte far *)&blk, sizeof(anim_frame)))
			AN_ERROR(AN_BAD_ANIM_FILE);

#ifdef __AMIGA__
		blk.block_num = SWAP32LE(blk.block_num);
		blk.recsize = SWAP32LE(blk.recsize);
		blk.code = SWAP16LE(blk.code);
#endif

      if (blk.code == AN_END_OF_ANIM)
      	return(false);

		if (free_space>=(blk.recsize+sizeof(anim_frame)))
      {
			_fmemcpy(frame, (byte far *)&blk, sizeof(anim_frame));

      	free_space -= sizeof(anim_frame);
   	   frame += sizeof(anim_frame);
         PageLen += sizeof(anim_frame);

		   if (!IO_FarRead(Movie_FHandle, (byte far *)frame, blk.recsize))
				AN_ERROR(AN_BAD_ANIM_FILE);

         free_space -= blk.recsize;
         frame += blk.recsize;
         PageLen += blk.recsize;
      }
      else
      {
	      lseek(Movie_FHandle,chunkstart,SEEK_SET);
         free_space = 0;
      }
   }

   return(true);
}


//---------------------------------------------------------------------------
// MOVIE_GetFrame() - Returns pointer to next Block/Screen of animation
//
// PURPOSE: This function "Buffers" the movie presentation from allocated
//				ram.  It loads and buffers incomming frames of animation..
//
// RETURNS:  0 - Ok
//				 1 - End Of File
//---------------------------------------------------------------------------
int16_t MOVIE_GetFrame()
{
	uint16_t ReturnVal;
   anim_frame blk;

	if (PageLen == 0)
   {
    	if (MorePagesAvail)
	      MorePagesAvail = MOVIE_LoadBuffer(Movie_FHandle);
      else
      	return(1);
	}

   BufferPtr = NextPtr;
	_fmemcpy(&blk, BufferPtr, sizeof(anim_frame));
   PageLen-=sizeof(anim_frame);
   PageLen-=blk.recsize;
   NextPtr = BufferPtr+sizeof(anim_frame)+blk.recsize;
	return(0);
}



//---------------------------------------------------------------------------
// MOVIE_HandlePage() - This handles the current page of data from the
//								ram buffer...
//
// PURPOSE: Process current Page of anim.
//
//
// RETURNS:
//
//---------------------------------------------------------------------------
void MOVIE_HandlePage(MovieStuff_t *MovieStuff)
{
	anim_frame blk;
	char huge *frame;
   uint16_t wait_time;

	_fmemcpy(&blk,BufferPtr,sizeof(anim_frame));
	BufferPtr+=sizeof(anim_frame);
   frame = BufferPtr;

	IN_ReadControl(0,&ci);

   switch (blk.code)
   {

      //-------------------------------------------
      //
      //
      //-------------------------------------------

	 	case AN_SOUND:				// Sound Chunk
		{
      	uint16_t sound_chunk;
         sound_chunk = *(uint16_t far *)frame;
#ifdef __AMIGA__
			sound_chunk = SWAP16LE(sound_chunk);
#endif
      	SD_PlaySound(sound_chunk);
         BufferPtr+=blk.recsize;
      }
      break;


      //-------------------------------------------
      //
      //
      //-------------------------------------------

		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
		case MV_CNVT_CODE('P','M'):
		break;
#endif
#if 0
		case MV_CNVT_CODE('P','M'):				// Play Music
		{
      	uint16_t song_chunk;
         song_chunk = *(uint16_t far *)frame;
         SD_MusicOff();

			if (!audiosegs[STARTMUSIC+musicchunk])
			{
//				MM_BombOnError(false);
				CA_CacheAudioChunk(STARTMUSIC + musicchunk);
//				MM_BombOnError(true);
			}

			if (mmerror)
				mmerror = false;
			else
			{
				MM_SetLock(&((memptr)audiosegs[STARTMUSIC + musicchunk]),true);
				SD_StartMusic((MusicGroup far *)audiosegs[STARTMUSIC + musicchunk]);
			}

         BufferPtr+=blk.recsize;
      }
      break;
#endif


      //-------------------------------------------
      //
      //
      //-------------------------------------------

	 	case AN_FADE_IN_FRAME:				// Fade In Page
		/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
        	VW_FadeIn();
#else
        	//VL_FadeIn(0,255,MK_FP(movie_palette,0),30);
			VL_FadeIn(0,255,movie_palette,30);
#endif
			fade_flags = FADE_NONE;
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
		unused_movie_flags = 0x3F;
#endif
         EverFaded = true;
			screenfaded = false;
      break;



      //-------------------------------------------
      //
      //
      //-------------------------------------------

	 	case AN_FADE_OUT_FRAME:				// Fade Out Page
			VW_FadeOut();
			screenfaded = true;
			fade_flags = FADE_NONE;
			/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
			unused_movie_flags = 0;
#endif
      break;


      //-------------------------------------------
      //
      //
      //-------------------------------------------

	 	case AN_PAUSE:				// Pause
		{
      	uint16_t vbls;
         vbls = *(uint16_t far *)frame;
#ifdef __AMIGA__
			vbls = SWAP16LE(vbls);
#endif
			IN_UserInput(vbls);
         BufferPtr+=blk.recsize;
      }
      break;



      //-------------------------------------------
      //
      //
      //-------------------------------------------

   	case AN_PAGE:				// Graphics Chunk
#if 1
         if (movie_flag == MV_FILL)
         {
            // First page comming in.. Fill screen with fill color...
            //
		/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
		// Uncommented MV_READ line for v1.0
#ifdef GAMEVER_RESTORATION_AOG_100
            movie_flag = MV_READ;	// Set READ flag to skip the first frame on an anim repeat
#else
            movie_flag = MV_NONE;	// Set READ flag to skip the first frame on an anim repeat
#endif
			  	JM_VGALinearFill(screenloc[0],3*80*208,*frame);
            frame++;
         }
         else
#endif 
				VL_LatchToScreen(displayofs+ylookup[MovieStuff->start_line], 320>>2, MovieStuff->end_line-MovieStuff->start_line, 0, MovieStuff->start_line);

         MOVIE_ShowFrame(frame);

         /*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
//#if 0  
         if (movie_flag == MV_READ)
         {
         	seek_pos = tell(Movie_FHandle);
            movie_flag = MV_NONE;
         }
#endif 
#ifdef GAMEVER_RESTORATION_AOG
         CycleColors();
#endif
   		FlipPages();

         /*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/

         if (TimeCount < MovieStuff->ticdelay)
         {
	         wait_time = MovieStuff->ticdelay - TimeCount;
				VL_WaitVBL(wait_time);
         }
         else
				VL_WaitVBL(1);

			TimeCount = 0;

			if ((!screenfaded) && (ci.button0 || ci.button1 || LastScan))
			{
				ExitMovie = true;
				if (EverFaded)					// This needs to be a passed flag...
				{
					VW_FadeOut();
					screenfaded = true;
				}
			}
		break;


		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
//#if 0
      //-------------------------------------------
      //
      //
      //-------------------------------------------

		case AN_PRELOAD_BEGIN:			// These are NOT handled YET!
		case AN_PRELOAD_END:
		break;

#endif
      //-------------------------------------------
      //
      //
      //-------------------------------------------

		case AN_END_OF_ANIM:
			ExitMovie = true;
		break;


      //-------------------------------------------
      //
      //
      //-------------------------------------------

		default:
			AN_ERROR(HANDLEPAGE_BAD_CODE);
		break;
   }
}


//---------------------------------------------------------------------------
// MOVIE_Play() - Playes an Animation
//
// RETURNS: true  - Movie File was found and "played"
//				false - Movie file was NOT found!
//---------------------------------------------------------------------------
boolean MOVIE_Play(MovieStuff_t *MovieStuff)
{
	// Init our Movie Stuff...
   //

   SetupMovie(MovieStuff);

   // Start the anim process
   //

   /*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
   if ((Movie_FHandle = open(MovieStuff->FName, O_RDONLY|O_BINARY)) == -1)
#else
   if ((Movie_FHandle = open(MovieStuff->FName, O_RDONLY|O_BINARY, S_IREAD)) == -1)	  
#endif
     	return(false);

   while (movie_reps && (!ExitMovie))
	{
      /*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
//#if 0 	
      if (movie_flag == MV_SKIP)
	   	if (lseek(Movie_FHandle, seek_pos, SEEK_SET) == -1)
         	return(false);
#endif
	   for (;!ExitMovie;)
   	{
      	if (MOVIE_GetFrame(Movie_FHandle))
         	break;

         MOVIE_HandlePage(MovieStuff);
      }

      movie_reps--;
      movie_flag = MV_SKIP;
   }

   ShutdownMovie();

   return(true);
}




//--------------------------------------------------------------------------
// FlipPages()
//---------------------------------------------------------------------------
void FlipPages(void)
{

#ifndef DRAW_TO_FRONT

	displayofs = bufferofs;

#ifdef __AMIGA__
	VH_UpdateScreen();
#else
	asm	cli
	asm	mov	cx,[displayofs]
	asm	mov	dx,3d4h		// CRTC address register
	asm	mov	al,0ch		// start address high register
	asm	out	dx,al
	asm	inc	dx
	asm	mov	al,ch
	asm	out	dx,al   	// set the high byte
#if 0
	asm	dec	dx
	asm	mov	al,0dh		// start address low register
	asm	out	dx,al
	asm	inc	dx
	asm	mov	al,cl
	asm	out	dx,al		// set the low byte
#endif
	asm	sti
#endif

	bufferofs += SCREENSIZE;
	if (bufferofs > PAGE3START)
		bufferofs = PAGE1START;

#endif

}
