
#include "3d_def.h"

#include "jm_tp.h"
#include "jm_io.h"
#ifdef __AMIGA__
#include "lowlevel.h"
#endif

//#pragma hdrstop

// As is, this switch will not work ... the data associated with this
// is not saved out correctly.
//
//#define CACHE_KEY_DATA


//
// End game message
//

char far EndGameStr[] = {"    End current game?\n"
								 " Are you sure (Y or N)?"
};


#define ENDGAMESTR	(EndGameStr)

char far QuitToDosStr[] = {"      Quit to DOS?\n"
									" Are you sure (Y or N)?"
};

//#define FREEFONT(fontnum)	{MM_SetPurge (&(memptr)grsegs[fontnum],3);}
#define FREEFONT(fontnum) {if (fontnum != STARTFONT+2 && grsegs[fontnum]) UNCACHEGRCHUNK(fontnum);}


boolean EscPressed = false;

int16_t	lastmenumusic;


//===========================================================================
//
// 							    PRIVATE PROTOTYPES
//
//===========================================================================

void CP_ReadThis(void);
void CP_OrderingInfo(void);
void DrawEpisodePic(int16_t w);
void DrawAllSoundLights(int16_t which);
void ReadGameNames(void);
void FreeMusic(void);
void CP_GameOptions(void);
void DrawGopMenu(void);
void CalibrateJoystick(void);
void ExitGame(void);
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
void CP_Switches(void);
void DrawSwitchMenu(void);
void DrawAllSwitchLights(int16_t which);
void DrawSwitchDescription(int16_t which);
#endif


extern boolean refresh_screen;


//===========================================================================
//
// 							       LOCAL DATA...
//
//===========================================================================

CP_iteminfo
	far MainItems=	{MENU_X,MENU_Y,12,MM_READ_THIS,0,9,{77, 1,154,9,1}},
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
	far GopItems=	{MENU_X,MENU_Y+30,3,0,0,9,{77, 1,154,9,1}},
#else
	far GopItems=	{MENU_X,MENU_Y+30,4,0,0,9,{77, 1,154,9,1}},
#endif
	far SndItems=	{SM_X,SM_Y,12,0,0,7,		{87,-1,144,7,1}},
	far LSItems=	{LSM_X,LSM_Y,10,0,0,8,	{86,-1,144,8,1}},
	far CtlItems=	{CTL_X,CTL_Y,7,-1,0,9,	{87,1,174,9,1}},
	far CusItems=	{CST_X,CST_Y+7,6,-1,0,15,{54,-1,203,7,1}},
#ifdef __AMIGA__
	// there are only 6 episodes
	far NewEitems=	{NE_X,NE_Y, 6,0,0,16,	{43,-2,119,16,1}},
#else
	far NewEitems=	{NE_X,NE_Y,11,0,0,16,	{43,-2,119,16,1}},
#endif
	far NewItems=	{NM_X,NM_Y,4,1,0,16,		{60,-2,105,16,1}}
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
	;
#else
	,
#ifdef GAMEVER_RESTORATION_AOG_POST200
	far SwitchItems=	{MENU_X,MENU_Y+15,6,0,0,9,{87,-1,152,7,1}};
#elif (defined GAMEVER_RESTORATION_AOG)
	far SwitchItems=	{MENU_X,MENU_Y+25,5,0,0,9,{87,-1,132,7,1}};
#else
	far SwitchItems=	{MENU_X,MENU_Y+25,4,0,0,9,{87,-1,132,7,1}};
#endif
#endif



//#pragma warn -sus

CP_itemtype far MainMenu[]=
{
	{AT_ENABLED,"NEW MISSION",(void (*)(int16_t))CP_NewGame,COAL_FONT},
	{AT_READIT,"ORDERING INFO",(void (*)(int16_t))CP_OrderingInfo},
	{AT_READIT,"INSTRUCTIONS",(void (*)(int16_t))CP_ReadThis},
	{AT_ENABLED,"STORY",(void (*)(int16_t))CP_BlakeStoneSaga},
	{AT_DISABLED,"",0},
	{AT_ENABLED,"GAME OPTIONS",(void (*)(int16_t))CP_GameOptions},
	{AT_ENABLED,"HIGH SCORES",(void (*)(int16_t))CP_ViewScores},
	{AT_ENABLED,"LOAD MISSION",(void (*)(int16_t))CP_LoadGame},
	{AT_DISABLED,"SAVE MISSION",(void (*)(int16_t))CP_SaveGame},
	{AT_DISABLED,"",0},
	{AT_ENABLED,"BACK TO DEMO",(void (*)(int16_t))CP_ExitOptions},
	{AT_ENABLED,"LOGOFF",0}
},

far GopMenu[]=
{
	{AT_ENABLED,"SOUND",(void (*)(int16_t))CP_Sound},
	{AT_ENABLED,"CONTROLS",(void (*)(int16_t))CP_Control},
	{AT_ENABLED,"CHANGE VIEW",(void (*)(int16_t))CP_ChangeView},
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	{AT_ENABLED,"SWITCHES",(void (*)(int16_t))CP_Switches},
#endif
},

far SndMenu[]=
{
 {AT_ENABLED,"NONE",0},
 {AT_ENABLED,"PC SPEAKER",0},
 {AT_ENABLED,"ADLIB/SOUND BLASTER",0},
 {AT_DISABLED,"",0},
 {AT_DISABLED,"",0},
 {AT_ENABLED,"NONE",0},
 {AT_ENABLED,"DISNEY SOUND SOURCE",0},
 {AT_ENABLED,"SOUND BLASTER",0},
 {AT_DISABLED,"",0},
 {AT_DISABLED,"",0},
 {AT_ENABLED,"NONE",0},
 {AT_ENABLED,"ADLIB/SOUND BLASTER",0}
},

far CtlMenu[]=
{
 {AT_DISABLED,"MOUSE ENABLED",0},
 {AT_DISABLED,"JOYSTICK ENABLED",0},
 {AT_DISABLED,"USE JOYSTICK PORT 2",0},
 {AT_DISABLED,"GRAVIS GAMEPAD ENABLED",0},
 {AT_DISABLED,"CALIBRATE JOYSTICK",0},
 {AT_DISABLED,"MOUSE SENSITIVITY",(void (*)(int16_t))MouseSensitivity},
 {AT_ENABLED,"CUSTOMIZE CONTROLS",(void (*)(int16_t))CustomControls}
},

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
far SwitchMenu[]=
{
#ifdef GAMEVER_RESTORATION_AOG
 {AT_ENABLED,"LIGHTING",0},
 {AT_ENABLED,"CEILING TEXTURES",0},
 {AT_ENABLED,"FLOOR TEXTURES",0},
 {AT_ENABLED,"HEART BEAT SOUND",0},
 {AT_ENABLED,"REBA ATTACK INFO",0},
#ifdef GAMEVER_RESTORATION_AOG_POST200
 {AT_ENABLED,"<TAB> SHOWS ROTATED MAP",0},
#endif
#else
 {AT_ENABLED,"LIGHTING",0},
 {AT_ENABLED,"REBA ATTACK INFO",0},
 {AT_ENABLED,"SHOW CEILINGS",0},
 {AT_ENABLED,"SHOW FLOORS",0}
#endif
},
#endif


//#pragma warn +sus

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
far NewEmenu[]=
{
 {AT_ENABLED,"MISSION 1:\n"
				 "STAR INSTITUTE",0},

 {AT_NON_SELECTABLE,"MISSION 2:\n"
				  "FLOATING FORTRESS",0},

 {AT_NON_SELECTABLE,"MISSION 3:\n"
				  "UNDERGROUND NETWORK",0},

 {AT_NON_SELECTABLE,"MISSION 4:\n"
				  "STAR PORT",0},

 {AT_NON_SELECTABLE,"MISSION 5:\n"
				  "HABITAT II",0},

 {AT_NON_SELECTABLE,"MISSION 6:\n"
				  "SATELLITE DEFENSE",0},
},
#endif

far NewMenu[]=
{
 {AT_ENABLED,"LEVEL 1:\n"
				 "NOVICE AGENT",0},
 {AT_ENABLED,"LEVEL 2:\n"
				 "SKILLED AGENT",0},
 {AT_ENABLED,"LEVEL 3:\n"
				 "EXPERT AGENT",0},
 {AT_ENABLED,"LEVEL 4:\n"
				 "VETERAN AGENT",0}
},

far LSMenu[]=
{
 {AT_ENABLED,"",0},
 {AT_ENABLED,"",0},
 {AT_ENABLED,"",0},
 {AT_ENABLED,"",0},
 {AT_ENABLED,"",0},
 {AT_ENABLED,"",0},
 {AT_ENABLED,"",0},
 {AT_ENABLED,"",0},
 {AT_ENABLED,"",0},
 {AT_ENABLED,"",0}
},

far CusMenu[]=
{
 {AT_ENABLED,"",0},
 {0,"",0},
 {AT_ENABLED,"",0},
 {0,"",0},
 {AT_ENABLED,"",0},
 {AT_ENABLED,"",0}
}
;


int16_t color_hlite[]=
{
	HIGHLIGHT_DISABLED_COLOR,
	HIGHLIGHT_TEXT_COLOR,
	READHCOLOR,
	HIGHLIGHT_DEACTIAVED_COLOR,
};

int16_t color_norml[]=
{
	DISABLED_TEXT_COLOR,
	ENABLED_TEXT_COLOR,
	READCOLOR,
	DEACTIAVED_TEXT_COLOR,
};

int16_t EpisodeSelect[6]={1};

int16_t SaveGamesAvail[10],StartGame,SoundStatus=1,pickquick;
char far SaveGameNames[10][GAME_DESCRIPTION_LEN+1],far SaveName[13]="SAVEGAM?.";



////////////////////////////////////////////////////////////////////
//
// INPUT MANAGER SCANCODE TABLES
//
////////////////////////////////////////////////////////////////////

#ifndef CACHE_KEY_DATA

static byte
					far * far ScanNames[] =		// Scan code names with single chars
					{
	"?","?","1","2","3","4","5","6","7","8","9","0","-","+","?","?",
	"Q","W","E","R","T","Y","U","I","O","P","[","]","|","?","A","S",
	"D","F","G","H","J","K","L",";","\"","?","?","?","Z","X","C","V",
	"B","N","M",",",".","/","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","\xf","?","-","\x15","5","\x11","+","?",
	"\x13","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?"
					},	// DEBUG - consolidate these
					far ExtScanCodes[] =	// Scan codes with >1 char names
					{
	1,0xe,0xf,0x1d,0x2a,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,
	0x3f,0x40,0x41,0x42,0x43,0x44,0x57,0x59,0x46,0x1c,0x36,
	0x37,0x38,0x47,0x49,0x4f,0x51,0x52,0x53,0x45,0x48,
	0x50,0x4b,0x4d,0x00
					},
					far * far ExtScanNames[] =	// Names corresponding to ExtScanCodes
					{
	"ESC","BKSP","TAB","CTRL","LSHFT","SPACE","CAPSLK","F1","F2","F3","F4",
	"F5","F6","F7","F8","F9","F10","F11","F12","SCRLK","ENTER","RSHFT",
	"PRTSC","ALT","HOME","PGUP","END","PGDN","INS","DEL","NUMLK","UP",
	"DOWN","LEFT","RIGHT",""
					};

#else

byte far *ScanNames, far * ExtScanNames;
ScanCode far *ExtScanCodes;

#endif

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
static byte special_keys[] =
{
#ifdef GAMEVER_RESTORATION_AOG
sc_L,sc_P,sc_M,sc_S,sc_I,sc_H,sc_C,sc_F,sc_Q,sc_W,sc_E,sc_Enter,sc_1,sc_2,sc_3,sc_4,sc_5,sc_Tab
#else
sc_Tilde,sc_Plus,sc_Minus,sc_L,sc_P,sc_M,sc_S,sc_I,sc_Q,sc_W,sc_E,sc_Enter,sc_1,sc_2,sc_3,sc_4,sc_5,sc_Tab
#endif
};
#endif




//-------------------------------------------------------------------------
// HelpScreens()
//-------------------------------------------------------------------------
void HelpScreens()
{
#ifndef ID_CACHE_HELP
	HelpPresenter("HELP.TXT",false,0,true);
#else
	HelpPresenter(NULL,false,HELPTEXT,true);
#endif
}


//-------------------------------------------------------------------------
// HelpPresenter()
//-------------------------------------------------------------------------
void HelpPresenter(char *fname,boolean continue_keys, uint16_t id_cache, boolean startmusic)
{
	#define FULL_VIEW_WIDTH			19

	PresenterInfo pi;
	int16_t oldwidth;

	memset(&pi,0,sizeof(pi));

	pi.flags = TPF_SHOW_PAGES;
	if (continue_keys)
		pi.flags |= TPF_CONTINUE;

	VW_FadeOut();

// Change view size to MAX! (scaler clips shapes on smaller views)
//
	oldwidth = viewwidth/16;
	if (oldwidth != FULL_VIEW_WIDTH)
		NewViewSize(FULL_VIEW_WIDTH);

// Draw help border
//
	CacheLump(H_TOPWINDOWPIC,H_BOTTOMINFOPIC);
	VWB_DrawPic (0,0,H_TOPWINDOWPIC);
	VWB_DrawPic (0,8,H_LEFTWINDOWPIC);
	VWB_DrawPic (312,8,H_RIGHTWINDOWPIC);
	VWB_DrawPic (8,176,H_BOTTOMINFOPIC);
	UnCacheLump(H_TOPWINDOWPIC,H_BOTTOMINFOPIC);

// Setup for text presenter
//
	pi.xl=8;
	pi.yl=8;
	pi.xh=311;
	pi.yh=175;
	pi.ltcolor=0x7b;
	pi.bgcolor=0x7d;
	pi.dkcolor=0x7f;
	pi.shcolor=0x00;
	pi.fontnumber=4;

	if (continue_keys)
		pi.infoline = (char far *)" UP / DN - PAGES       ENTER - CONTINUES         ESC - EXITS";
	else
		pi.infoline = (char far *)"           UP / DN - PAGES            ESC - EXITS";

	if (startmusic)
		StartCPMusic(TEXTSONG);

// Load, present, and free help text.
//
	TP_LoadScript(fname,&pi,id_cache);
	TP_Presenter(&pi);
	TP_FreeScript(&pi,id_cache);

	MenuFadeOut();

// Reset view size
//
	if (oldwidth != FULL_VIEW_WIDTH)
		NewViewSize(oldwidth);

	if (startmusic && TPscan==sc_Escape)
		StartCPMusic(MENUSONG);
	IN_ClearKeysDown();
}

