#ifndef _MOVIE_H_
#define _MOVIE_H_


#include "jm_vl.h"

//==========================================================================
//
//  UNIT:  MOVIE.H
//
//==========================================================================


#pragma pack(2)
typedef struct
{
	uint16_t code;
   int32_t block_num;
   int32_t recsize;
}anim_frame;
#pragma pack()


#pragma pack(2)
typedef struct
{
	uint16_t opt;
   uint16_t offset;
   uint16_t length;
} anim_chunk;
#pragma pack()


//-------------------------------------------------------------------------
//   MovieStuff Anim Stucture...
//
//
//  fname 			-- File Name of the Anim to be played..
//	 rep				-- Number of repetitions to play the anim
//	 ticdelay		-- Tic wait between frames
//  maxmembuffer 	-- Maximum ammount to use as a ram buffer
//  start_line 	-- Starting line of screen to copy to other pages
//  end_line   	-- Ending line  "   "   "   "   "   "   "   "
//
typedef struct
{
	char FName[13];
	int8_t rep;
	int8_t ticdelay;

   uint32_t MaxMemBuffer;

	int16_t start_line;
	int16_t end_line;
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
   memptr palette;
#endif

} MovieStuff_t;



//=========================================================================
//
//											EXTERNS
//
//=========================================================================

extern memptr displaybuffer;
extern MovieStuff_t Movies[];

//===========================================================================
//
//								     Prototypes
//
//===========================================================================

void MOVIE_ShowFrame (char huge *inpic);
boolean MOVIE_Play(MovieStuff_t *MovieStuff);
void SetupMovie(MovieStuff_t *MovieStuff);
void ShutdownMovie(void);

#endif
