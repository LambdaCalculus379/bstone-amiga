#ifndef BSZ_LOWLEVEL_H
#define BSZ_LOWLEVEL_H

#include <stdint.h>
//#include <stdbool.h>

void BE_ST_StartKeyboardService(void (*keyServicePtr)(void));
void BE_ST_StopKeyboardService(void);
int16_t BE_ST_KbHit(void);

void BE_ST_GetMouseDelta(int16_t *x, int16_t *y);
uint16_t BE_ST_GetMouseButtons(void);

void BE_ST_GetJoyAbs(uint16_t joy, uint16_t *xp, uint16_t *yp);
int16_t BE_ST_GetJoyButtons(uint16_t joy);

void BE_ST_StartAudioSDService(void (*timerServicePtr)(void));
void BE_ST_StopAudioSDService(void);
void BE_ST_SetTimer(uint16_t speed);

void BE_ST_ShortSleep(void);

int BE_ST_InitAudio(void);
void BE_ST_ShutdownAudio(void);
void BE_ST_PlayDigiSound(uint8_t *data, int32_t length);
void BE_ST_SetDigiSoundVol(uint32_t volume, uint32_t pan);
void BE_ST_StopDigiSound(void);

void BE_ST_ALOut(uint8_t reg, uint8_t val);
void BE_ST_PlaySound(int sound);
void BE_ST_StopSound(void);
void BEL_ST_LoadDigiSounds(void);
void BEL_ST_FreeDigiSounds(void);

void BE_ST_PlayMusic(int music);
void BE_ST_MusicOn(void);
void BE_ST_MusicOff(void);
void BE_ST_ShutMusic(void);

#endif
