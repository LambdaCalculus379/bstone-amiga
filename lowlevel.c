
#include <stdint.h>
#include <stdbool.h>
#include <devices/input.h>
//#include <devices/gameport.h>
#include <intuition/intuition.h>
#include <proto/lowlevel.h>
#include <clib/macros.h>
#include <proto/dos.h>
#include <proto/ahi.h>

// Mouse
static uint16_t g_mouseButtonState = 0;
static int16_t g_mx = 0;
static int16_t g_my = 0;

// Joystick
//static ULONG joyPort = 1;
#define JOYSTICK_MIN 0
#define JOYSTICK_MAX 2
//#define JOYSTICK_MAX MaxJoyValue
#define JOYSTICK_CENTER ((JOYSTICK_MAX-JOYSTICK_MIN)/2)

// Keyboard
static uint8_t g_sdlLastKeyScanCode = 0;
static void (*g_sdlKeyboardInterruptFuncPtr)(void) = NULL;
static struct Interrupt g_inputHandler;
static struct MsgPort *g_inputPort = NULL;
static struct IOStdReq *g_inputReq = NULL;

// Timer
APTR g_timerIntHandle = NULL;
#define PC_PIT_RATE 1193182
#define TIMERDELAY_MAX 90000
//#define TIMER_SIGNAL

typedef enum BE_ST_ScanCode_T {
     BE_ST_SC_ESC = 1,
     BE_ST_SC_1,
     BE_ST_SC_2,
     BE_ST_SC_3,
     BE_ST_SC_4,
     BE_ST_SC_5,
     BE_ST_SC_6,
     BE_ST_SC_7,
     BE_ST_SC_8,
     BE_ST_SC_9,
     BE_ST_SC_0, // 0Bh
     BE_ST_SC_MINUS,
     BE_ST_SC_EQUALS,
     BE_ST_SC_BSPACE,
     BE_ST_SC_TAB,
     BE_ST_SC_Q, // 10h
     BE_ST_SC_W,
     BE_ST_SC_E,
     BE_ST_SC_R,
     BE_ST_SC_T,
     BE_ST_SC_Y,
     BE_ST_SC_U,
     BE_ST_SC_I,
     BE_ST_SC_O,
     BE_ST_SC_P,
     BE_ST_SC_LBRACKET,
     BE_ST_SC_RBRACKET,
     BE_ST_SC_ENTER,
     BE_ST_SC_LCTRL,
     BE_ST_SC_A, // 1Eh
     BE_ST_SC_S,
     BE_ST_SC_D,
     BE_ST_SC_F,
     BE_ST_SC_G,
     BE_ST_SC_H,
     BE_ST_SC_J,
     BE_ST_SC_K,
     BE_ST_SC_L,
     BE_ST_SC_SEMICOLON,
     BE_ST_SC_QUOTE,
     BE_ST_SC_GRAVE,
     BE_ST_SC_LSHIFT,
     BE_ST_SC_BACKSLASH,
     BE_ST_SC_Z, // 2Ch
     BE_ST_SC_X,
     BE_ST_SC_C,
     BE_ST_SC_V,
     BE_ST_SC_B,
     BE_ST_SC_N,
     BE_ST_SC_M,
     BE_ST_SC_COMMA,
     BE_ST_SC_PERIOD,
     BE_ST_SC_SLASH,
     BE_ST_SC_RSHIFT,
     BE_ST_SC_PRINTSCREEN, // 37h but kind of special
     BE_ST_SC_LALT,
     BE_ST_SC_SPACE,
     BE_ST_SC_CAPSLOCK,
     BE_ST_SC_F1, // 3Bh
     BE_ST_SC_F2,
     BE_ST_SC_F3,
     BE_ST_SC_F4,
     BE_ST_SC_F5,
     BE_ST_SC_F6,
     BE_ST_SC_F7,
     BE_ST_SC_F8,
     BE_ST_SC_F9,
     BE_ST_SC_F10,
     BE_ST_SC_NUMLOCK, // 45h
     BE_ST_SC_SCROLLLOCK,
     BE_ST_SC_KP_7,
     BE_ST_SC_KP_8,
     BE_ST_SC_KP_9,
     BE_ST_SC_KP_MINUS,
     BE_ST_SC_KP_4,
     BE_ST_SC_KP_5,
     BE_ST_SC_KP_6,
     BE_ST_SC_KP_PLUS,
     BE_ST_SC_KP_1,
     BE_ST_SC_KP_2,
     BE_ST_SC_KP_3,
     BE_ST_SC_KP_0,
     BE_ST_SC_KP_PERIOD, // 53h
     // A couple of "special" keys (scancode E0h sent first)
     BE_ST_SC_KP_DIVIDE = 0x35,
     BE_ST_SC_KP_ENTER = 0x1C,
     // Back to a few "non-special" keys
     BE_ST_SC_F11 = 0x57,
     BE_ST_SC_F12 = 0x58,
     // And again special keys
     BE_ST_SC_INSERT = 0x52,
     BE_ST_SC_DELETE = 0x53,
     BE_ST_SC_HOME = 0x47,
     BE_ST_SC_END = 0x4F,
     BE_ST_SC_PAGEUP = 0x49,
     BE_ST_SC_PAGEDOWN = 0x51,
     BE_ST_SC_LEFT = 0x4B,
     BE_ST_SC_RIGHT = 0x4D,
     BE_ST_SC_UP = 0x48,
     BE_ST_SC_DOWN = 0x50,
     BE_ST_SC_RALT = 0x38,
     BE_ST_SC_RCTRL = 0x1D,
     // Two extra keys
     BE_ST_SC_LESSTHAN = 0x56,
     BE_ST_SC_KP_MULTIPLY = 0x37,
     // This one is different from all the rest (6 scancodes sent on press ONLY)
     BE_ST_SC_PAUSE = 0xE1,

     // SPECIAL - Used to mark maximum, may have to update if 0xFF is actually used
     BE_ST_SC_MAX = 0xFF,
} BE_ST_ScanCode_T; 