//--------------------------------------------------------------------------
// US_ControlPanel() - Control Panel!  Ta Da!
//--------------------------------------------------------------------------
void US_ControlPanel(byte scancode)
{
	int16_t which;

#ifdef CACHE_KEY_DATA
 CA_CacheGrChunk(SCANNAMES_DATA);
 CA_CacheGrChunk(EXTSCANNAMES_DATA);
 CA_CacheGrChunk(EXTSCANCODES_DATA);

 ScanNames = grsegs[SCANNAMES_DATA];
 ExtScanNames = grsegs[EXTSCANNAMES_DATA];
 ExtScanCodes = grsegs[EXTSCANCODES_DATA];
#else

//
// This code doesn't correctly save the table data -- it saves garbage
// for SCANNAMES and EXTSCANCODES...
//

//	IO_WriteFile("SCANNAME.BIN",ScanNames,sizeof(ScanNames));
//	IO_WriteFile("EXTSCNAM.BIN",ExtScanNames,sizeof(ExtScanNames));
//	IO_WriteFile("EXTSCCOD.BIN",ExtScanCodes,sizeof(ExtScanCodes));

#endif

	if (ingame)
		if (CP_CheckQuick(scancode))
			return;

	SetupControlPanel();
	StartCPMusic(MENUSONG);

 //
 // F-KEYS FROM WITHIN GAME
 //
 switch(scancode)
 {
  case sc_F1:
	 CleanupControlPanel();
	 HelpScreens();
	 return;

  case sc_F2:
	 CP_SaveGame(0);
    goto finishup;

  case sc_F3:
	 CP_LoadGame(0);
//	 refresh_screen=false;
    goto finishup;

  case sc_F4:
    CP_Sound();
	 goto finishup;

  case sc_F5:
    CP_ChangeView();
    goto finishup;

  case sc_F6:
    CP_Control();
    goto finishup;

  finishup:
	 CleanupControlPanel();
	 return;
 }


 DrawMainMenu();
 MenuFadeIn();
 StartGame=0;

 //
 // MAIN MENU LOOP
 //
 do
 {
  which=HandleMenu(&MainItems,&MainMenu[0],NULL);

  switch(which)
  {
	case MM_VIEW_SCORES:
	  if (MainMenu[MM_VIEW_SCORES].routine == NULL)
		 if (CP_EndGame())
			 StartGame=1;

	  DrawMainMenu();
	  MenuFadeIn();
	  break;

	case -1:
	case MM_LOGOFF:
	  CP_Quit();
	  break;

	default:
	  if (!StartGame)
	  {
		DrawMainMenu();
		MenuFadeIn();
	  }
  }

 //
 // "EXIT OPTIONS" OR "NEW GAME" EXITS
 //
 } while(!StartGame);

 //
 // DEALLOCATE EVERYTHING
 //
 CleanupControlPanel();
 if (!loadedgame)
	 StopMusic();


 //
 // CHANGE MAINMENU ITEM
 //
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
 if (startgame)
#else
 if (startgame || loadedgame)
#endif
 {
  //#pragma warn -sus
  MainMenu[MM_VIEW_SCORES].routine = NULL;
  _fstrcpy(MainMenu[MM_VIEW_SCORES].string,"END GAME");
  //#pragma warn +sus
 }

 if (ingame && loadedgame)
	refresh_screen=false;


#ifdef CACHE_KEY_DATA
 FREEFONT(SCANNAMES_DATA);
 FREEFONT(EXTSCANNAMES_DATA);
 FREEFONT(EXTSCANCODES_DATA);
#endif

 // RETURN/START GAME EXECUTION
}

//--------------------------------------------------------------------------
// DrawMainMenu(void)
//--------------------------------------------------------------------------
void DrawMainMenu(void)
{
	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
	ControlPanelFree();
#endif
	CA_CacheScreen (BACKGROUND_SCREENPIC);
	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
	ControlPanelAlloc();
#endif

	ClearMScreen();
	DrawMenuTitle("MAIN OPTIONS");
	DrawInstructions(IT_STANDARD);

 //
 // CHANGE "MISSION" AND "DEMO"
 //
	if (ingame)
	{
		_fstrcpy(&MainMenu[MM_BACK_TO_DEMO].string[8],"MISSION");
		MainMenu[MM_BACK_TO_DEMO].active=AT_READIT;
	}
	else
	{
		_fstrcpy(&MainMenu[MM_BACK_TO_DEMO].string[8],"DEMO");
		MainMenu[MM_BACK_TO_DEMO].active=AT_ENABLED;
	}

	fontnumber = 4;				// COAL

	DrawMenu(&MainItems,&MainMenu[0]);

	VW_UpdateScreen();
}

//--------------------------------------------------------------------------
// READ THIS!
//--------------------------------------------------------------------------
void CP_ReadThis(void)
{
	ControlPanelFree();
	HelpScreens();
	ControlPanelAlloc();
}


//--------------------------------------------------------------------------
// CP_OrderingInfo()
//--------------------------------------------------------------------------
void CP_OrderingInfo(void)
{
	ControlPanelFree();
#ifndef ID_CACHE_HELP
	HelpPresenter("ORDER.TXT",false,0,true);
#else
	HelpPresenter(NULL,false,ORDERTEXT,true);
#endif
	ControlPanelAlloc();
}


//-------------------------------------------------------------------------
// CP_BlakeStoneSaga()
//-------------------------------------------------------------------------
void CP_BlakeStoneSaga()
{
	ControlPanelFree();
#ifndef ID_CACHE_HELP
	HelpPresenter("SAGA.TXT",false,0,true);
#else
	HelpPresenter(NULL,false,SAGATEXT,true);
#endif
	ControlPanelAlloc();
}

//--------------------------------------------------------------------------
// CP_CheckQuick() - CHECK QUICK-KEYS & QUIT (WHILE IN A GAME)
//--------------------------------------------------------------------------
int16_t CP_CheckQuick(uint16_t scancode)
{
	switch(scancode)
	{
	// END GAME
	//
		case sc_F7:
			VW_ScreenToScreen (displayofs,bufferofs,80,160);
			CA_CacheGrChunk(STARTFONT+1);

			WindowH=160;
			if (Confirm(ENDGAMESTR))
			{
				playstate = ex_died;
				pickquick = gamestate.lives = 0;
			}

			WindowH=200;
			fontnumber=4;
			return(1);

	// QUICKSAVE
	//
		case sc_F8:
			if (SaveGamesAvail[LSItems.curpos] && pickquick)
			{
				char string[100]="Quick Save will overwrite:\n\"";

				CA_CacheGrChunk(STARTFONT+1);

				_fstrcat(string,SaveGameNames[LSItems.curpos]);
				strcat(string,"\"?");
				VW_ScreenToScreen (displayofs,bufferofs,80,160);

#if IN_DEVELOPMENT
				if (TestQuickSave || Confirm(string))
				{
            	if (TestQuickSave)
               	TestQuickSave--;
#else
				if (Confirm(string))
            {
#endif
					CA_CacheGrChunk(STARTFONT+1);
					CP_SaveGame(1);
					fontnumber=4;
				}
				else
					refresh_screen=false;
			}
			else
			{
				CA_CacheGrChunk(STARTFONT+1);

				VW_FadeOut ();

				StartCPMusic(MENUSONG);
				pickquick=CP_SaveGame(0);

				lasttimecount = TimeCount;
#ifdef __AMIGA__
				//LL_StartMouse();
				int16_t dummyX, dummyY;
				BE_ST_GetMouseDelta(&dummyX, &dummyY);
#else
				if (MousePresent)
					Mouse(MDelta);						// Clear accumulated mouse movement
#endif
			}

			return(1);

	// QUICKLOAD
	//
		case sc_F9:
			if (SaveGamesAvail[LSItems.curpos] && pickquick)
			{
				char string[100]="Quick Load:\n\"";

				CA_CacheGrChunk(STARTFONT+1);

				_fstrcat(string,SaveGameNames[LSItems.curpos]);
				strcat(string,"\"?");
				VW_ScreenToScreen (displayofs,bufferofs,80,160); 

				if (Confirm(string))
					CP_LoadGame(1);
				else
				{
					refresh_screen=false;
					return(1);
				}

				fontnumber=4;
			}
			else
			{
				CA_CacheGrChunk(STARTFONT+1);

				VW_FadeOut ();

				StartCPMusic(MENUSONG);
				pickquick=CP_LoadGame(0);

				lasttimecount = TimeCount;
#ifdef __AMIGA__
				//LL_StartMouse();
				int16_t dummyX, dummyY;
				BE_ST_GetMouseDelta(&dummyX, &dummyY);
#else
				if (MousePresent)
					Mouse(MDelta);	// Clear accumulated mouse movement
#endif
				PM_CheckMainMem ();
			}

			if (pickquick)
				refresh_screen=false;
			return(1);

	// QUIT
	//
		case sc_F10:
			CA_CacheGrChunk(STARTFONT+1);
			VW_ScreenToScreen (displayofs,bufferofs,80,160);

			WindowX=WindowY=0;
			WindowW=320;
			WindowH=160;
			if (Confirm(QuitToDosStr))
				ExitGame();

			refresh_screen=false;
			WindowH=200;
			fontnumber=4;

			return(1);
	}

	return(0);
}

//--------------------------------------------------------------------------
// END THE CURRENT GAME
//--------------------------------------------------------------------------
int16_t CP_EndGame(void)
{
	if (!Confirm(ENDGAMESTR))
		return 0;

	pickquick = gamestate.lives = 0;
	playstate = ex_died;
/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
   InstantQuit = 1;
#endif

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
//#if 0
//#pragma warn -sus
	 MainMenu[MM_VIEW_SCORES].routine=&CP_ViewScores;
	_fstrcpy(MainMenu[MM_VIEW_SCORES].string,"HIGH SCORES");
//#pragma warn +sus
#endif

	return 1;
}

//--------------------------------------------------------------------------
// CP_ViewScores() - VIEW THE HIGH SCORES
//--------------------------------------------------------------------------
void CP_ViewScores(void)
{
	fontnumber=4;
	StartCPMusic(ROSTER_MUS);
	DrawHighScores ();
	VW_UpdateScreen ();
	MenuFadeIn();
	fontnumber=1;

	IN_Ack();

	StartCPMusic(MENUSONG);
	MenuFadeOut();
}

//--------------------------------------------------------------------------
// CP_NewGame() - START A NEW GAME
//--------------------------------------------------------------------------
void CP_NewGame(void)
{
	int16_t which,episode;

	DrawMenuTitle("Difficulty Level");
	DrawInstructions(IT_STANDARD);


/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG

firstpart:

 DrawNewEpisode();
 do
 {
  which=HandleMenu(&NewEitems,&NewEmenu[0],DrawEpisodePic);
  switch(which)
  {
	case -1:
	 MenuFadeOut();
	 return;

	default:
	 if (!EpisodeSelect[which])
	 {
	  SD_PlaySound (NOWAYSND);
	  CacheMessage(READTHIS_TEXT);
	  IN_ClearKeysDown();
	  IN_Ack();
	  VL_Bar(35,69,250,62,TERM_BACK_COLOR);
	  DrawNewEpisode();
	  which = 0;
	 }
	 else
	 {
	  episode = which;
	  which = 1;
	 }
	 break;
  }

 } while (!which);

 ShootSnd();
#else
 episode = 0;
#endif

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
 //
 // ALREADY IN A GAME?
 //
	if (ingame)
		if (!Confirm(CURGAME))
		{
			MenuFadeOut();
			return;
		}
#endif

secondpart:

	MenuFadeOut();
	if (ingame)
		CA_CacheScreen(BACKGROUND_SCREENPIC);
	DrawNewGame();
	which=HandleMenu(&NewItems,&NewMenu[0],DrawNewGameDiff);

 if (which<0)
 {
  MenuFadeOut();
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
  goto firstpart;
#else
  return;
#endif
 }

 ShootSnd();
 MenuFadeOut();
 ControlPanelFree();

 if (Breifing(BT_INTRO,episode))
 {
	CA_CacheScreen (BACKGROUND_SCREENPIC);
	ControlPanelAlloc();
	goto secondpart;
 }

 StartGame=1;
 NewGame(which,episode);

 //
 // CHANGE "READ THIS!" TO NORMAL COLOR
 //
 MainMenu[MM_READ_THIS].active=AT_ENABLED;

}

//---------------------------------------------------------------------------
// DrawMenuTitle() - Draws the menu title
//---------------------------------------------------------------------------
void DrawMenuTitle(char *title)
{

	fontnumber = 3;
	CA_CacheGrChunk(STARTFONT+3);

	PrintX = WindowX = 32;
	PrintY = WindowY = 32;
	WindowW = 244;
	WindowH = 20;

	SETFONTCOLOR(TERM_SHADOW_COLOR,TERM_BACK_COLOR);
	US_PrintCentered(title);

	WindowX = 32-1;
	WindowY = 32-1;

	SETFONTCOLOR(ENABLED_TEXT_COLOR,TERM_BACK_COLOR);
	US_PrintCentered(title);

	FREEFONT(STARTFONT+3);

}

//---------------------------------------------------------------------------
// DrawInstructions() - Draws instructions centered at the bottom of
//								the view screen.
//
// NOTES: Orginal font number or font color is not maintained.
//---------------------------------------------------------------------------
void DrawInstructions(inst_type Type)
{
	#define INSTRUCTIONS_Y_POS		154+10

	char *instr[MAX_INSTRUCTIONS] = {"UP/DN SELECTS - ENTER CHOOSES - ESC EXITS",
												 "PRESS ANY KEY TO CONTINUE",
												 "ENTER YOUR NAME AND PRESS ENTER",
												 "RT/LF ARROW SELECTS - ENTER CHOOSES"};

	fontnumber = 2;

	WindowX = 48;
   WindowY = INSTRUCTIONS_Y_POS;
	WindowW = 236;
   WindowH = 8;

   VWB_Bar(WindowX,WindowY-1,WindowW,WindowH,TERM_BACK_COLOR);

	SETFONTCOLOR(TERM_SHADOW_COLOR,TERM_BACK_COLOR);
	US_PrintCentered(instr[Type]);

   WindowX--;
   WindowY--;

	SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR,TERM_BACK_COLOR);
	US_PrintCentered(instr[Type]);
}

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG

