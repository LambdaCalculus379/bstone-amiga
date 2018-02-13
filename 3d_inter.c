// 3D_INTER.C

#include "3d_def.h"
//#pragma hdrstop


//==========================================================================
//
//									LOCAL CONSTANTS
//
//==========================================================================


//==========================================================================
//
//									LOCAL VARABLES
//
//==========================================================================

#ifndef ID_CACHE_BRIEFS
char BreifingText[13] = {"BRIEF_Wx.TXT"};
#endif

//==========================================================================

/*
==================
=
= CLearSplitVWB
=
==================
*/

void ClearSplitVWB (void)
{
	memset (update,0,sizeof(update));
	WindowX = 0;
	WindowY = 0;
	WindowW = 320;
	WindowH = 152;
}


//==========================================================================




/*
==================
=
= Breifing
=
==================
*/

boolean Breifing(breifing_type BreifingType,uint16_t episode)
{
#ifndef ID_CACHE_BRIEFS
	char chars[3] = {'L','W','I'};

	BreifingText[6] = chars[BreifingType];
	BreifingText[7] = '1'+episode;

	HelpPresenter(BreifingText,true,0,false);
#else
	HelpPresenter(NULL,true,BRIEF_W1+(episode*2)+BreifingType-1,false);
#endif

	return(EscPressed);
}


//==========================================================================


/*
=================
=
= PreloadGraphics
=
= Fill the cache up
=
=================
*/

void ShPrint(char far *text, int8_t shadow_color, boolean single_char)
{
	uint16_t old_color=fontcolor,old_x=px,old_y=py;
	char far *str,buf[2]={0,0};

	if (single_char)
	{
		str = buf;
		buf[0]=*text;
	}
	else
		str = text;

	fontcolor = shadow_color;
	py++;
	px++;
	USL_DrawString(str);						// JTR - This marks blocks!

	fontcolor = old_color;
	py = old_y;
	px = old_x;
	USL_DrawString(str);						// JTR - This marks blocks!
}

void PreloadUpdate(uint16_t current, uint16_t total)
{
	uint16_t w=WindowW-10;

	if (current > total)
		current=total;

	w = ((int32_t)w * current) / total;
	if (w)
		VWB_Bar(WindowX,WindowY,w-1,1,BORDER_TEXT_COLOR);

	VW_UpdateScreen();
}

char far prep_msg[]="^ST1^CEGet Ready, Blake!\r^XX";

void DisplayPrepingMsg(char far *text)
{
#if GAME_VERSION != SHAREWARE_VERSION

// Bomb out if FILE_ID.DIZ is bad!!
//
	if (((gamestate.mapon != 1) || (gamestate.episode != 0)) &&
		 (gamestate.flags & GS_BAD_DIZ_FILE))
		Quit(NULL);

#endif

// Cache-in font
//
	fontnumber=1;
	CA_CacheGrChunk(STARTFONT+1);
	BMAmsg(text);
	UNCACHEGRCHUNK(STARTFONT+1);

// Set thermometer boundaries
//
	WindowX = 36;
	WindowY = 188;
	WindowW = 260;
	WindowH = 32;


// Init MAP and GFX thermometer areas
//
	VWB_Bar(WindowX,WindowY-7,WindowW-10,2,BORDER_LO_COLOR);
	VWB_Bar(WindowX,WindowY-7,WindowW-11,1,BORDER_TEXT_COLOR-15);
	VWB_Bar(WindowX,WindowY,WindowW-10,2,BORDER_LO_COLOR);
	VWB_Bar(WindowX,WindowY,WindowW-11,1,BORDER_TEXT_COLOR-15);

// Update screen and fade in
//
	VW_UpdateScreen();
	if (screenfaded)
		VW_FadeIn();
}

void PreloadGraphics(void)
{
	WindowY=188;

	if (!(gamestate.flags & GS_QUICKRUN))
		VW_FadeIn ();

	PM_Preload(PreloadUpdate);
	IN_UserInput(70);

	if (playstate != ex_transported)
		VW_FadeOut ();

	DrawPlayBorder ();
	VW_UpdateScreen ();
}

//==========================================================================

/*
==================
=
= DrawHighScores
=
==================
*/

#define SCORE_Y_SPACING			7			//