typedef struct {
	bool isSpecial; // Scancode of 0xE0 sent?
	uint8_t dosScanCode;
} emulatedDOSKeyEvent;

#define emptyDOSKeyEvent {false, 0}

const emulatedDOSKeyEvent sdlKeyMappings[128] = {
	{false,  BE_ST_SC_GRAVE},
	{false,  BE_ST_SC_1},
	{false,  BE_ST_SC_2},
	{false,  BE_ST_SC_3},
	{false,  BE_ST_SC_4},
	{false,  BE_ST_SC_5},
	{false,  BE_ST_SC_6},
	{false,  BE_ST_SC_7},
	{false,  BE_ST_SC_8},
	{false,  BE_ST_SC_9},
	{false,  BE_ST_SC_0},
	{false,  BE_ST_SC_MINUS},
	{false,  BE_ST_SC_EQUALS},
	{false,  BE_ST_SC_BACKSLASH},
	emptyDOSKeyEvent,
	{false,  BE_ST_SC_KP_0}, //{true,  BE_ST_SC_INSERT},
	{false,  BE_ST_SC_Q},
	{false,  BE_ST_SC_W},
	{false,  BE_ST_SC_E},
	{false,  BE_ST_SC_R},
	{false,  BE_ST_SC_T},
	{false,  BE_ST_SC_Y},
	{false,  BE_ST_SC_U},
	{false,  BE_ST_SC_I},
	{false,  BE_ST_SC_O},
	{false,  BE_ST_SC_P},
	{false,  BE_ST_SC_LBRACKET},
	{false,  BE_ST_SC_RBRACKET},
	emptyDOSKeyEvent,
	{false,  BE_ST_SC_KP_1}, //{true,  BE_ST_SC_END},
	{false,  BE_ST_SC_KP_2},
	{false,  BE_ST_SC_KP_3}, //{true,  BE_ST_SC_PAGEDOWN},
	{false,  BE_ST_SC_A},
	{false,  BE_ST_SC_S},
	{false,  BE_ST_SC_D},
	{false,  BE_ST_SC_F},
	{false,  BE_ST_SC_G},
	{false,  BE_ST_SC_H},
	{false,  BE_ST_SC_J},
	{false,  BE_ST_SC_K},
	{false,  BE_ST_SC_L},
	{false,  BE_ST_SC_SEMICOLON},
	{false,  BE_ST_SC_QUOTE},
	emptyDOSKeyEvent, // international key next to Enter
	emptyDOSKeyEvent,
	{false,  BE_ST_SC_KP_4},
	{false,  BE_ST_SC_KP_5},
	{false,  BE_ST_SC_KP_6},
	{false,  BE_ST_SC_LESSTHAN}, // international key between the left Shift and Z
	{false,  BE_ST_SC_Z},
	{false,  BE_ST_SC_X},
	{false,  BE_ST_SC_C},
	{false,  BE_ST_SC_V},
	{false,  BE_ST_SC_B},
	{false,  BE_ST_SC_N},
	{false,  BE_ST_SC_M},
	{false,  BE_ST_SC_COMMA},
	{false,  BE_ST_SC_PERIOD},
	{false,  BE_ST_SC_SLASH},
	emptyDOSKeyEvent,
	{false,  BE_ST_SC_KP_PERIOD},
	{false,  BE_ST_SC_KP_7}, //{true,  BE_ST_SC_HOME},
	{false,  BE_ST_SC_KP_8},
	{false,  BE_ST_SC_KP_9}, //{true,  BE_ST_SC_PAGEUP},
	{false,  BE_ST_SC_SPACE},
	{false,  BE_ST_SC_BSPACE},
	{false,  BE_ST_SC_TAB},
	{true,  BE_ST_SC_KP_ENTER},
	{false,  BE_ST_SC_ENTER},
	{false,  BE_ST_SC_ESC},
	{true,  BE_ST_SC_DELETE},
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	{false,  BE_ST_SC_KP_MINUS},
	emptyDOSKeyEvent,
	{true,  BE_ST_SC_UP},
	{true,  BE_ST_SC_DOWN},
	{true,  BE_ST_SC_RIGHT},
	{true,  BE_ST_SC_LEFT},
	{false,  BE_ST_SC_F1},
	{false,  BE_ST_SC_F2},
	{false,  BE_ST_SC_F3},
	{false,  BE_ST_SC_F4},
	{false,  BE_ST_SC_F5},
	{false,  BE_ST_SC_F6},
	{false,  BE_ST_SC_F7},
	{false,  BE_ST_SC_F8},
	{false,  BE_ST_SC_F9},
	{false,  BE_ST_SC_F10},
	{false,  BE_ST_SC_NUMLOCK},
	{false,  BE_ST_SC_SCROLLLOCK},
	{true,  BE_ST_SC_KP_DIVIDE},
	{false,  BE_ST_SC_KP_MULTIPLY}, //{false,  BE_ST_SC_PRINTSCREEN},
	{false,  BE_ST_SC_KP_PLUS},
	{false,  BE_ST_SC_PAUSE}, // Help key
	{false,  BE_ST_SC_LSHIFT},
	{false,  BE_ST_SC_RSHIFT},
	{false,  BE_ST_SC_CAPSLOCK},
	{false,  BE_ST_SC_LCTRL},
	{false,  BE_ST_SC_LALT},
	{true,  BE_ST_SC_RALT},
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	emptyDOSKeyEvent,
	// no Amiga equivalents
	//{false,  BE_ST_SC_F11},
	//{false,  BE_ST_SC_F12},
	//{true,  BE_ST_SC_RCTRL},
};