//--------------------------------------------------------------------------
// DrawNewEpisode() - DRAW NEW EPISODE MENU
//--------------------------------------------------------------------------
void DrawNewEpisode(void)
{
	ClearMScreen();

	DrawMenuTitle("CHOOSE A MISSION");
	DrawInstructions(IT_STANDARD);

	PrintY=51;
	WindowX=58;

	fontnumber = 2;							// six point font
	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
	DrawMenu(&NewEitems,&NewEmenu[0]);
	//DrawMenu(&NewEitems,&NewEmenu7[0]);

	DrawEpisodePic(NewEitems.curpos);

	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();

}

#endif

//--------------------------------------------------------------------------
// DrawNewGame() - DRAW NEW GAME MENU
//--------------------------------------------------------------------------
void DrawNewGame(void)
{
	ClearMScreen();
	DrawMenuTitle("DIFFICULTY LEVEL");
	DrawInstructions(IT_STANDARD);

	fontnumber = 2;							// six point font
	DrawMenu(&NewItems,&NewMenu[0]);

	DrawNewGameDiff(NewItems.curpos);

	px=48;
	py=INSTRUCTIONS_Y_POS-24;
	ShPrint("        HIGHER DIFFICULTY LEVELS CONTAIN",TERM_SHADOW_COLOR,false);

	px=48;
	py+=6;
	ShPrint("            MORE, STRONGER ENEMIES",TERM_SHADOW_COLOR,false);


	VW_UpdateScreen();

	MenuFadeIn();
	WaitKeyUp();
}

//--------------------------------------------------------------------------
// DRAW NEW GAME GRAPHIC
//--------------------------------------------------------------------------
void DrawNewGameDiff(int16_t w)
{
	VWB_DrawPic(192,77,w+C_BABYMODEPIC);
}

//--------------------------------------------------------------------------
// DRAW NEW GAME GRAPHIC
//--------------------------------------------------------------------------
void DrawEpisodePic(int16_t w)
{
	VWB_DrawPic(176,72,w+C_EPISODE1PIC);
}

//--------------------------------------------------------------------------
// CP_GameOptions() - DRAW THE GAME OPTIONS MENU
//--------------------------------------------------------------------------
void CP_GameOptions(void)
{
	int16_t which,i;

	CA_CacheScreen (BACKGROUND_SCREENPIC);
	DrawGopMenu();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which=HandleMenu(&GopItems,&GopMenu[0],NULL);

		if (which != -1)
		{
			DrawGopMenu();
			MenuFadeIn();
		}

	} while(which>=0);

	MenuFadeOut();
}

//--------------------------------------------------------------------------
// DrawGopMenu(void)
//--------------------------------------------------------------------------
void DrawGopMenu(void)
{
	CA_CacheScreen (BACKGROUND_SCREENPIC);

	ClearMScreen();
	DrawMenuTitle("GAME OPTIONS");
	DrawInstructions(IT_STANDARD);

	fontnumber = 4;				// COAL

	DrawMenu(&GopItems,&GopMenu[0]);

	VW_UpdateScreen();
}


/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
void ChangeSwaps(void)
{
	WindowX=WindowY=0;
	WindowW=320;
	WindowH=200;
   Message(Computing);

	PM_Shutdown();
	PM_Startup ();
	ClearMemory();
	ControlPanelAlloc();

   IN_UserInput(50);
	IN_ClearKeysDown();

}
#endif

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
//--------------------------------------------------------------------------
// GAME SWITCHES MENU
//--------------------------------------------------------------------------
void CP_Switches(void)
{
	int16_t which,i;

	CA_CacheScreen (BACKGROUND_SCREENPIC);
	DrawSwitchMenu();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which=HandleMenu(&SwitchItems,&SwitchMenu[0],DrawAllSwitchLights);

		switch (which)
		{
			/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
			// Bits of different code for AOG
			case	SW_LIGHTING:
				gamestate.flags ^= GS_LIGHTING;
				ShootSnd();
				DrawSwitchMenu();
			break;

#ifndef GAMEVER_RESTORATION_AOG
			case	SW_REBA_ATTACK_INFO:
				gamestate.flags ^= GS_ATTACK_INFOAREA;
				ShootSnd();
				DrawSwitchMenu();
			break;
#endif

			case	SW_CEILING:
#ifdef GAMEVER_RESTORATION_AOG
				if (IsA386)
				{
#endif
					gamestate.flags ^= GS_DRAW_CEILING;
					ShootSnd();
					DrawSwitchMenu();
#ifdef GAMEVER_RESTORATION_AOG
					break;
				}
#endif
			break;

			case	SW_FLOORS:
#ifdef GAMEVER_RESTORATION_AOG
				if (IsA386)
				{
#endif
					gamestate.flags ^= GS_DRAW_FLOOR;
					ShootSnd();
#ifdef GAMEVER_RESTORATION_AOG
					ChangeSwaps();
#endif
					DrawSwitchMenu();
#ifdef GAMEVER_RESTORATION_AOG
					break;
				}
#endif
			break;
#ifdef GAMEVER_RESTORATION_AOG

			case	SW_HEART_BEAT:
				gamestate.flags ^= GS_HEARTB_SOUND;
				ShootSnd();
				DrawSwitchMenu();
			break;

			case	SW_REBA_ATTACK_INFO:
				gamestate.flags ^= GS_ATTACK_INFOAREA;
				ShootSnd();
				DrawSwitchMenu();
			break;
#ifdef GAMEVER_RESTORATION_AOG_POST200

			case	SW_AUTO_MAP:
				gamestate.flags ^= GS_TAB_ROTATED_MAP;
				ShootSnd();
				DrawSwitchMenu();
			break;
#endif
#endif
		}

	} while(which>=0);

	MenuFadeOut();
}

//--------------------------------------------------------------------------
// DrawSwitchMenu(void)
//--------------------------------------------------------------------------
void DrawSwitchMenu(void)
{
	CA_CacheScreen (BACKGROUND_SCREENPIC);

	ClearMScreen();
	DrawMenuTitle("GAME SWITCHES");
	DrawInstructions(IT_STANDARD);
	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
	SwitchMenu[SW_CEILING].active = SwitchMenu[SW_FLOORS].active = IsA386;
#endif

	fontnumber = 2;

	DrawMenu(&SwitchItems,&SwitchMenu[0]);
	DrawAllSwitchLights(SwitchItems.curpos);

	VW_UpdateScreen();
}

//--------------------------------------------------------------------------
// DrawAllSwitchLights()
//--------------------------------------------------------------------------
void DrawAllSwitchLights(int16_t which)
{
	int16_t i;
	uint16_t Shape;

	for (i=0;i<SwitchItems.amount;i++)
		if (SwitchMenu[i].string[0])
		{
			Shape = C_NOTSELECTEDPIC;

			//
			// DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
			//

			if (SwitchItems.cursor.on)
				if (i == which)			// Is the cursor sitting on this pic?
					Shape +=2;

			switch(i)
			{
				/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
				// Bits of different code for AOG
				case SW_LIGHTING:
					if (gamestate.flags & GS_LIGHTING)
						Shape++;
				break;

#ifndef GAMEVER_RESTORATION_AOG
				case SW_REBA_ATTACK_INFO:
					if (gamestate.flags & GS_ATTACK_INFOAREA)
						Shape++;
				break;
#endif

				case SW_CEILING:
					if (gamestate.flags & GS_DRAW_CEILING)
						Shape++;
				break;

				case SW_FLOORS:
					if (gamestate.flags & GS_DRAW_FLOOR)
						Shape++;
				break;
#ifdef GAMEVER_RESTORATION_AOG

				case SW_HEART_BEAT:
					if (gamestate.flags & GS_HEARTB_SOUND)
						Shape++;
				break;

				case SW_REBA_ATTACK_INFO:
					if (gamestate.flags & GS_ATTACK_INFOAREA)
						Shape++;
				break;
#ifdef GAMEVER_RESTORATION_AOG_POST200

				case SW_AUTO_MAP:
					if (gamestate.flags & GS_TAB_ROTATED_MAP)
						Shape++;
				break;
#endif
#endif
			}

			VWB_DrawPic(SwitchItems.x-16,SwitchItems.y+i*SwitchItems.y_spacing-1,Shape);
		}

	DrawSwitchDescription(which);

}


//--------------------------------------------------------------------------
//  DrawSwitchDescription()
//--------------------------------------------------------------------------
void DrawSwitchDescription(int16_t which)
{
	#define DESCRIPTIONS_Y_POS		134

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
	char *instr[] = {"TOGGLES LIGHT SOURCING IN HALLWAYS",
						  "TOGGLES GRAPHICS DRAWN FOR CEILINGS",
						  "TOGGLES GRAPHICS DRAWN FOR FLOOR",
						  "TOGGLES HEART BEAT SOUND WITH EKG",
						  "TOGGLES DETAILED ATTACKER INFO"
#ifdef GAMEVER_RESTORATION_AOG_POST200
						   ,
						  "SWAPS THE <TAB> / <SHIFT TAB> FUNCTIONS"
#endif
	};
#else
	char *instr[] = {"TOGGLES LIGHT SOURCING IN HALLWAYS",
						  "TOGGLES DETAILED ATTACKER INFO",
						  "TOGGLES CEILING MAPPING",
						  "TOGGLES FLOOR MAPPING"};
#endif

	fontnumber = 2;

	WindowX = 48;
	WindowY = DESCRIPTIONS_Y_POS;
	WindowW = 236;
	WindowH = 8;

	VWB_Bar(WindowX,WindowY-1,WindowW,WindowH,TERM_BACK_COLOR);

	SETFONTCOLOR(TERM_SHADOW_COLOR,TERM_BACK_COLOR);
	US_PrintCentered(instr[which]);

	WindowX--;
	WindowY--;

	SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR,TERM_BACK_COLOR);
	US_PrintCentered(instr[which]);
}
#endif



