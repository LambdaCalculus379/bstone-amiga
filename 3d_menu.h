//
// WL_MENU.H
//
#include "id_heads.h"

#define GAME_DESCRIPTION_LEN	31


//
// Menu Color Defines
//

#define HIGHLIGHT_BOX_COLOR			0x52			// Box behind text for cursor
#define HIGHLIGHT_TEXT_COLOR			0x59			// Text color for text on cursor
#define HIGHLIGHT_DISABLED_COLOR		0x56			// Text color for text on cursor for a turned off item
#define HIGHLIGHT_DEACTIAVED_COLOR  0x55


#define ENABLED_TEXT_COLOR			0x56
#define DISABLED_TEXT_COLOR		0x53
#define DEACTIAVED_TEXT_COLOR		0x52

#define INSTRUCTIONS_TEXT_COLOR	0x53

/*** BLAKE STONE: ALIENS OF GOLD V3.0 RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG_300
#define TERM_BACK_COLOR				0x04
#else
#define TERM_BACK_COLOR				0x02
#endif
#define TERM_SHADOW_COLOR			0x01

//
// Clearable Menu Terminal Area
//
#define SCREEN_X						32
#define SCREEN_Y						28
#define SCREEN_W						244
#define SCREEN_H						132


#define BORDCOLOR		(0x78)
#define BORD2COLOR	(0x74)
#define DEACTIVE		(0x72)
#define BKGDCOLOR		(0x76)
#define STRIPE			0x2c

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
#define MenuFadeOut()	VL_FadeOut(0,255,43,0,0,10)
#else
#define MenuFadeOut()	VL_FadeOut(0,255,40,44,44,10)
#endif


#define READCOLOR		0x4a
#define READHCOLOR	0x47
#define VIEWCOLOR		0x7f
#define TEXTCOLOR		WHITE
#define HIGHLIGHT		0x13

//#define MenuFadeIn()	VL_FadeIn(0,255,&vgapal,10)
#define MenuFadeIn()	VL_FadeIn(0,255,vgapal,10)

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
#define MENUSONG  	MEETINGA_MUS
#define ROSTER_MUS   LEVELA_MUS
#define TEXTSONG		RUMBAA_MUS
#else
#define MENUSONG  	LASTLAFF_MUS
#define ROSTER_MUS   HISCORE_MUS
#define TEXTSONG		TOHELL_MUS
#endif

#define QUITSUR	"Are you sure you want\n"\
						"to quit this great game? (Y/N)"

#define CURGAME "   Continuing past this\n"\
					 "  point will end the game\n"\
					 " you're currently playing.\n"\
					 "\n"\
					 " Start a NEW game? (Y/N)"

#if 0
#define CURGAME	"You are currently in\n"\
						"a game. Continuing will\n"\
						"erase the old game.\n"\
						"Start a NEW game? (Y/N)"
#endif

#define GAMESVD	"There's already a game\n"\
						"saved at this position.\n"\
						"\n"\
						"    Overwrite? (Y/N)"



#define SENSITIVE	60
#define CENTER		SENSITIVE*2

#define MENU_X	111
#define MENU_Y	50

#define SM_X	121
#define SM_Y	64
#define SM_W	54

#define CTL_X	100
#define CTL_Y	70

#define LSM_X	85
#define LSM_Y	55
#define LSM_W	144
#define LSM_H	10*13+10

#define NM_X	71
#define NM_Y	66

#define NE_X	58
#define NE_Y	54

#define CST_X		77
#define CST_Y		60

#define CST_START	77
#define CST_SPC	41

#define LSA_X	96
#define LSA_Y	80
#define LSA_W	130
#define LSA_H	42

typedef enum mm_labels {MM_NEW_MISSION,
								MM_ORDERING_INFO,
								MM_READ_THIS,
								MM_BLAKE_STONE_SAGA,
								MM_BLANK1,
								MM_GAME_OPTIONS,
								MM_VIEW_SCORES,
								MM_LOAD_MISSION,
								MM_SAVE_MISSION,
								MM_BLANK2,
								MM_BACK_TO_DEMO,
								MM_LOGOFF,
} mm_labels;

// CP_Switch() menu labels
//

typedef enum
{
	SW_LIGHTING,
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
	SW_REBA_ATTACK_INFO,
#endif
	SW_CEILING,
	SW_FLOORS,
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
	SW_HEART_BEAT,
	SW_REBA_ATTACK_INFO,
	SW_AUTO_MAP,
#endif

} sw_labels;



//
//  ActiveType flags for menu options (SEE CP_itemtype.active)
//
typedef enum
{
	AT_DISABLED = 0,
	AT_ENABLED,
	AT_READIT,
	AT_NON_SELECTABLE,				// Menu Bar/Separator - Not a selectable item.


} activetypes;

//
// TYPEDEFS
//
typedef struct
{
	uint8_t x;
	int8_t y_ofs;
	uint8_t width;
	uint8_t height;
	int8_t on;
} CP_cursortype;

typedef struct
{
	uint8_t x;
	uint8_t y;
	uint8_t amount;
	int8_t curpos;
	uint8_t indent;
	uint8_t y_spacing;

	CP_cursortype cursor;

} CP_iteminfo;


typedef struct
{
	activetypes active;
	char string[36];
	void (* routine)(int16_t temp1);
	uint8_t fontnumber;							// Font to print text in
	uint8_t height;								// Hight of text (Y_Offset from previous line)
} CP_itemtype;


typedef struct
{
	int16_t allowed[4];
} CustomCtrls;

extern CP_itemtype far MainMenu[],far NewEMenu[];
extern CP_iteminfo far MainItems;

//
// FUNCTION PROTOTYPES
//
void SetupControlPanel(void);
void CleanupControlPanel(void);
void ControlPanelFree(void);
void ControlPanelAlloc(void);

void DrawMenu(CP_iteminfo far *item_i,CP_itemtype far *items);			
int16_t HandleMenu(CP_iteminfo far *item_i,CP_itemtype far *items,void (*routine)(int16_t w));	
void ClearMScreen(void);
void DrawWindow(int16_t x,int16_t y,int16_t w,int16_t h,int16_t wcolor);
void DrawOutline(int16_t x,int16_t y,int16_t w,int16_t h,int16_t color1,int16_t color2);
void WaitKeyUp(void);
void ReadAnyControl(ControlInfo *ci);
void TicDelay(int16_t count);
void CacheLump(int16_t lumpstart,int16_t lumpend);
void UnCacheLump(int16_t lumpstart,int16_t lumpend);
void StartCPMusic(int16_t song);
int16_t  Confirm(char far *string);
void Message(char far *string);
void CheckPause(void);
void ShootSnd(void);
void CheckSecretMissions(void);

void DrawGun(CP_iteminfo far *item_i,CP_itemtype far *items,int16_t x,int16_t *y,int16_t which,int16_t basey,void (*routine)(int16_t w));	
void DrawHalfStep(int16_t x,int16_t y,int16_t y_spacing);
void EraseGun(CP_iteminfo far *item_i,CP_itemtype far *items,int16_t x,int16_t y,int16_t which);	  
void SetTextColor(CP_itemtype far *items,int16_t hlight);
void DrawMenuGun(CP_iteminfo far *iteminfo);
void DrawStripes(int16_t y);

void DefineMouseBtns(void);
void DefineJoyBtns(void);
void DefineKeyBtns(void);
void DefineKeyMove(void);
void EnterCtrlData(int16_t index,CustomCtrls *cust,void (*DrawRtn)(int16_t),void (*PrintRtn)(int16_t),int16_t type);

void DrawMainMenu(void);
void DrawSoundMenu(void);
void DrawLoadSaveScreen(int16_t loadsave);
void DrawNewEpisode(void);
void DrawNewGame(void);
void DrawChangeView(int16_t view);
void DrawMouseSens(void);
void DrawCtlScreen(void);
void DrawCustomScreen(void);
void DrawLSAction(int16_t which);
void DrawCustMouse(int16_t hilight);
void DrawCustJoy(int16_t hilight);
void DrawCustKeybd(int16_t hilight);
void DrawCustKeys(int16_t hilight);
void PrintCustMouse(int16_t i);
void PrintCustJoy(int16_t i);
void PrintCustKeybd(int16_t i);
void PrintCustKeys(int16_t i);

void PrintLSEntry(int16_t w,int16_t color);
void TrackWhichGame(int16_t w);
void DrawNewGameDiff(int16_t w);
void FixupCustom(int16_t w);

void CP_BlakeStoneSaga(void);
void CP_NewGame(void);
void CP_Sound(void);
int16_t  CP_LoadGame(int16_t quick);
int16_t  CP_SaveGame(int16_t quick);
void CP_Control(void);
void CP_ChangeView(void);
void CP_ExitOptions(void);
void CP_Quit(void);
void CP_ViewScores(void);
int16_t  CP_EndGame(void);
int16_t  CP_CheckQuick(uint16_t scancode);
void CustomControls(void);
void MouseSensitivity(void);

void DrawMenuTitle(char *title);
void CheckForEpisodes(void);
void HelpPresenter(char *fname,boolean continuekeys, uint16_t id_cache, boolean startmusic);
void ShadowPrint(char far *string, int16_t x, int16_t y);

//
// VARIABLES
//
extern int16_t SaveGamesAvail[10],StartGame,SoundStatus;
extern char far SaveGameNames[10][GAME_DESCRIPTION_LEN+1], far SaveName[13];

enum {MOUSE,JOYSTICK,KEYBOARDBTNS,KEYBOARDMOVE};	// FOR INPUT TYPES

//
// WL_INTER
//
typedef struct {
		int16_t kill,secret,treasure;
		int32_t time;
		} LRstruct;

extern LRstruct LevelRatios[];

void Write (int16_t x,int16_t y,char *string);