static struct InputEvent * __saveds BEL_ST_InputHandler(register struct InputEvent *moo __asm("a0"), register APTR id __asm("a1"))
{
	struct InputEvent *coin;
	int scanCode, /*isPressed,*/ isRelease;

	for (coin = moo; coin; coin = coin->ie_NextEvent)
	{
		//isPressed = !(coin->ie_Code & IECODE_UP_PREFIX);
		isRelease = coin->ie_Code & IECODE_UP_PREFIX;
		scanCode = coin->ie_Code & ~IECODE_UP_PREFIX;

		// keyboard
		if (coin->ie_Class == IECLASS_RAWKEY)
		{
			//BEL_ST_HandleEmuKeyboardEvent(isPressed, sdlKeyMappings[scanCode]);
			UWORD rawKey = coin->ie_Code;
			const emulatedDOSKeyEvent *keyEvent = &sdlKeyMappings[scanCode];
			if (keyEvent->isSpecial)
			{
				g_sdlLastKeyScanCode = 0xe0;
				g_sdlKeyboardInterruptFuncPtr();
			}
			g_sdlLastKeyScanCode = keyEvent->dosScanCode | isRelease;
			g_sdlKeyboardInterruptFuncPtr();
		}

		// mouse
		if ((coin->ie_Class == IECLASS_RAWMOUSE))
		{
			if (coin->ie_Code != IECODE_NOBUTTON)
			{
				switch (scanCode)
				{
					case IECODE_LBUTTON:
						if (!isRelease)
							g_mouseButtonState |= 1;
						else
							g_mouseButtonState &= ~1;
						break;
					case IECODE_RBUTTON:
						if (!isRelease)
							g_mouseButtonState |= 2;
						else
							g_mouseButtonState &= ~2;
						break;
					case IECODE_MBUTTON:
						if (!isRelease)
							g_mouseButtonState |= 4;
						else
							g_mouseButtonState &= ~4;
						break;
				}
			}
			g_mx += coin->ie_position.ie_xy.ie_x;
			g_my += coin->ie_position.ie_xy.ie_y;
		}
	}