//--------------------------------------------------------------------------
// HANDLE SOUND MENU
//--------------------------------------------------------------------------
void CP_Sound(void)
{
	int16_t which,i;

	CA_CacheScreen (BACKGROUND_SCREENPIC);
 DrawSoundMenu();
 MenuFadeIn();
 WaitKeyUp();

 do
 {
  which=HandleMenu(&SndItems,&SndMenu[0],DrawAllSoundLights);
  //
  // HANDLE MENU CHOICES
  //
  switch(which)
  {
	//
	// SOUND EFFECTS
	//
	case 0:
	 if (SoundMode!=sdm_Off)
	 {
	  SD_WaitSoundDone();
	  SD_SetSoundMode(sdm_Off);
	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
	  SD_SetDigiDevice(sds_Off);
#endif
	  DrawSoundMenu();
	 }
	 break;

	case 1:
	  if (SoundMode!=sdm_PC)
	  {
		SD_WaitSoundDone();
		SD_SetSoundMode(sdm_PC);
		CA_LoadAllSounds();
		DrawSoundMenu();
		ShootSnd();
	  }
	  break;

	case 2:
	  if (SoundMode!=sdm_AdLib)
	  {
		SD_WaitSoundDone();
		SD_SetSoundMode(sdm_AdLib);
		CA_LoadAllSounds();
		DrawSoundMenu();
		ShootSnd();
	  }
	  break;

	//
	// DIGITIZED SOUND
	//
	case 5:
	  if (DigiMode!=sds_Off)
	  {
		SD_SetDigiDevice(sds_Off);
		DrawSoundMenu();
	  }
	  break;

	case 6:
	  if (DigiMode!=sds_SoundSource)
	  {
      /*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
      if (SoundMode==sdm_Off)
			SD_SetSoundMode(sdm_PC);
#endif
		SD_SetDigiDevice(sds_SoundSource);
		/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
		CA_LoadAllSounds();
#endif
		DrawSoundMenu();
		ShootSnd();
	  }
	  break;

	case 7:
	  if (DigiMode!=sds_SoundBlaster)
	  {
      /*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
      if (SoundMode==sdm_Off)
			SD_SetSoundMode(sdm_PC);
#endif
		SD_SetDigiDevice(sds_SoundBlaster);
		/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
		CA_LoadAllSounds();
#endif
		DrawSoundMenu();
	   ShootSnd();
	  }
	  break;

	//
	// MUSIC
	//
	case 10:
	  if (MusicMode!=smm_Off)
	  {
		SD_SetMusicMode(smm_Off);
		DrawSoundMenu();
		ShootSnd();
	  }
	  break;

	case 11:
	  if (MusicMode!=smm_AdLib)
	  {
		SD_SetMusicMode(smm_AdLib);
		DrawSoundMenu();
		ShootSnd();
		StartCPMusic(MENUSONG);
	  }
	  break;
  }
 } while(which>=0);

 MenuFadeOut();
}

//--------------------------------------------------------------------------
// DrawSoundMenu() - DRAW THE SOUND MENU
//--------------------------------------------------------------------------
void DrawSoundMenu(void)
{
	int16_t i,on;

	//
	// DRAW SOUND MENU
	//

	ClearMScreen();
	DrawMenuTitle("SOUND SETTINGS");
	DrawInstructions(IT_STANDARD);

	//
	// IF NO ADLIB, NON-CHOOSENESS!
	//

	if (!AdLibPresent && !SoundBlasterPresent)
	{
		SndMenu[2].active=SndMenu[10].active=SndMenu[11].active=AT_DISABLED;
	}

	if (!SoundSourcePresent)
		SndMenu[6].active=AT_DISABLED;

	if (!SoundBlasterPresent)
		SndMenu[7].active=AT_DISABLED;

	if (!SoundSourcePresent && !SoundBlasterPresent)
		SndMenu[5].active=AT_DISABLED;

	fontnumber = 4;

	SETFONTCOLOR(DISABLED_TEXT_COLOR,TERM_BACK_COLOR);
	ShadowPrint("SOUND EFFECTS",105,52);
	ShadowPrint("DIGITIZED SOUNDS",105,87);
	ShadowPrint("BACKGROUND MUSIC",105,121);

	fontnumber = 2;
	DrawMenu(&SndItems,&SndMenu[0]);


	DrawAllSoundLights(SndItems.curpos);

	VW_UpdateScreen();

}

//--------------------------------------------------------------------------
// DrawAllSoundLights()
//--------------------------------------------------------------------------
void DrawAllSoundLights(int16_t which)
{
	int16_t i;
	uint16_t Shape;

	for (i=0;i<SndItems.amount;i++)
		if (SndMenu[i].string[0])
		{
			Shape = C_NOTSELECTEDPIC;

	 		//
	 		// DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
	 		//

			if (SndItems.cursor.on)
			   if (i == which)			// Is the cursor sitting on this pic?
			   	Shape +=2;

			switch(i)
			{
				//
			  	// SOUND EFFECTS
				//
				case 0: if (SoundMode==sdm_Off) Shape++; break;
				case 1: if (SoundMode==sdm_PC) Shape++; break;
			  	case 2: if (SoundMode==sdm_AdLib) Shape++; break;

			  	//
			  	// DIGITIZED SOUND
			  	//
			  	case 5: if (DigiMode==sds_Off) Shape++; break;
				case 6: if (DigiMode==sds_SoundSource) Shape++; break;
				case 7: if (DigiMode==sds_SoundBlaster) Shape++; break;

				//
				// MUSIC
				//
				case 10: if (MusicMode==smm_Off) Shape++; break;
				case 11: if (MusicMode==smm_AdLib) Shape++; break;
			}

			VWB_DrawPic(SndItems.x-16,SndItems.y+i*SndItems.y_spacing-1,Shape);
		}
}

char far LOADSAVE_GAME_MSG[2][25]={"^ST1^CELoading Game\r^XX",
											  "^ST1^CESaving Game\r^XX"};

extern int8_t LS_current,LS_total;

//--------------------------------------------------------------------------
// DrawLSAction() - DRAW LOAD/SAVE IN PROGRESS
//--------------------------------------------------------------------------
void DrawLSAction(int16_t which)
{
	int8_t total[]={19,19};

	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
   VW_FadeOut();
   screenfaded = true;
#endif
	DrawTopInfo(sp_loading+which);
	DrawPlayBorder();
	DisplayPrepingMsg(LOADSAVE_GAME_MSG[which]);

	if (which)
		PreloadUpdate(1,1);	// GFX: bar is full when saving...

	LS_current=1;
	LS_total=total[which];
	WindowY = 181;
}

//--------------------------------------------------------------------------
// CP_LoadGame() - LOAD SAVED GAMES
//--------------------------------------------------------------------------
int16_t CP_LoadGame(int16_t quick)
{
 int handle,which,exit=0;
 char name[13];


 _fstrcpy(name,SaveName);

 //
 // QUICKLOAD?
 //
 if (quick)
 {
	which=LSItems.curpos;

	if (SaveGamesAvail[which])
	{
		name[7]=which+'0';
		/*** BLAKE STONE: ALIENS OF GOLD V1.0+2.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
		handle=open(name,O_BINARY);
#elif (defined GAMEVER_RESTORATION_AOG_200)
		handle=open(name,O_RDONLY | O_BINARY);
#else
      MakeDestPath(name);
		handle=open(tempPath,O_RDONLY | O_BINARY);
#endif
		DrawLSAction(0);						// Testing...
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 ***/
#ifdef GAMEVER_RESTORATION_AOG_100
		loadedgame=true;
		LoadTheGame(handle);
#else
		if (!(loadedgame=LoadTheGame(handle)))
      {
      	LS_current = -1;		// clean up
      }
#endif

		close(handle);
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 ***/
#ifdef GAMEVER_RESTORATION_AOG_100
		return(1);
#else
		return(loadedgame);
#endif
	}
 }

restart:
;

 DrawLoadSaveScreen(0);

 do
 {
  which=HandleMenu(&LSItems,&LSMenu[0],TrackWhichGame);
  if (which>=0 && SaveGamesAvail[which])
  {
	ShootSnd();
	name[7]=which+'0';

	/*** BLAKE STONE: ALIENS OF GOLD V1.0+2.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
	handle=open(name,O_BINARY);
#elif (defined GAMEVER_RESTORATION_AOG_200)
	handle=open(name,O_RDONLY | O_BINARY);
#else
	MakeDestPath(name);
	handle=open(tempPath,O_RDONLY | O_BINARY);
#endif

	DrawLSAction(0);

	/*** BLAKE STONE: ALIENS OF GOLD V1.0 ***/
#ifdef GAMEVER_RESTORATION_AOG_100
	loadedgame=true;
	LoadTheGame(handle);
#else
	if (!LoadTheGame(handle))
   {
		exit = StartGame = loadedgame = 0;
      LS_current = -1;			// Clean up
      goto restart;
   }
#endif
	close(handle);

	/*** BLAKE STONE: ALIENS OF GOLD V1.0 ***/
#ifdef GAMEVER_RESTORATION_AOG_100
	StartGame = true;
#else
	loadedgame = StartGame= true;
#endif
	ShootSnd();
	//
	// CHANGE "READ THIS!" TO NORMAL COLOR
	//
	MainMenu[MM_READ_THIS].active=AT_ENABLED;
	exit=1;
	break;
  }

 } while(which>=0);

  if (which==-1)
	  MenuFadeOut();

 if (loadedgame)
	refresh_screen=false;

 return exit;
}

///////////////////////////////////
//
// HIGHLIGHT CURRENT SELECTED ENTRY
//
void TrackWhichGame(int16_t w)
{
	static int16_t lastgameon=0;

	PrintLSEntry(lastgameon,ENABLED_TEXT_COLOR);
	PrintLSEntry(w,HIGHLIGHT_TEXT_COLOR);

	lastgameon=w;
}

//--------------------------------------------------------------------------
// DRAW THE LOAD/SAVE SCREEN
//--------------------------------------------------------------------------
void DrawLoadSaveScreen(int16_t loadsave)
{
	#define DISKX	100
	#define DISKY	0

	int16_t i;

	CA_CacheScreen (BACKGROUND_SCREENPIC);
	ClearMScreen();

	fontnumber=1;

	if (!loadsave)
		DrawMenuTitle("Load Mission");
	else
		DrawMenuTitle("Save Mission");

	DrawInstructions(IT_STANDARD);

	for (i=0;i<10;i++)
		PrintLSEntry(i,ENABLED_TEXT_COLOR);

	fontnumber = 4;
	DrawMenu(&LSItems,&LSMenu[0]);

	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();
}

//--------------------------------------------------------------------------
// PRINT LOAD/SAVE GAME ENTRY W/BOX OUTLINE
//--------------------------------------------------------------------------
void PrintLSEntry(int16_t w,int16_t color)
{
	char buff[4];
	SETFONTCOLOR(color,BKGDCOLOR);
	DrawOutline(LSM_X+LSItems.indent,LSM_Y+w*LSItems.y_spacing-2,LSM_W-LSItems.indent,8,color,color);

	fontnumber=2;

	PrintX=LSM_X+LSItems.indent+2;
	PrintY=LSM_Y+w*LSItems.y_spacing;

	if (SaveGamesAvail[w])
		US_Print(SaveGameNames[w]);
	else
		US_Print("       ----- EMPTY -----");

	fontnumber=1;
}

//--------------------------------------------------------------------------
// SAVE CURRENT GAME
//--------------------------------------------------------------------------
int16_t CP_SaveGame(int16_t quick)
{

	int handle,which,exit=0;
	uint16_t nwritten;
	char name[13],input[GAME_DESCRIPTION_LEN+1];
	boolean temp_caps = allcaps;
	US_CursorStruct TermCursor = {'@',0,HIGHLIGHT_TEXT_COLOR,2};

	_fstrcpy(name,SaveName);

	allcaps = true;
	use_custom_cursor = true;
	US_CustomCursor = TermCursor;

	//
	// QUICKSAVE?
	//
	if (quick)
	{
		which=LSItems.curpos;

		if (SaveGamesAvail[which])
		{
			DrawLSAction(1);					// Testing...
			name[7]=which+'0';
			unlink(name);
			//_fmode=O_BINARY;
			/*** BLAKE STONE: ALIENS OF GOLD V2.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_PRE210
         MakeDestPath(name);
			handle=creat(tempPath,S_IREAD|S_IWRITE);
#else
			handle=creat(name,S_IREAD|S_IWRITE);
#endif

			lseek(handle,0,SEEK_SET);
			SaveTheGame(handle,&SaveGameNames[which][0]);
			close(handle);

			return 1;
		}
	}

	DrawLoadSaveScreen(1);

	do
	{
		which=HandleMenu(&LSItems,&LSMenu[0],TrackWhichGame);
		if (which>=0)
		{
			//
			// OVERWRITE EXISTING SAVEGAME?
			//
			if (SaveGamesAvail[which])
			{
				if (!Confirm(GAMESVD))
		 		{
					DrawLoadSaveScreen(1);
					continue;
		  		}
		  		else
	 			{
		  			DrawLoadSaveScreen(1);
		  			PrintLSEntry(which,HIGHLIGHT_TEXT_COLOR);
	  				VW_UpdateScreen();
			 	}
			}

			ShootSnd();

			_fstrcpy(input,&SaveGameNames[which][0]);
			name[7]=which+'0';

			fontnumber=2;
			VWB_Bar(LSM_X+LSItems.indent+1,LSM_Y+which*LSItems.y_spacing-1,LSM_W-LSItems.indent-1,7,HIGHLIGHT_BOX_COLOR);
         SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR,HIGHLIGHT_BOX_COLOR);
			VW_UpdateScreen();


			if (US_LineInput(LSM_X+LSItems.indent+2,LSM_Y+which*LSItems.y_spacing,input,input,true,GAME_DESCRIPTION_LEN,LSM_W-LSItems.indent-10))
			{
				SaveGamesAvail[which]=1;
				_fstrcpy(&SaveGameNames[which][0],input);

				unlink(name);
				//_fmode=O_BINARY;
		/*** BLAKE STONE: ALIENS OF GOLD V2.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_PRE210
            MakeDestPath(name);
				handle=creat(tempPath,S_IREAD|S_IWRITE);
#else
				handle=creat(name,S_IREAD|S_IWRITE);
#endif
				lseek(handle,0,SEEK_SET);

				DrawLSAction(1);
				SaveTheGame(handle,input);

				close(handle);

				ShootSnd();
    			exit=1;
	   	}
			else
	   	{
			 	VWB_Bar(LSM_X+LSItems.indent+1,LSM_Y+which*LSItems.y_spacing-1,LSM_W-LSItems.indent-1,7,TERM_BACK_COLOR);
	   	 	PrintLSEntry(which,HIGHLIGHT_TEXT_COLOR);
	   	 	VW_UpdateScreen();
		    	SD_PlaySound(ESCPRESSEDSND);
   		 	continue;
			}

	   	fontnumber=1;
   		break;
		}

 	} while(which>=0);

	MenuFadeOut();
 	use_custom_cursor = false;
   allcaps = temp_caps;
 	return exit;
}

//--------------------------------------------------------------------------
// EXIT OPTIONS
//--------------------------------------------------------------------------
void CP_ExitOptions(void)
{
	StartGame=1;
}

//--------------------------------------------------------------------------
// DEFINE CONTROLS
//--------------------------------------------------------------------------
void CP_Control(void)
{

	enum {MOUSEENABLE,JOYENABLE,USEPORT2,PADENABLE,CALIBRATEJOY,MOUSESENS,CUSTOMIZE};	 

 	int16_t i,which;

	CA_CacheScreen (BACKGROUND_SCREENPIC);

 DrawCtlScreen();
 MenuFadeIn();
 WaitKeyUp();

 do
 {
  which=HandleMenu(&CtlItems,&CtlMenu[0],NULL);
  switch(which)
  {
   case MOUSEENABLE:
     mouseenabled^=1;
#ifndef __AMIGA__
     _CX=_DX=CENTER;
     Mouse(4);
#endif
     DrawCtlScreen();
     CusItems.curpos=-1;
	 ShootSnd();
     break;

   case JOYENABLE:
     joystickenabled^=1;
	  if (joystickenabled)
		  CalibrateJoystick();		 
     DrawCtlScreen();
     CusItems.curpos=-1;
     ShootSnd();
     break;

	case USEPORT2:
	  joystickport^=1;
	  DrawCtlScreen();
	  ShootSnd();
	  break;

	case PADENABLE:
	  joypadenabled^=1;
	  DrawCtlScreen();
	  ShootSnd();
	  break;

	case CALIBRATEJOY:
		CalibrateJoystick();
		DrawCtlScreen();
	break;


	case MOUSESENS:
	case CUSTOMIZE:
	  DrawCtlScreen();
	  MenuFadeIn();
	  WaitKeyUp();
	  break;
  }
 } while(which>=0);

 MenuFadeOut();
}

//--------------------------------------------------------------------------
// DRAW MOUSE SENSITIVITY SCREEN
//--------------------------------------------------------------------------
void DrawMousePos(void)
{
	VWB_Bar(74,92,160,8,HIGHLIGHT_BOX_COLOR);
	DrawOutline(73,91,161,9,ENABLED_TEXT_COLOR,ENABLED_TEXT_COLOR);
	VWB_Bar(74+160/10*mouseadjustment,92,16,8,HIGHLIGHT_TEXT_COLOR);
}

void DrawMouseSens(void)
{
	ClearMScreen();
	DrawMenuTitle("MOUSE SENSITIVITY");
	DrawInstructions(IT_MOUSE_SEN);

	fontnumber = 4;

	SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR,TERM_BACK_COLOR);
	PrintX=36;
	PrintY=91;
	US_Print("SLOW");
	PrintX=242;
	US_Print("FAST");

	DrawMousePos();

	VW_UpdateScreen();
	MenuFadeIn();
}

//--------------------------------------------------------------------------
// CalibrateJoystick()
//--------------------------------------------------------------------------
void CalibrateJoystick(void)
{
	word minx,maxx,miny,maxy;

	CacheMessage(CALJOY1_TEXT);		 
	VW_UpdateScreen();

	while (IN_GetJoyButtonsDB(joystickport));
	while ((LastScan != sc_Escape) && !IN_GetJoyButtonsDB(joystickport));
	if (LastScan == sc_Escape)
		return;

	IN_GetJoyAbs(joystickport,&minx,&miny);
	while (IN_GetJoyButtonsDB(joystickport));

	CacheMessage(CALJOY2_TEXT);			  
	VW_UpdateScreen();

	while ((LastScan != sc_Escape) && !IN_GetJoyButtonsDB(joystickport));
	if (LastScan == sc_Escape)
		return;

	IN_GetJoyAbs(joystickport,&maxx,&maxy);
	if ((minx == maxx) || (miny == maxy))
		return;

	IN_SetupJoy(joystickport,minx,maxx,miny,maxy);
	while (IN_GetJoyButtonsDB(joystickport));

	IN_ClearKeysDown();
	JoystickCalibrated = true;
}


//--------------------------------------------------------------------------
// ADJUST MOUSE SENSITIVITY
//--------------------------------------------------------------------------
void MouseSensitivity(void)
{
	ControlInfo ci;
	int16_t exit=0,oldMA;

	oldMA=mouseadjustment;
	DrawMouseSens();
	do
	{
		ReadAnyControl(&ci);
		switch(ci.dir)
		{
			case dir_North:
   		case dir_West:
     			if (mouseadjustment)
				{
					mouseadjustment--;
	   			DrawMousePos();
      			VW_UpdateScreen();
      			SD_PlaySound(MOVEGUN1SND);
      			while(Keyboard[sc_LeftArrow])
#ifdef __AMIGA__
					//VL_WaitVBL(1)
					BE_ST_ShortSleep()
#endif
					;
      			WaitKeyUp();
     			}
     		break;

   		case dir_South:
   		case dir_East:
				if (mouseadjustment<9)
				{
					mouseadjustment++;
					DrawMousePos();
					VW_UpdateScreen();
					SD_PlaySound(MOVEGUN1SND);
					while(Keyboard[sc_RightArrow])
#ifdef __AMIGA__
						//VL_WaitVBL(1)
						BE_ST_ShortSleep()
#endif
						;
					WaitKeyUp();
				}
			break;
		}

		if (ci.button0 || Keyboard[sc_Space] || Keyboard[sc_Enter])
			exit=1;
		else
		if (ci.button1 || Keyboard[sc_Escape])
			exit=2;

	} while(!exit);

	if (exit==2)
	{
		mouseadjustment=oldMA;
		SD_PlaySound(ESCPRESSEDSND);
	}
	else
		SD_PlaySound(SHOOTSND);

	WaitKeyUp();
	MenuFadeOut();
}

//--------------------------------------------------------------------------
// DrawCtlScreen() - DRAW CONTROL MENU SCREEN
//--------------------------------------------------------------------------
void DrawCtlScreen(void)
{
	#define Y_CTL_PIC_OFS	(3)

	int16_t i,x,y;

	ClearMScreen();
	DrawMenuTitle("CONTROL");
	DrawInstructions(IT_STANDARD);

	WindowX=0;
	WindowW=320;
	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);

	if (JoysPresent[0])
		CtlMenu[1].active=
		CtlMenu[2].active=
		CtlMenu[3].active=
		CtlMenu[4].active=AT_ENABLED;

	CtlMenu[2].active=CtlMenu[3].active=CtlMenu[4].active=joystickenabled;

	if (MousePresent)
	{
		CtlMenu[5].active=
		CtlMenu[0].active=AT_ENABLED;
	}

	CtlMenu[5].active=mouseenabled;

	fontnumber = 4;
	DrawMenu(&CtlItems,&CtlMenu[0]);

	x=CTL_X+CtlItems.indent-24;
	y=CTL_Y+Y_CTL_PIC_OFS;
	if (mouseenabled)
		VWB_DrawPic(x,y,C_SELECTEDPIC);
	else
		VWB_DrawPic(x,y,C_NOTSELECTEDPIC);

	y=CTL_Y+9+Y_CTL_PIC_OFS;
	if (joystickenabled)
		VWB_DrawPic(x,y,C_SELECTEDPIC);
	else
		VWB_DrawPic(x,y,C_NOTSELECTEDPIC);

	y=CTL_Y+9*2+Y_CTL_PIC_OFS;
	if (joystickport)
		VWB_DrawPic(x,y,C_SELECTEDPIC);
	else
		VWB_DrawPic(x,y,C_NOTSELECTEDPIC);

	y=CTL_Y+9*3+Y_CTL_PIC_OFS;
	if (joypadenabled)
		VWB_DrawPic(x,y,C_SELECTEDPIC);
	else
		VWB_DrawPic(x,y,C_NOTSELECTEDPIC);

	//
	// PICK FIRST AVAILABLE SPOT
	//

	if (CtlItems.curpos<0 || !CtlMenu[CtlItems.curpos].active)
		for (i=0;i<6;i++)
			if (CtlMenu[i].active)
			{
				CtlItems.curpos=i;
				break;
			}

	DrawMenuGun(&CtlItems);
	VW_UpdateScreen();
}

enum {FIRE,STRAFE,RUN,OPEN};
char mbarray[4][3]={"B0","B1","B2","B3"},
		order[4]={RUN,OPEN,FIRE,STRAFE};

//--------------------------------------------------------------------------
// CustomControls() CUSTOMIZE CONTROLS
//--------------------------------------------------------------------------
void CustomControls(void)
{
	int16_t which;

 DrawCustomScreen();

 do
 {
  which=HandleMenu(&CusItems,&CusMenu[0],FixupCustom);

  switch(which)
  {
	case 0:
	  DefineMouseBtns();
	  DrawCustMouse(1);
	  break;

	case 2:
	  DefineJoyBtns();
	  DrawCustJoy(0);
	  break;

	case 4:
	  DefineKeyBtns();
	  DrawCustKeybd(0);
	  break;

	case 5:
	  DefineKeyMove();
	  DrawCustKeys(0);
  }
 } while(which>=0);



 MenuFadeOut();
}

//--------------------------------------------------------------------------
// DEFINE THE MOUSE BUTTONS
//--------------------------------------------------------------------------
void DefineMouseBtns(void)
{
	/*** BLAKE STONE VERSIONS RESTORATION ***/
#ifdef GAMEVER_RESTORATION_ANY_LATE
	CustomCtrls mouseallowed={{1,1,1,1}};
#else
	CustomCtrls mouseallowed={{0,1,1,1}};
#endif
	EnterCtrlData(2,&mouseallowed,DrawCustMouse,PrintCustMouse,MOUSE);
}

//--------------------------------------------------------------------------
// DEFINE THE JOYSTICK BUTTONS
//--------------------------------------------------------------------------
void DefineJoyBtns(void)
{
	CustomCtrls joyallowed={{1,1,1,1}};
	EnterCtrlData(5,&joyallowed,DrawCustJoy,PrintCustJoy,JOYSTICK);
}

//--------------------------------------------------------------------------
// DEFINE THE KEYBOARD BUTTONS
//--------------------------------------------------------------------------
void DefineKeyBtns(void)
{
	CustomCtrls keyallowed={{1,1,1,1}};
	EnterCtrlData(8,&keyallowed,DrawCustKeybd,PrintCustKeybd,KEYBOARDBTNS);
}

//--------------------------------------------------------------------------
// DEFINE THE KEYBOARD BUTTONS
//--------------------------------------------------------------------------
void DefineKeyMove(void)
{
	CustomCtrls keyallowed={{1,1,1,1}};
	EnterCtrlData(10,&keyallowed,DrawCustKeys,PrintCustKeys,KEYBOARDMOVE);
}

/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
//--------------------------------------------------------------------------
// TestForValidKey
//--------------------------------------------------------------------------
boolean TestForValidKey(ScanCode Scan)
{
	char far *pos;

//#pragma warn -pia

	if (!(pos = _fmemchr(buttonscan,Scan,sizeof(buttonscan))))
  		pos = _fmemchr(dirscan,Scan,sizeof(dirscan));

//#pragma warn +pia

	if (pos)
   {
   	*pos = sc_None;
		SD_PlaySound(SHOOTDOORSND);
		DrawCustomScreen();
   }

	return(!(boolean)pos);
}
#endif


//--------------------------------------------------------------------------
// EnterCtrlData() - ENTER CONTROL DATA FOR ANY TYPE OF CONTROL
//--------------------------------------------------------------------------

enum {FWRD,RIGHT,BKWD,LEFT};
int16_t moveorder[4]={LEFT,RIGHT,FWRD,BKWD};

void EnterCtrlData(int16_t index,CustomCtrls *cust,void (*DrawRtn)(int16_t),void (*PrintRtn)(int16_t),int16_t type)
{
	int16_t j,exit,tick,redraw,which,x,picked;
	ControlInfo ci;
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
  	boolean clean_display = true;
#endif

	ShootSnd();
	PrintY=CST_Y+13*index;
	IN_ClearKeysDown();
	exit=0;
	redraw=1;

	CA_CacheGrChunk(STARTFONT+fontnumber);

	//
	// FIND FIRST SPOT IN ALLOWED ARRAY
	//
	for (j=0;j<4;j++)
		if (cust->allowed[j])
		{
			 which=j;
			 break;
		}

	do
	{
	  if (redraw)
	  {
	  		x=CST_START+CST_SPC*which;
	  		DrawRtn(1);

			VWB_Bar(x-1,PrintY-1,CST_SPC,7,HIGHLIGHT_BOX_COLOR);
			SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR,HIGHLIGHT_BOX_COLOR);
			PrintRtn(which);
   		PrintX=x;
   		SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR,TERM_BACK_COLOR);
			VW_UpdateScreen();
   		WaitKeyUp();
   		redraw=0;
		}

		ReadAnyControl(&ci);

  		if (type==MOUSE || type==JOYSTICK)
	 		if (IN_KeyDown(sc_Enter)||IN_KeyDown(sc_Control)||IN_KeyDown(sc_Alt))
			{
		   	 IN_ClearKeysDown();
				 ci.button0=ci.button1=false;
		   }

	  //
	  // CHANGE BUTTON VALUE?
	  //

	  if ((ci.button0|ci.button1|ci.button2|ci.button3)||
   	  ((type==KEYBOARDBTNS||type==KEYBOARDMOVE) && LastScan==sc_Enter))
	  {
   		tick=TimeCount=picked=0;
			SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR,HIGHLIGHT_BOX_COLOR);

		   do
		   {
			   int16_t button,result=0;

			   if (type==KEYBOARDBTNS||type==KEYBOARDMOVE)
			      IN_ClearKeysDown();

			   //
			   // FLASH CURSOR
			   //

				if (TimeCount>10)
			   {
			   	switch(tick)
			     	{
				   	case 0:
							VWB_Bar(x-1,PrintY-1,CST_SPC,7,HIGHLIGHT_BOX_COLOR);
						break;

				      case 1:
							PrintX=x;
							US_Print("?");
							SD_PlaySound(HITWALLSND);
				  	}

			     	tick^=1;
			     	TimeCount=0;
			     	VW_UpdateScreen();
			   }

		    //
		    // WHICH TYPE OF INPUT DO WE PROCESS?
			 //

		    switch(type)
		    {
				 case MOUSE:
#ifdef __AMIGA__
					button = BE_ST_GetMouseButtons();
#else
			       Mouse(3);
					 button=_BX;
#endif
		   	    switch(button)
			       {
						case 1: result=1; break;
						case 2: result=2; break;
						case 4: result=3; break;
			       }

			       if (result)
			       {
						int16_t z;

						for (z=0;z<4;z++)
						  if (order[which]==buttonmouse[z])
						  {
							   buttonmouse[z]=bt_nobutton;
	   						break;
						  }

						buttonmouse[result-1]=order[which];
						picked=1;
						SD_PlaySound(SHOOTDOORSND);
						/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
						clean_display = false;
#endif
		 	       }
   	    	 break;

	     case JOYSTICK:
   	    if (ci.button0) result=1;
      	 else
	       if (ci.button1) result=2;
   	    else
	       if (ci.button2) result=3;
			 else
      	 if (ci.button3) result=4;

	       if (result)
			 {
				int16_t z;

				for (z=0;z<4;z++)
				  if (order[which]==buttonjoy[z])
				  {
					   buttonjoy[z]=bt_nobutton;
					   break;
				  }

				buttonjoy[result-1]=order[which];
				picked=1;
				SD_PlaySound(SHOOTDOORSND);
				/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
				clean_display = false;
#endif
				}
	  	  break;

   	  case KEYBOARDBTNS:
	       if (LastScan)
   	    {
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
          	if (LastScan == sc_Escape)
            	break;

	   	  	if (_fmemchr(special_keys,LastScan,sizeof(special_keys)))
					SD_PlaySound(NOWAYSND);
				else
#endif
   	      {
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
//#pragma warn -pia
      	   	if ((clean_display = TestForValidKey(LastScan)))
						ShootSnd();
//#pragma warn +pia
#endif

					buttonscan[order[which]]=LastScan;
					picked=1;
					/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
					ShootSnd();
#endif
      	   }
				IN_ClearKeysDown();
   	    }
	     break;


	     case KEYBOARDMOVE:
	       if (LastScan)
	       {
			/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
	        	if (LastScan == sc_Escape)
   	        	break;

		     	if (_fmemchr(special_keys,LastScan,sizeof(special_keys)))
					SD_PlaySound(NOWAYSND);
				else
#endif
	         {
		/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
//#pragma warn -pia
   	      	if ((clean_display = TestForValidKey(LastScan)))
						ShootSnd();
//#pragma warn +pia
#endif

					dirscan[moveorder[which]]=LastScan;
					picked=1;
					/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
					ShootSnd();
#endif
      	   }
				IN_ClearKeysDown();
			 }
	     break;
	    }


	    //
   	 // EXIT INPUT?
	    //

		 if (IN_KeyDown(sc_Escape))
   	 {
	   	  picked=1;
		     continue;
		 }

   } while(!picked);

   /*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
   if (!clean_display)
     	DrawCustomScreen();
#endif

	SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR,TERM_BACK_COLOR);
   redraw=1;
   WaitKeyUp();
	continue;
  }

  if (ci.button1 || IN_KeyDown(sc_Escape))
    exit=1;

  //
  // MOVE TO ANOTHER SPOT?
  //
  switch(ci.dir)
  {

   case dir_West:
		VWB_Bar(x-1,PrintY-1,CST_SPC,7,TERM_BACK_COLOR);
		SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR,TERM_BACK_COLOR);
  		PrintRtn(which);
     	do
     	{
      	which--;
	      if (which<0)
				which=3;
     	} while(!cust->allowed[which]);

     redraw=1;
     SD_PlaySound(MOVEGUN1SND);
	  while(ReadAnyControl(&ci),ci.dir!=dir_None);
	  IN_ClearKeysDown();
	break;



	case dir_East:
		VWB_Bar(x-1,PrintY-1,CST_SPC,7,TERM_BACK_COLOR);
		SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR,TERM_BACK_COLOR);
		PrintRtn(which);
		do
		{
			which++;
			if (which>3)
				which=0;
		} while(!cust->allowed[which]);

		redraw=1;
		SD_PlaySound(MOVEGUN1SND);

		while(ReadAnyControl(&ci),ci.dir!=dir_None);

	   IN_ClearKeysDown();
   break;

	case dir_North:
   case dir_South:
	  exit=1;
  }

 } while(!exit);

	FREEFONT(STARTFONT+fontnumber);

 SD_PlaySound(ESCPRESSEDSND);
 WaitKeyUp();
}


//--------------------------------------------------------------------------
// FixupCustom() - FIXUP GUN CURSOR OVERDRAW SHIT
//--------------------------------------------------------------------------
void FixupCustom(int16_t w)
{
	static int16_t lastwhich=-1;

	switch(w)
	{
		case 0: DrawCustMouse(1); break;
  		case 2: DrawCustJoy(1); break;
  		case 4: DrawCustKeybd(1); break;
  		case 5: DrawCustKeys(1);
	}


	if (lastwhich>=0)
	{
		if (lastwhich!=w)
			switch(lastwhich)
    		{
	  			case 0: DrawCustMouse(0); break;
     			case 2: DrawCustJoy(0); break;
     			case 4: DrawCustKeybd(0); break;
     			case 5: DrawCustKeys(0);
    		}
	}

	lastwhich=w;
}


//---------------------------------------------------------------------------
// DrawCustomScreen() - DRAW CUSTOMIZE SCREEN
//---------------------------------------------------------------------------
void DrawCustomScreen(void)
{
	int16_t i;

	ClearMScreen();
	DrawMenuTitle("CUSTOMIZE");
	DrawInstructions(IT_STANDARD);

	//
	// MOUSE
	//

	WindowX=32;
	WindowW=244;

	fontnumber = 4;

	SETFONTCOLOR(0x0C,TERM_BACK_COLOR);


	PrintY=49;
	US_CPrint("MOUSE\n");
	PrintY=79;
	US_CPrint("JOYSTICK/GRAVIS GAMEPAD\n");
	PrintY=109;
	US_CPrint("KEYBOARD\n");

	fontnumber = 2;

	SETFONTCOLOR(DISABLED_TEXT_COLOR,TERM_BACK_COLOR);

   for (i=60;i<=120;i+=30)
   {
		ShadowPrint("RUN",CST_START,i);
		ShadowPrint("OPEN",CST_START+CST_SPC*1,i);
		ShadowPrint("FIRE",CST_START+CST_SPC*2,i);
		ShadowPrint("STRAFE",CST_START+CST_SPC*3,i);
	}

	ShadowPrint("LEFT",CST_START,135);
	ShadowPrint("RIGHT",CST_START+CST_SPC*1,135);
	ShadowPrint("FWRD",CST_START+CST_SPC*2,135);
	ShadowPrint("BKWRD",CST_START+CST_SPC*3,135);


	DrawCustMouse(0);
	DrawCustJoy(0);
	DrawCustKeybd(0);
	DrawCustKeys(0);

	//
	// PICK STARTING POINT IN MENU
	//
	if (CusItems.curpos<0)
		for (i=0;i<CusItems.amount;i++)
		  if (CusMenu[i].active)
		  {
				CusItems.curpos=i;
				break;
		  }

	VW_UpdateScreen();
	MenuFadeIn();
}


//---------------------------------------------------------------------------
// PrintCustMouse
//---------------------------------------------------------------------------
void PrintCustMouse(int16_t i)
{
	int16_t j;

	for (j=0;j<4;j++)
		if (order[i]==buttonmouse[j])
		{
			PrintX=CST_START+CST_SPC*i;
			US_Print(mbarray[j]);
			break;
		}
}



//---------------------------------------------------------------------------
// DrawCustMouse
//---------------------------------------------------------------------------
void DrawCustMouse(int16_t hilight)
{
	int16_t i,color;

	color=ENABLED_TEXT_COLOR;

	if (hilight)
		color=HIGHLIGHT_TEXT_COLOR;

	SETFONTCOLOR(color,TERM_BACK_COLOR);

	if (!mouseenabled)
	{
		SETFONTCOLOR(DISABLED_TEXT_COLOR,TERM_BACK_COLOR);
		CusMenu[0].active=AT_DISABLED;
	}
	else
		CusMenu[0].active=AT_ENABLED;

	PrintY=CST_Y+7;
	for (i=0;i<4;i++)
		PrintCustMouse(i);
}



//---------------------------------------------------------------------------
// PrintCustJoy
//---------------------------------------------------------------------------
void PrintCustJoy(int16_t i)
{
	int16_t j;

	for (j=0;j<4;j++)
		if (order[i]==buttonjoy[j])
		{
			PrintX=CST_START+CST_SPC*i;
			US_Print(mbarray[j]);
			break;
		}
}


//---------------------------------------------------------------------------
// DrawCustJoy
//---------------------------------------------------------------------------
void DrawCustJoy(int16_t hilight)
{
	int16_t i,color;


	color=ENABLED_TEXT_COLOR;
	if (hilight)
		color=HIGHLIGHT_TEXT_COLOR;

	SETFONTCOLOR(color,TERM_BACK_COLOR);

	if (!joystickenabled)
	{
		SETFONTCOLOR(DISABLED_TEXT_COLOR,TERM_BACK_COLOR);
		CusMenu[2].active=AT_DISABLED;
	}
	else
		CusMenu[2].active=AT_ENABLED;

	PrintY=CST_Y+37;
	for (i=0;i<4;i++)
		PrintCustJoy(i);
}



//---------------------------------------------------------------------------
// PrintCustKeybd
//---------------------------------------------------------------------------
void PrintCustKeybd(int16_t i)
{
	PrintX=CST_START+CST_SPC*i;
	US_Print(IN_GetScanName(buttonscan[order[i]]));
}




//---------------------------------------------------------------------------
// DrawCustKeybd
//---------------------------------------------------------------------------
void DrawCustKeybd(int16_t hilight)
{
	int16_t i,color;

	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
	color=ENABLED_TEXT_COLOR;
   if (hilight)
		color=HIGHLIGHT_TEXT_COLOR;
#else
   if (hilight)
		color=HIGHLIGHT_TEXT_COLOR;
   else
 		color=ENABLED_TEXT_COLOR;
#endif

	SETFONTCOLOR(color,TERM_BACK_COLOR);

	PrintY=CST_Y+67;

	for (i=0;i<4;i++)
		PrintCustKeybd(i);
}




//---------------------------------------------------------------------------
// PrintCustKeys()
//---------------------------------------------------------------------------
void PrintCustKeys(int16_t i)
{
	PrintX=CST_START+CST_SPC*i;
	US_Print(IN_GetScanName(dirscan[moveorder[i]]));
}




//---------------------------------------------------------------------------
// DrawCustKeys()
//---------------------------------------------------------------------------
void DrawCustKeys(int16_t hilight)
{
	int16_t i,color;

	color=ENABLED_TEXT_COLOR;

	if (hilight)
		color=HIGHLIGHT_TEXT_COLOR;

	SETFONTCOLOR(color,TERM_BACK_COLOR);

	PrintY=CST_Y+82;
	for (i=0;i<4;i++)
		PrintCustKeys(i);
}


//---------------------------------------------------------------------------
// CP_ChangeView()
//---------------------------------------------------------------------------
void CP_ChangeView(void)
{
 int16_t exit=0,oldview,newview,lastview;
 ControlInfo ci;


 WindowX=WindowY=0;
 WindowW=320;
 WindowH=200;
 newview=oldview=lastview=viewwidth/16;
 DrawChangeView(oldview);

 do
 {
  CheckPause();
  ReadAnyControl(&ci);
  switch(ci.dir)
  {
	case dir_South:
	case dir_West:
	  newview--;
	  /*** BLAKE STONE: ALIENS OF GOLD V1.0 ***/
#ifdef GAMEVER_RESTORATION_AOG_100
	  if (newview<4)
		 newview=4;
#else
	  if (newview<6)
		 newview=6;
#endif
	  ShowViewSize(newview);
	  VW_UpdateScreen();
	  if (newview != lastview)
		  SD_PlaySound(HITWALLSND);
	  TicDelay(10);
	  lastview=newview;
	  break;

	case dir_North:
	case dir_East:
	  newview++;
	  /*** BLAKE STONE: ALIENS OF GOLD V1.0 ***/
#ifdef GAMEVER_RESTORATION_AOG_100
	  if (newview>19)
		 newview=19;
#else
	  if (newview>20)
		 newview=20;
#endif
	  ShowViewSize(newview);
	  VW_UpdateScreen();
	  if (newview != lastview)
			SD_PlaySound(HITWALLSND);
	  TicDelay(10);
	  lastview=newview;
	  break;
  }

  if (ci.button0 || Keyboard[sc_Enter])
	 exit=1;
  else
  if (ci.button1 || Keyboard[sc_Escape])
  {
	viewwidth=oldview*16;
	SD_PlaySound(ESCPRESSEDSND);
	MenuFadeOut();
	return;
  }

 } while(!exit);

 ControlPanelFree();

 if (oldview!=newview)
 {
	SD_PlaySound (SHOOTSND);
	Message(Computing);
	NewViewSize(newview);
 }

 ControlPanelAlloc();

 ShootSnd();
 MenuFadeOut();
}