void	DrawHighScores(void)
{
	char		buffer[16],*str;
	word		i,
				w,h;
	HighScore	*s;

	ClearMScreen();
	CA_CacheScreen (BACKGROUND_SCREENPIC);
	DrawMenuTitle("HIGH SCORES");

	if (playstate != ex_title)
		DrawInstructions(IT_HIGHSCORES);

	fontnumber=2;
	SETFONTCOLOR(ENABLED_TEXT_COLOR,TERM_BACK_COLOR);

	ShadowPrint("NAME",86,60);
	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
	// Re-enable code for AOG
#ifdef GAMEVER_RESTORATION_AOG
	ShadowPrint("MISSION",150,60);
#endif
	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
	ShadowPrint("SCORE",205,60);
#else
	ShadowPrint("SCORE",175,60);
#endif
	ShadowPrint("MISSION",247,53);
	ShadowPrint("RATIO",254,60);

	for (i = 0,s = Scores;i < MaxScores;i++,s++)
	{
		SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR-1,TERM_BACK_COLOR);
		//
		// name
		//
		if (*s->name)
			ShadowPrint(s->name,45,68 + (SCORE_Y_SPACING * i));

		/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
		// Re-enable code for AOG
#ifdef GAMEVER_RESTORATION_AOG
//#if 0
		//
		// mission
		//

		ltoa(s->episode+1,buffer,10);
		ShadowPrint(buffer,165,68 + (SCORE_Y_SPACING * i));
#endif

		//
		// score
		//

		if (s->score > 9999999)
			SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR+1,TERM_BACK_COLOR);

		ltoa(s->score,buffer,10);
		USL_MeasureString(buffer,&w,&h);
		/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
		// Should explain the comment mentioning 235
#ifdef GAMEVER_RESTORATION_AOG
		ShadowPrint(buffer,235 - w,68 + (SCORE_Y_SPACING * i));
#else
		ShadowPrint(buffer,205 - w,68 + (SCORE_Y_SPACING * i));		// 235
#endif

		//
		// mission ratio
		//
		ltoa(s->ratio,buffer,10);
		USL_MeasureString(buffer,&w,&h);
		ShadowPrint(buffer,272-w,68 + (SCORE_Y_SPACING * i));
	}

	VW_UpdateScreen ();
}

//===========================================================================


/*
=======================
=
= CheckHighScore
=
=======================
*/

void	CheckHighScore (int32_t score,word other)
{
	word		i,j;
	int16_t			n;
	HighScore	myscore;
	US_CursorStruct TermCursor = {'@',0,HIGHLIGHT_TEXT_COLOR,2};


	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
   // Check for cheaters

   if (DebugOk)
	{
   	SD_PlaySound(NOWAYSND);
      return;
   }
#endif

	strcpy(myscore.name,"");
	myscore.score = score;
	myscore.episode = gamestate.episode;
	myscore.completed = other;
	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
	myscore.ratio = ShowStats(0,0,ss_justcalc);
#else
	myscore.ratio = ShowStats(0,0,ss_justcalc,&gamestuff.level[gamestate.mapon].stats);
#endif

	for (i = 0,n = -1;i < MaxScores;i++)
	{
		if ((myscore.score > Scores[i].score) ||	((myscore.score == Scores[i].score)
			&& (myscore.completed > Scores[i].completed)))
		{
			for (j = MaxScores;--j > i;)
				Scores[j] = Scores[j - 1];
			Scores[i] = myscore;
			n = i;
			break;
		}
	}

	StartCPMusic (ROSTER_MUS);
	DrawHighScores ();

	VW_FadeIn ();

	if (n != -1)
	{
		//
		// got a high score
		//

		DrawInstructions(IT_ENTER_HIGHSCORE);
		SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR,TERM_BACK_COLOR);
		PrintY = 68+(SCORE_Y_SPACING * n);
		PrintX = 45;
		use_custom_cursor = true;
		allcaps = true;
		US_CustomCursor = TermCursor;
		US_LineInput(PrintX,PrintY,Scores[n].name,nil,true,MaxHighName,100);
	}
	else
	{
		IN_ClearKeysDown ();
		IN_UserInput(500);
	}

   StopMusic();
	use_custom_cursor = false;
}

//===========================================================================

//--------------------------------------------------------------------------
// Random()
//--------------------------------------------------------------------------
uint16_t Random(uint16_t Max)
{
	uint16_t returnval;

   if (Max)
   {
		if (Max > 255)
   		returnval = (US_RndT()<<8) + US_RndT();
	   else
   		returnval = US_RndT();

		return(returnval % Max);
   }
   else
   	return(0);
}