	return moo;
}

void BE_ST_StartKeyboardService(void (*keyServicePtr)(void))
{
	g_sdlKeyboardInterruptFuncPtr = keyServicePtr;
	if ((g_inputPort = CreateMsgPort()))
	{
		if ((g_inputReq = (struct IOStdReq *)CreateIORequest(g_inputPort, sizeof(*g_inputReq))))
		{
			if (!OpenDevice((STRPTR)"input.device", 0, (struct IORequest *)g_inputReq, 0))
			{
				g_inputHandler.is_Node.ln_Type = NT_INTERRUPT;
				g_inputHandler.is_Node.ln_Pri = 100;
				g_inputHandler.is_Node.ln_Name = (STRPTR)"Catacomb 3-D input handler";
				g_inputHandler.is_Code = (void (*)())&BEL_ST_InputHandler;
				g_inputReq->io_Data = (void *)&g_inputHandler;
				g_inputReq->io_Command = IND_ADDHANDLER;
				if (!DoIO((struct IORequest *)g_inputReq))
				{
					//return true;
				}
			}
		}
	}
	//return false;
}

void BE_ST_StopKeyboardService(void)
{
	if (g_inputReq)
	{
		g_inputReq->io_Data = (void *)&g_inputHandler;
		g_inputReq->io_Command = IND_REMHANDLER;
		DoIO((struct IORequest *)g_inputReq);
		CloseDevice((struct IORequest *)g_inputReq);
		DeleteIORequest((struct IORequest *)g_inputReq);
		g_inputReq = NULL;
	}

	if (g_inputPort)
	{
		DeleteMsgPort(g_inputPort);
		g_inputPort = NULL;
	}
}

int16_t BE_ST_KbHit(void)
{
	return g_sdlLastKeyScanCode;
}

void BE_ST_GetMouseDelta(int16_t *x, int16_t *y)
{
	*x = g_mx;
	*y = g_my;
	g_mx = g_my = 0;
}

uint16_t BE_ST_GetMouseButtons(void)
{
	return g_mouseButtonState;
}

void BE_ST_GetJoyAbs(uint16_t joy, uint16_t *xp, uint16_t *yp)
{
	int jx = 0;
	int jy = 0;
	ULONG portState;

	portState = ReadJoyPort(/*joy*/1);

	if ((portState & JP_TYPE_MASK) != JP_TYPE_NOTAVAIL)
	{
		jx = jy = JOYSTICK_CENTER; // center for the joystick detection
	}

	if (((portState & JP_TYPE_MASK) == JP_TYPE_GAMECTLR) || ((portState & JP_TYPE_MASK) == JP_TYPE_JOYSTK))
	{
		if (portState & JPF_JOY_UP)
			jy = JOYSTICK_MIN;
		else if (portState & JPF_JOY_DOWN)
			jy = JOYSTICK_MAX;

		if (portState & JPF_JOY_LEFT)
			jx = JOYSTICK_MIN;
		else if (portState & JPF_JOY_RIGHT)
			jx = JOYSTICK_MAX;
		//BE_ST_DebugText(0, 8, "joy %d portstate %08x jx %d jy %d", joy, portState, jx, jy);
	}

	*xp = jx;
	*yp = jy;
}