//---------------------------------------------------------------------------
// DrawChangeView()
//---------------------------------------------------------------------------
void DrawChangeView(int16_t view)
{
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
	BevelBox(0,0,320,200-STATUSLINES,BORDER_HI_COLOR,BORDER_MED_COLOR,BORDER_LO_COLOR);
#else
	DrawTopInfo(sp_changeview);
#endif
	ShowViewSize(view);

	fontnumber = 1;
	CA_CacheGrChunk(STARTFONT+1);
	CacheBMAmsg(CHANGEVIEW_TEXT);
	FREEFONT(STARTFONT+1);

	VW_UpdateScreen();

	MenuFadeIn();
}

//---------------------------------------------------------------------------
// CP_Quit() - QUIT THIS INFERNAL GAME!
//---------------------------------------------------------------------------
void CP_Quit(void)
{
	if (Confirm(QuitToDosStr))
		ExitGame();

	DrawMainMenu();
}


////////////////////////////////////////////////////////////////////
//
// SUPPORT ROUTINES
//
////////////////////////////////////////////////////////////////////



//---------------------------------------------------------------------------
// Clear Menu screens to dark red
//---------------------------------------------------------------------------
void ClearMScreen(void)
{
	VWB_Bar(SCREEN_X,SCREEN_Y,SCREEN_W,SCREEN_H,TERM_BACK_COLOR);
}