int16_t BE_ST_GetJoyButtons(uint16_t joy)
{
	int16_t buttons = 0;
	ULONG portState;

	portState = ReadJoyPort(/*joy*/1);

	if (((portState & JP_TYPE_MASK) == JP_TYPE_GAMECTLR) /*|| ((portState & JP_TYPE_MASK) == JP_TYPE_JOYSTK)*/)
	{
		// CD32 gamepad
		if (portState & JPF_BUTTON_BLUE) buttons |= 1 << 0;
		if (portState & JPF_BUTTON_RED) buttons |= 1 << 1;
		if (portState & JPF_BUTTON_YELLOW) buttons |= 1 << 2;
		if (portState & JPF_BUTTON_GREEN) buttons |= 1 << 3;
		/*if (portState & JPF_BUTTON_FORWARD) buttons |= 1 << 4;
		if (portState & JPF_BUTTON_REVERSE) buttons |= 1 << 5;
		if (portState & JPF_BUTTON_PLAY) buttons |= 1 << 6;*/
		//BE_ST_DebugText(0, 16, "joy %d portstate %08x buttons %u", joy, portState, buttons);
	}
	else if ((portState & JP_TYPE_MASK) == JP_TYPE_JOYSTK)
	{
		// regular 3 button joystick/gamepad
		if (portState & JPF_BUTTON_BLUE) buttons |= 1 << 0;
		if (portState & JPF_BUTTON_RED) buttons |= 1 << 1;
		if (portState & JPF_BUTTON_PLAY) buttons |= 1 << 2;
	}

	return buttons;
}

static void BEL_ST_TimerInterrupt(register APTR intData __asm("a1"))
{
	static void (*timerServiceFunc)(void);
	timerServiceFunc = intData;
	timerServiceFunc();
}

void BE_ST_StartAudioSDService(void (*timerServicePtr)(void))
{
	g_timerIntHandle = AddTimerInt((APTR)BEL_ST_TimerInterrupt, timerServicePtr);
}

void BE_ST_StopAudioSDService(void)
{
	RemTimerInt(g_timerIntHandle);
	g_timerIntHandle = NULL;
}

void BE_ST_SetTimer(uint16_t speed)
{
	ULONG timerDelay;
	//printf("%s speed %u\n",__FUNCTION__, speed);
	if (speed > 0)
	{
		timerDelay = (1000 * 1000) / (PC_PIT_RATE / speed);//printf("%s timerDelay %lu\n",__FUNCTION__, timerDelay);
		timerDelay = MIN(timerDelay, TIMERDELAY_MAX);//printf("%s timerDelay %lu\n",__FUNCTION__, timerDelay);
		StartTimerInt(g_timerIntHandle, timerDelay, TRUE);
	}
	else
	{
		StopTimerInt(g_timerIntHandle);
	}
}

// technically not in lowlevel.library, but whatever
void BE_ST_ShortSleep(void)
{
#ifdef TIMER_SIGNAL
	SetSignal(0, SIGBREAKF_CTRL_F);
	g_dstTimeCount = srctimecount+timetowait;
	Wait(SIGBREAKF_CTRL_F);
#else
	Delay(1);
#endif
}

// Sound
static struct Library      *AHIBase;
static struct MsgPort      *AHImp     = NULL;
static struct AHIRequest   *AHIio     = NULL;
static BYTE                 AHIDevice = -1;
static struct AHIAudioCtrl *actrl     = NULL;
static uint32_t digivol;
static uint32_t digipan;
enum
{
	CHANNEL_DIGI,
	CHANNEL_ADLIB,
	CHANNEL_MUSIC,
	CHANNEL_MAX
};
enum
{
	SOUND_DIGI,
	SOUND_ADLIB,
	SOUND_MUSIC,
	SOUND_MAX
}; // TODO this is a bit nasty
#define OPL_SAMPLE_RATE 11025
#define DIGI_SAMPLE_RATE 7000
#define LASTSOUND 100 // for both games
static int8_t *g_digiSounds[LASTSOUND+1];
static int g_digiSoundSamples[LASTSOUND+1];
static int g_lastSound = -1;