//---------------------------------------------------------------------------
// Un/Cache a LUMP of graphics
//---------------------------------------------------------------------------
void CacheLump(int16_t lumpstart,int16_t lumpend)
{
	int16_t i;

	for (i=lumpstart;i<=lumpend;i++)
		CA_CacheGrChunk(i);
}



//---------------------------------------------------------------------------
// UnCacheLump
//---------------------------------------------------------------------------
void UnCacheLump(int16_t lumpstart,int16_t lumpend)
{
	int16_t i;

	for (i=lumpstart;i<=lumpend;i++)
		FREEFONT(i);
}


//---------------------------------------------------------------------------
// Draw a window for a menu
//---------------------------------------------------------------------------
void DrawWindow(int16_t x,int16_t y,int16_t w,int16_t h,int16_t wcolor)
{
	VWB_Bar(x,y,w,h,wcolor);
	DrawOutline(x,y,w,h,BORD2COLOR,DEACTIVE);
}



//---------------------------------------------------------------------------
// DrawOutline
//---------------------------------------------------------------------------
void DrawOutline(int16_t x,int16_t y,int16_t w,int16_t h,int16_t color1,int16_t color2)
{
	VWB_Hlin(x,x+w,y,color2);
	VWB_Vlin(y,y+h,x,color2);
	VWB_Hlin(x,x+w,y+h,color1);
	VWB_Vlin(y,y+h,x+w,color1);
}


//---------------------------------------------------------------------------
// SetupControlPanel() - Setup Control Panel stuff - graphics, etc.
//---------------------------------------------------------------------------
void SetupControlPanel(void)
{

	//
	// CACHE GRAPHICS & SOUNDS
	//
//	CA_CacheScreen (BACKGROUND_SCREENPIC);

	ControlPanelAlloc();

//	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	fontnumber=2;

	WindowH=200;

	if (!ingame)
		CA_LoadAllSounds();
	else
		MainMenu[MM_SAVE_MISSION].active=AT_ENABLED;

	ReadGameNames();

	//
	// CENTER MOUSE
	//

#ifndef __AMIGA__
	_CX=_DX=CENTER;
	Mouse(4);
#endif

}


//---------------------------------------------------------------------------
// ReadGameNames()
//---------------------------------------------------------------------------
void ReadGameNames()
{
	struct ffblk f;
	char name[13];
	int16_t which;

// SEE WHICH SAVE GAME FILES ARE AVAILABLE & READ STRING IN
//
	_fstrcpy(name,SaveName);
	/*** BLAKE STONE: ALIENS OF GOLD V2.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_PRE210
	MakeDestPath(name);
	if (!findfirst(tempPath,&f,0))
#else
	if (!findfirst(name,&f,0))
#endif
		do
		{
			which=f.ff_name[7]-'0';
			if (which<10)
			{
				int handle;
				char temp[GAME_DESCRIPTION_LEN+1];

				SaveGamesAvail[which]=1;
				/*** BLAKE STONE: ALIENS OF GOLD V1.0+2.0 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_PRE210
				MakeDestPath(f.ff_name);
				handle=open(tempPath,O_RDONLY | O_BINARY);
#elif (!defined GAMEVER_RESTORATION_AOG_100)
				handle=open(f.ff_name,O_RDONLY | O_BINARY);
#else
				handle=open(f.ff_name,O_BINARY);
#endif
				if (FindChunk(handle,"DESC"))
				{
					read(handle,temp,GAME_DESCRIPTION_LEN+1);
					_fstrcpy(&SaveGameNames[which][0],temp);
				}
				else
					_fstrcpy(&SaveGameNames[which][0],"DESCRIPTION LOST");
				close(handle);
			}
		} while(!findnext(&f));
}

//---------------------------------------------------------------------------
// CleanupControlPanel() - Clean up all the Control Panel stuff
//---------------------------------------------------------------------------
void CleanupControlPanel(void)
{
	if (!loadedgame)
		FreeMusic();
	ControlPanelFree();
	fontnumber = 4;
}

//---------------------------------------------------------------------------
// ControlPanelFree() - This FREES the control panel lump from memory
//								 and REALLOCS the ScaledDirectory
//---------------------------------------------------------------------------
void ControlPanelFree(void)
{
	UnCacheLump(CONTROLS_LUMP_START,CONTROLS_LUMP_END);
	NewViewSize(viewsize);
}

//---------------------------------------------------------------------------
// ControlPanelAlloc() - This CACHEs the control panel lump into memory
//							    and FREEs the ScaledDirectory.
//---------------------------------------------------------------------------
void ControlPanelAlloc(void)
{
	/*** BLAKE STONE: ALIENS OF GOLD V1.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_100
	FreeScaleDirectory();
#endif
	CacheLump(CONTROLS_LUMP_START,CONTROLS_LUMP_END);
}

//---------------------------------------------------------------------------
// ShadowPrint() - Shadow Prints given text @ a given x & y in default font
//
// NOTE: Font MUST already be loaded
//---------------------------------------------------------------------------
void ShadowPrint(char far *strng,int16_t x, int16_t y)
{
	int16_t old_bc,old_fc;

	old_fc = fontcolor;
	old_bc = backcolor;

	PrintX = x+1;
	PrintY = y+1;

	SETFONTCOLOR(TERM_SHADOW_COLOR,TERM_BACK_COLOR);
	US_Print(strng);

	PrintX = x;
	PrintY = y;
	SETFONTCOLOR(old_fc,old_bc);
	US_Print(strng);
}

//---------------------------------------------------------------------------
// HandleMenu() - Handle moving gun around a menu
//---------------------------------------------------------------------------
int16_t HandleMenu(CP_iteminfo far *item_i,CP_itemtype far *items,void (*routine)(int16_t w))
{
	#define box_on 	item_i->cursor.on
	int8_t key;
	static int16_t redrawitem=1,lastitem=-1;
	int16_t i,x,y,basey,exit,which,flash_tics;
	ControlInfo ci;

	which=item_i->curpos;
	x=item_i->x;
	basey=item_i->y;
	y=basey+which*item_i->y_spacing;
	box_on = 1;
	DrawGun(item_i,items,x,&y,which,basey,routine);

	SetTextColor(items+which,1);

	if (redrawitem)
	{
		ShadowPrint((items+which)->string,item_i->x+item_i->indent,item_i->y+which*item_i->y_spacing);
	}

	//
	// CALL CUSTOM ROUTINE IF IT IS NEEDED
	//

	if (routine)
		routine(which);

	VW_UpdateScreen();

	flash_tics=40;
	exit=0;
	TimeCount=0;
	IN_ClearKeysDown();

	do
	{
		CalcTics();
		flash_tics -= tics;

		CycleColors();

		//
		// CHANGE GUN SHAPE
		//

		if (flash_tics <= 0)
		{
			flash_tics = 40;

			box_on ^= 1;

			if (box_on)
				DrawGun(item_i,items,x,&y,which,basey,routine);
			else
			{
				EraseGun(item_i,items,x,y,which);
				if (routine)
					routine(which);
			}


			VW_UpdateScreen();
		}

		CheckPause();


		//
		// SEE IF ANY KEYS ARE PRESSED FOR INITIAL CHAR FINDING
		//

		key=LastASCII;
		if (key)
		{
			int16_t ok=0;

			if (key>='a')
				key-='a'-'A';

			for (i=which+1;i<item_i->amount;i++)
				if ((items+i)->active && (items+i)->string[0]==key)
				{
					EraseGun(item_i,items,x,y,which);
					which=i;
					item_i->curpos=which;			// jtr -testing
					box_on = 1;
					DrawGun(item_i,items,x,&y,which,basey,routine);
					VW_UpdateScreen();

					ok=1;
					IN_ClearKeysDown();
					break;
				}

			//
			// DIDN'T FIND A MATCH FIRST TIME THRU. CHECK AGAIN.
			//

			if (!ok)
			{
				for (i=0;i<which;i++)
					if ((items+i)->active && (items+i)->string[0]==key)
					{
						EraseGun(item_i,items,x,y,which);
						which=i;
						item_i->curpos=which;			// jtr -testing
						box_on = 1;
						DrawGun(item_i,items,x,&y,which,basey,routine);
						VW_UpdateScreen();

						IN_ClearKeysDown();
						break;
					}
			}
		}

		//
		// GET INPUT
		//

		ReadAnyControl(&ci);

		switch(ci.dir)
		{
			//------------------------
			// MOVE UP
			//
			case dir_North:
				EraseGun(item_i,items,x,y,which);

				do
				{
					if (!which)
						which=item_i->amount-1;
					else
						which--;

				} while(!(items+which)->active);

				item_i->curpos=which;			// jtr -testing

				box_on = 1;
				DrawGun(item_i,items,x,&y,which,basey,routine);

				VW_UpdateScreen();

				TicDelay(20);
			break;

			//--------------------------
			// MOVE DOWN
			//
			case dir_South:
				EraseGun(item_i,items,x,y,which);

				do
				{
					if (which==item_i->amount-1)
						which=0;
					else
						which++;
				} while(!(items+which)->active);

				item_i->curpos=which;			// jtr -testing

				box_on = 1;
				DrawGun(item_i,items,x,&y,which,basey,routine);

				VW_UpdateScreen();

				TicDelay(20);
			break;
		}

		if (ci.button0 ||	Keyboard[sc_Space] || Keyboard[sc_Enter])
			exit=1;

		if (ci.button1 || Keyboard[sc_Escape])
			exit=2;

	} while(!exit);

	IN_ClearKeysDown();

	//
	// ERASE EVERYTHING
	//

//	if (lastitem!=which)
//	{
		box_on = 0;
		redrawitem=1;
		EraseGun(item_i,items,x,y,which);
//	}
//	else
//		redrawitem=0;

	if (routine)
	{
		routine(which);
	}

	VW_UpdateScreen();

	item_i->curpos=which;

	lastitem=which;

	switch(exit)
	{
		case 1:
			//
			// CALL THE ROUTINE
			//
			if ((items+which)->routine!=NULL)
			{
			// Make sure there's room to save when CP_SaveGame() is called.
			//
				if ((int32_t)((items+which)->routine)==(int32_t)(CP_SaveGame))
					if (!CheckDiskSpace(DISK_SPACE_NEEDED,CANT_SAVE_GAME_TXT,cds_menu_print))
						return(which);

				/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
			 //
			 // ALREADY IN A GAME?
			 //
				if (ingame && ((items+which)->routine == (void (*)(int16_t))CP_NewGame))
					if (!Confirm(CURGAME))
					{
						MenuFadeOut();
						return 0;
					}
#endif

				ShootSnd();
				MenuFadeOut();
				(items+which)->routine(0);
			}
			return which;

		case 2:
			SD_PlaySound(ESCPRESSEDSND);
			return -1;
	}

	return 0; // JUST TO SHUT UP THE ERROR MESSAGES!
}

//---------------------------------------------------------------------------
// EraseGun() - ERASE GUN & DE-HIGHLIGHT STRING
//---------------------------------------------------------------------------
void EraseGun(CP_iteminfo far *item_i,CP_itemtype far *items,int16_t x,int16_t y,int16_t which)
{
	VWB_Bar(item_i->cursor.x,y+item_i->cursor.y_ofs,item_i->cursor.width,item_i->cursor.height,TERM_BACK_COLOR);
	SetTextColor(items+which,0);

   ShadowPrint((items+which)->string,item_i->x+item_i->indent,y);

//	VW_UpdateScreen();

 	x++;				// Shut up compiler
}


//---------------------------------------------------------------------------
// DrawGun() - DRAW GUN AT NEW POSITION
//---------------------------------------------------------------------------
void DrawGun(CP_iteminfo far *item_i,CP_itemtype far *items,int16_t x,int16_t *y,int16_t which,int16_t basey,void (*routine)(int16_t w))
{
	*y=basey+which*item_i->y_spacing;

   VWB_Bar(item_i->cursor.x,*y+item_i->cursor.y_ofs,item_i->cursor.width,item_i->cursor.height,HIGHLIGHT_BOX_COLOR);
	SetTextColor(items+which,1);

   ShadowPrint((items+which)->string,item_i->x+item_i->indent,item_i->y+which*item_i->y_spacing);

	//
	// CALL CUSTOM ROUTINE IF IT IS NEEDED
	//

	if (routine)
		routine(which);

//	VW_UpdateScreen();
//	SD_PlaySound(MOVEGUN2SND);

	x++;				// Shutup compiler
}

//---------------------------------------------------------------------------
// TicDelay() - DELAY FOR AN AMOUNT OF TICS OR UNTIL CONTROLS ARE INACTIVE
//---------------------------------------------------------------------------
void TicDelay(int16_t count)
{
	ControlInfo ci;

	TimeCount=0;

	do	{
		ReadAnyControl(&ci);
	} while(TimeCount<count && ci.dir!=dir_None);
}

//---------------------------------------------------------------------------
// DrawMenu() - Draw a menu
//
//       This also calculates the Y position of the current items in the
//			CP_itemtype structures.
//---------------------------------------------------------------------------
void DrawMenu(CP_iteminfo far *item_i,CP_itemtype far *items)	 
{
	int16_t i,which=item_i->curpos;

	WindowX=PrintX=item_i->x+item_i->indent;
	WindowY=PrintY=item_i->y;

	WindowW=320;
	WindowH=200;

	for (i=0;i<item_i->amount;i++)
	{
		SetTextColor(items+i,which==i);
		ShadowPrint((items+i)->string,WindowX,item_i->y+i*item_i->y_spacing);
	}
}

//---------------------------------------------------------------------------
// SetTextColor() - SET TEXT COLOR (HIGHLIGHT OR NO)
//---------------------------------------------------------------------------
void SetTextColor(CP_itemtype far *items,int16_t hlight)
{
	if (hlight)
	{
		SETFONTCOLOR(color_hlite[items->active],TERM_BACK_COLOR);
	}
	else
	{
		SETFONTCOLOR(color_norml[items->active],TERM_BACK_COLOR);
	}
}

//---------------------------------------------------------------------------
// WaitKeyUp() - WAIT FOR CTRLKEY-UP OR BUTTON-UP
//---------------------------------------------------------------------------
void WaitKeyUp(void)
{
	ControlInfo ci;

	while(ReadAnyControl(&ci),ci.button0 |
									  ci.button1 |
									  ci.button2 |
									  ci.button3|
									  Keyboard[sc_Space]|
									  Keyboard[sc_Enter]|
									  Keyboard[sc_Escape]);
}

//---------------------------------------------------------------------------
// ReadAnyControl() - READ KEYBOARD, JOYSTICK AND MOUSE FOR INPUT
//---------------------------------------------------------------------------
void ReadAnyControl(ControlInfo *ci)
{
 int16_t mouseactive=0;

 IN_ReadControl(0,ci);

#ifdef __AMIGA__
	//VL_WaitVBL(1);
	BE_ST_ShortSleep();
#endif

 //
 // UNDO some of the ControlInfo vars that were init
 // with IN_ReadControl() for the mouse...
 //
 if (ControlTypeUsed == ctrl_Mouse)
 {
 	//
   // Clear directions & buttons (if enabled or not)
   //
   ci->dir = dir_None;
   ci->button0 = ci->button1 = ci->button2 = ci->button3 = 0;
 }

 if (mouseenabled)
 {
  int16_t mousey,mousex;


  // READ MOUSE MOTION COUNTERS
  // RETURN DIRECTION
  // HOME MOUSE
  // CHECK MOUSE BUTTONS

#ifdef __AMIGA__
	BE_ST_GetMouseDelta(&mousex, &mousey);
	mousex+=CENTER;
	mousey+=CENTER;
#else
  Mouse(3);
  mousex=_CX;
  mousey=_DX;
#endif

  if (mousey<CENTER-SENSITIVE)
  {
	ci->dir=dir_North;
#ifndef __AMIGA__
	_CX=_DX=CENTER;
	Mouse(4);
#endif
	mouseactive=1;
  }
  else
  if (mousey>CENTER+SENSITIVE)
  {
	ci->dir=dir_South;
#ifndef __AMIGA__
	_CX=_DX=CENTER;
	Mouse(4);
#endif
	mouseactive=1;
  }

  if (mousex<CENTER-SENSITIVE)
  {
   ci->dir=dir_West;
#ifndef __AMIGA__
   _CX=_DX=CENTER;
   Mouse(4);
#endif
   mouseactive=1;
  }
  else
  if (mousex>CENTER+SENSITIVE)
  {
   ci->dir=dir_East;
#ifndef __AMIGA__
   _CX=_DX=CENTER;
   Mouse(4);
#endif
   mouseactive=1;
  }

  if (IN_MouseButtons())
  {
   ci->button0=IN_MouseButtons()&1;
   ci->button1=IN_MouseButtons()&2;
   ci->button2=IN_MouseButtons()&4;
   ci->button3=false;
   mouseactive=1;
  }
 }

 if (joystickenabled && !mouseactive)
 {
  int16_t jx,jy,jb;


  INL_GetJoyDelta(joystickport,&jx,&jy);
  if (jy<-SENSITIVE)
	 ci->dir=dir_North;
  else
  if (jy>SENSITIVE)
	 ci->dir=dir_South;

  if (jx<-SENSITIVE)
	 ci->dir=dir_West;
  else
  if (jx>SENSITIVE)
	 ci->dir=dir_East;


  jb=IN_JoyButtons();
  if (jb)
  {
	ci->button0=jb&1;
	ci->button1=jb&2;
	if (joypadenabled)
	{
	 ci->button2=jb&4;
	 ci->button3=jb&8;
	}
	else
	 ci->button2=ci->button3=false;
  }
 }
}

////////////////////////////////////////////////////////////////////
//
// DRAW DIALOG AND CONFIRM YES OR NO TO QUESTION
//
////////////////////////////////////////////////////////////////////
int16_t Confirm(char far *string)
{
	int16_t xit=0,i,x,y,tick=0,time,whichsnd[2]={ESCPRESSEDSND,SHOOTSND};


	Message(string);

// Next two lines needed for flashing cursor ...
//
	SETFONTCOLOR(BORDER_TEXT_COLOR,BORDER_MED_COLOR);
	CA_CacheGrChunk(STARTFONT+fontnumber);

	IN_ClearKeysDown();

	//
	// BLINK CURSOR
	//
	x=PrintX;
	y=PrintY;
	TimeCount=0;
	do
	{
		if (TimeCount>=10)
		{
			switch(tick)
			{
				case 0:
					VWB_Bar(x,y,8,13,BORDER_MED_COLOR);
					break;

				case 1:
					PrintX=x;
					PrintY=y;
					US_Print("_");
			}

			VW_UpdateScreen();
			tick^=1;
			TimeCount=0;
		}
#ifdef __AMIGA__
		BE_ST_ShortSleep();
#endif
	} while(!Keyboard[sc_Y] && !Keyboard[sc_N] && !Keyboard[sc_Escape]);


	if (Keyboard[sc_Y])
	{
		xit=1;
		ShootSnd();
	}

	while(Keyboard[sc_Y] || Keyboard[sc_N] || Keyboard[sc_Escape])
#ifdef __AMIGA__
		BE_ST_ShortSleep();
#endif
	;

	IN_ClearKeysDown();
	SD_PlaySound(whichsnd[xit]);

	FREEFONT(STARTFONT+fontnumber);

	return xit;
}

//---------------------------------------------------------------------------
// Message() - PRINT A MESSAGE IN A WINDOW
//---------------------------------------------------------------------------
void Message(char far *string)
{
	int16_t h=0,w=0,mw=0,i,x,y,time;
	fontstruct _seg *font;
	uint16_t OldPrintX,OldPrintY;

	fontnumber=1;
	CA_CacheGrChunk(STARTFONT+1);

	font=grsegs[STARTFONT+fontnumber];

	h=font->height;
	for (i=0;i<_fstrlen(string);i++)
		if (string[i]=='\n')
		{
			if (w>mw)
				mw=w;
			w=0;
			h+=font->height;
		}
		else
			w+=font->width[string[i]];

	if (w+10>mw)
	mw=w+10;

	OldPrintY = PrintY=(WindowH/2)-h/2;
	OldPrintX = PrintX=WindowX=160-mw/2;

	// bump down and to right for shadow

	PrintX++;
	PrintY++;
	WindowX++;

	BevelBox(WindowX-6,PrintY-6,mw+10,h+10,BORDER_HI_COLOR,BORDER_MED_COLOR,BORDER_LO_COLOR);

	SETFONTCOLOR(BORDER_LO_COLOR,BORDER_MED_COLOR);
	US_Print(string);

	PrintY=OldPrintY;
	WindowX=PrintX=OldPrintX;

	SETFONTCOLOR(BORDER_TEXT_COLOR,BORDER_MED_COLOR);
	US_Print(string);

	FREEFONT(STARTFONT+1);

	VW_UpdateScreen();
}



//--------------------------------------------------------------------------
// TerminateStr - Searches for an "^XX" and replaces with a 0 (NULL)
//--------------------------------------------------------------------------
void TerminateStr(char far *pos)
{
   pos = _fstrstr(pos,"^XX");

#if IN_DEVELOPMENT
   if (!pos)
   	MENU_ERROR(CACHE_MESSAGE_NO_END_MARKER);
#endif

	*pos = 0;
}

//---------------------------------------------------------------------------
// CacheMessage() - Caches and prints a message in a window.
//---------------------------------------------------------------------------
void CacheMessage(uint16_t MessageNum)
{
	char far *string;

	CA_CacheGrChunk(MessageNum);
   //string = MK_FP(grsegs[MessageNum],0);
   string = (char *)grsegs[MessageNum];


   TerminateStr(string);

   Message(string);

	FREEFONT(MessageNum);
}


/*** BLAKE STONE: ALIENS OF GOLD V1.00 RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG_100
//---------------------------------------------------------------------------
// CacheCompData() - Caches and Decompresses data from the VGAGRAPH
//
// NOTE: - User is responsible for freeing loaded data
//       - Returns the size of the data
//       - Does not call TerminateStr() for loaded TEXT data
//
// RETURNS: Lenght of loaded (decompressed) data
//
//---------------------------------------------------------------------------
uint32_t CacheCompData(uint16_t ItemNum, memptr *dest_loc)
{
   char far *compdata, far *dest_ptr;
	CompHeader_t far *CompHeader;
   uint32_t data_len;

		// Load compressed data

	CA_CacheGrChunk(ItemNum);
   //compdata = MK_FP(grsegs[ItemNum],0);
   compdata = grsegs[ItemNum];
	MM_SetLock (&grsegs[ItemNum], true);
   CompHeader = (CompHeader_t far *)compdata;

#ifdef __AMIGA__
	// TODO: what if the header was already in the memory? move to CAL_ExpandGrChunk
	CompHeader->OriginalLen = SWAP32LE(CompHeader->OriginalLen);
	CompHeader->CompType = SWAP16LE(CompHeader->CompType);
	CompHeader->CompressLen = SWAP32LE(CompHeader->CompressLen);
#endif
   data_len = CompHeader->OriginalLen;

   compdata+=sizeof(CompHeader_t);

   	// Allocate Dest Memory

	MM_GetPtr(dest_loc,data_len);
	MM_SetLock (dest_loc, true);
	//dest_ptr = MK_FP(*dest_loc,0);
	dest_ptr = *dest_loc;

   	// Decompress and terminate string

	if (!LZH_Startup())
   	Quit("out of memory");

	int count = LZH_Decompress(compdata,dest_ptr,data_len,CompHeader->CompressLen,(SRC_MEM|DEST_MEM));
   LZH_Shutdown();

   	// Free compressed data

   UNCACHEGRCHUNK(ItemNum);

   	// Return loaded size

	MM_SetLock (dest_loc, false);
   return(data_len);
}

//-------------------------------------------------------------------------
// CheckForSpecialCode() - Scans the Command Line parameters for
//									special code word and returns true if found.
//
// NOTE: - Requires that the MEMORY and CACHE manager be started up.
//       - The chunk being checked MUST be JAMPAKd - (this may change)
//
//-------------------------------------------------------------------------
boolean CheckForSpecialCode(uint16_t ItemNum)
{
   memptr code;
   boolean return_val = false;
   int8_t i;
   char far *code_ptr;

   // Allocate, Cache & Decomp into ram

   CacheCompData(ItemNum, &code);
   //code_ptr = MK_FP(code,0);
   code_ptr = code;
   TerminateStr(code_ptr);

   // Check for code

	for (i=1; i<_argc; i++)
		if (!_fmemicmp(_argv[i],code_ptr,_fstrlen(code_ptr)))
      	return_val = true;

	// free allocated memory

   MM_FreePtr(&code);

   return(return_val);
}
#endif


////////////////////////////////////////////////////////////////////
//
// THIS MAY BE FIXED A LITTLE LATER...
//
////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
// StartCPMusic()
//---------------------------------------------------------------------------
void StartCPMusic(int16_t song)
{
	musicnames	chunk;

	if (audiosegs[STARTMUSIC + lastmenumusic])	// JDC
		MM_FreePtr ((memptr *)&audiosegs[STARTMUSIC + lastmenumusic]);
	lastmenumusic = song;

	SD_MusicOff();
	chunk =	song;

	MM_BombOnError (false);
	CA_CacheAudioChunk(STARTMUSIC + chunk);
	MM_BombOnError (true);
	if (mmerror)
		mmerror = false;
	else
	{
		MM_SetLock((memptr)&audiosegs[STARTMUSIC + chunk],true);
		SD_StartMusic((MusicGroup far *)audiosegs[STARTMUSIC + chunk]);
	}
}

//---------------------------------------------------------------------------
// FreeMusic ()
//---------------------------------------------------------------------------
void FreeMusic (void)
{
	SD_MusicOff();
	if (audiosegs[STARTMUSIC + lastmenumusic])	// JDC
		MM_FreePtr ((memptr *)&audiosegs[STARTMUSIC + lastmenumusic]);
}


#ifdef CACHE_KEY_DATA

//---------------------------------------------------------------------------
//	IN_GetScanName() - 	Returns a string containing the name of the
//								specified scan code
//---------------------------------------------------------------------------
byte far* IN_GetScanName(ScanCode scan)
{
	byte		far *p;
	ScanCode	far *s;

	for (s = ExtScanCodes,p = ExtScanNames;*s;p+=7,s++)
		if (*s == scan)
			return((byte far *)p);

	return((byte far *)(ScanNames+(scan<<1)));
}

#else

//---------------------------------------------------------------------------
//	IN_GetScanName() - 	Returns a string containing the name of the
//								specified scan code
//---------------------------------------------------------------------------
byte far* IN_GetScanName(ScanCode scan)
{
	byte		far * far *p;
	ScanCode	far *s;

	for (s = ExtScanCodes,p = ExtScanNames;*s;p++,s++)
		if (*s == scan)
			return(*p);

	return(ScanNames[scan]);
}

#endif


//---------------------------------------------------------------------------
// CheckPause() - CHECK FOR PAUSE KEY (FOR MUSIC ONLY)
//---------------------------------------------------------------------------
void CheckPause(void)
{
	if (Paused)
	{
		switch(SoundStatus)
		{
			case 0:
				SD_MusicOn();
				break;

			case 1:
				SD_MusicOff();
				break;
		}

		SoundStatus^=1;
		VW_WaitVBL(3);
		IN_ClearKeysDown();
		Paused=false;
	}
}

//-------------------------------------------------------------------------
// DrawMenuGun() - DRAW GUN CURSOR AT CORRECT POSITION IN MENU
//-------------------------------------------------------------------------
void DrawMenuGun(CP_iteminfo far *iteminfo)
{
	int16_t x,y;

	x=iteminfo->cursor.x;
	y=iteminfo->y+iteminfo->curpos*iteminfo->y_spacing+iteminfo->cursor.y_ofs;

	VWB_Bar(x,y,iteminfo->cursor.width,iteminfo->cursor.height,HIGHLIGHT_BOX_COLOR);
}

//-------------------------------------------------------------------------
// ShootSnd()
//-------------------------------------------------------------------------
void ShootSnd(void)
{
	SD_PlaySound(SHOOTSND);
}

#if GAME_VERSION == SHAREWARE_VERSION


//-------------------------------------------------------------------------
// ShowPromo()
//-------------------------------------------------------------------------
void ShowPromo()
{
	#define PROMO_MUSIC HIDINGA_MUS

// Load and start music
//
	CA_CacheAudioChunk(STARTMUSIC+PROMO_MUSIC);
	SD_StartMusic((MusicGroup far *)audiosegs[STARTMUSIC+PROMO_MUSIC]);

// Show promo screen 1
//
	MenuFadeOut();
	CA_CacheScreen(PROMO1PIC);
	VW_UpdateScreen();
	MenuFadeIn();
	IN_UserInput(TickBase*20);

// Show promo screen 2
//
	MenuFadeOut();
	CA_CacheScreen(PROMO2PIC);
	VW_UpdateScreen();
	MenuFadeIn();
	IN_UserInput(TickBase*20);

// Music off and freed!
//
	StopMusic();
}

#endif

//-------------------------------------------------------------------------
// ExitGame()
//-------------------------------------------------------------------------
void ExitGame()
{
	int16_t i;

   /*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
   VW_FadeOut();
#endif
#if GAME_VERSION == SHAREWARE_VERSION
	ShowPromo();
#endif

	SD_MusicOff();
	SD_StopSound();

// SHUT-UP THE ADLIB
//
	for (i=1;i<=0xf5;i++)
		alOut(i,0);
	Quit(NULL);
}