int BE_ST_InitAudio(void)
{
	if ((AHImp = CreateMsgPort()))
	{
		if ((AHIio = (struct AHIRequest *)CreateIORequest(AHImp, sizeof(struct AHIRequest))))
		{
			AHIio->ahir_Version = 4;
			if (!(AHIDevice = OpenDevice(AHINAME, AHI_NO_UNIT, (struct IORequest *)AHIio, 0)))
			{
				//ULONG id = AHI_INVALID_ID;

				AHIBase = (struct Library *)AHIio->ahir_Std.io_Device;

				/*
				id = AHI_BestAudioID(
					AHIDB_Volume, TRUE,
					AHIDB_Panning, TRUE,
					AHIDB_Bits, 8,
					AHIDB_MinMixFreq, 7000,
					TAG_END);
					*/

				actrl = AHI_AllocAudio(
					//AHIA_AudioID, id,
					AHIA_Channels, CHANNEL_MAX,
					AHIA_Sounds, SOUND_MAX,
					TAG_END);

				if (actrl)
				{
					ULONG error = AHI_ControlAudio(actrl,
						AHIC_Play, TRUE,
						TAG_END);

					char modename[64];
					AHI_GetAudioAttrs(AHI_INVALID_ID, actrl,
								AHIDB_BufferLen, sizeof(modename),
								AHIDB_Name, (ULONG) modename,
								/*
								AHIDB_MaxChannels, (IPTR) &channels,
								AHIDB_Bits, (IPTR) &bits,
								*/
								TAG_END);

					printf("%s mode '%s'\n", __FUNCTION__, modename);

					return error == AHIE_OK;
				}
			}
		}
	}

	return 0;
}

void BE_ST_ShutdownAudio(void)
{
	if (AHIBase && actrl)
	{
		AHI_ControlAudio(actrl,
			AHIC_Play, FALSE,
			TAG_END);

		AHI_FreeAudio(actrl);
		actrl = NULL;
	}

	if (!AHIDevice)
	{
		AHIBase = NULL;
		CloseDevice((struct IORequest *)AHIio);
		AHIDevice = -1;
	}

	if (AHIio)
	{
		DeleteIORequest((struct IORequest *)AHIio);
		AHIio = NULL;
	}

	if (AHImp)
	{
		DeleteMsgPort(AHImp);
		AHImp = NULL;
	}
}

static bool BEL_ST_LoadSample(int8_t *data, int32_t length, int16_t sound)
{
	struct AHISampleInfo sample;
	ULONG error;

	//printf("%s(%p,%d)\n", __FUNCTION__, data, length);

	if (!actrl || !data)
		return false;

	sample.ahisi_Address = data;
	sample.ahisi_Type = AHIST_M8S;
	sample.ahisi_Length = length;
	AHI_UnloadSound(sound, actrl);
	error = AHI_LoadSound(sound, AHIST_SAMPLE, &sample, actrl);

	return error == AHIE_OK;
}

static void BEL_ST_PlaySample(int16_t channel, int16_t sound, int32_t length, int32_t rate, uint32_t vol, uint32_t pan, bool loop)
{
	//printf("%s(%p,%d)\n", __FUNCTION__, data, length);

	if (!actrl)
		return;

	AHI_SetFreq(channel, rate, actrl, AHISF_IMM);
	AHI_SetVol(channel, vol, pan, actrl, AHISF_IMM);
	AHI_SetSound(channel, sound, 0, length, actrl, AHISF_IMM);
	if (!loop)
		AHI_SetSound(channel, AHI_NOSOUND, 0, 0, actrl, AHISF_NONE);
}

static void BEL_ST_StopSample(int16_t channel)
{
	if (!actrl)
		return;

	AHI_SetSound(channel, AHI_NOSOUND, 0, 0, actrl, AHISF_IMM);
}

// Digi sounds
void BE_ST_PlayDigiSound(uint8_t *data, int32_t length)
{
	//printf("%s(%p,%d)\n", __FUNCTION__, data, length);

	BEL_ST_LoadSample(data, length, SOUND_DIGI);
	BEL_ST_PlaySample(CHANNEL_DIGI, SOUND_DIGI, length, DIGI_SAMPLE_RATE, digivol, digipan, false);
}

void BE_ST_SetDigiSoundVol(uint32_t volume, uint32_t pan)
{
	//printf("%s(%08x,%08x)\n", __FUNCTION__, volume, pan);

	if (!actrl)
		return;

	digivol = volume;
	digipan = pan;
	AHI_SetVol(CHANNEL_DIGI, volume, pan, actrl, AHISF_IMM);
}

void BE_ST_StopDigiSound(void)
{
	//printf("%s()\n", __FUNCTION__);
	BEL_ST_StopSample(CHANNEL_DIGI);
}

// AdLib sounds
#include <sys/stat.h>
#include <fcntl.h>
//#include <id_mm.h>

typedef struct
{
	uint32_t magic; /* magic number */
	uint32_t hdr_size; /* size of this header */ 
	uint32_t data_size; /* length of data (optional) */ 
	uint32_t encoding; /* data encoding format */
	uint32_t sample_rate; /* samples per second */
	uint32_t channels; /* number of interleaved channels */
} Audio_filehdr;

// dummy function
void BE_ST_ALOut(uint8_t reg, uint8_t val)
{
}

static void BEL_ST_LoadSound(char *filename, int i)
{
	int fp;

	if ((fp = open(filename, O_RDONLY, S_IREAD)) != -1)
	{
		Audio_filehdr header;
		read(fp, &header, sizeof(header));
		//int32_t nbyte = filelength(fp);
		int32_t nbyte = header.data_size;
		if (nbyte == -1)
		{
			nbyte = filelength(fp);
			nbyte -= header.hdr_size;
		}
		if ((g_digiSounds[i] = malloc(nbyte)))
		{
			read(fp, g_digiSounds[i], nbyte);
			g_digiSoundSamples[i] = nbyte;
		}
		close(fp);
	}
}

/*static*/ void BEL_ST_LoadDigiSounds(void)
{
	char filename[256];

	for (int i = 0; i < LASTSOUND; i++)
	{
		snprintf(filename, sizeof(filename), "adlib/sound%02d.au", i);
		//printf("%s loading %s...\n", __FUNCTION__, filename);
		BEL_ST_LoadSound(filename, i);
	}
}

static void BEL_ST_FreeSound(int i)
{
	if (!g_digiSounds[i])
		return;

	free(g_digiSounds[i]);
	g_digiSounds[i] = NULL;
	g_digiSoundSamples[i] = 0;
}

/*static*/ void BEL_ST_FreeDigiSounds(void)
{
	for (int i = 0; i < LASTSOUND; i++)
	{
		BEL_ST_FreeSound(i);
	}
}

void BE_ST_PlaySound(int sound)
{
	//printf("%s(%d)\n", __FUNCTION__, sound);

	BEL_ST_LoadSample(g_digiSounds[sound], g_digiSoundSamples[sound], SOUND_ADLIB);
	BEL_ST_PlaySample(CHANNEL_ADLIB, SOUND_ADLIB, g_digiSoundSamples[sound], OPL_SAMPLE_RATE, /*0xFFF0*/0xC000, 0x8000, false);
}

void BE_ST_StopSound(void)
{
	//printf("%s()\n", __FUNCTION__);
	BEL_ST_StopSample(CHANNEL_ADLIB);
}

// AdLib music
bool musicOn = false;
bool musicLoop = true;
bool musicOk = false;

void BE_ST_ShutMusic(void)
{
	BEL_ST_FreeSound(LASTSOUND);
}

void BE_ST_PlayMusic(int music)
{
	char filename[256];
	BE_ST_ShutMusic();
	snprintf(filename, sizeof(filename), "adlib/music%02d.au", music);
	//printf("%s loading %s...\n", __FUNCTION__, filename);
	// don't loop the Apogee fanfare because the title can display longer due to timing differences
	musicLoop = music != 2;
	BEL_ST_LoadSound(filename, LASTSOUND);
	musicOk = BEL_ST_LoadSample(g_digiSounds[LASTSOUND], g_digiSoundSamples[LASTSOUND], SOUND_MUSIC);
}

void BE_ST_MusicOn(void)
{
	if (musicOn && !musicOk)
		return;

	musicOn = true;
	BEL_ST_PlaySample(CHANNEL_MUSIC, SOUND_MUSIC, g_digiSoundSamples[LASTSOUND], OPL_SAMPLE_RATE, 0x10000, 0x8000, musicLoop);
}

void BE_ST_MusicOff(void)
{
	if (!musicOn)
		return;

	musicOn = false;
	BEL_ST_StopSample(CHANNEL_MUSIC);
}
