#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

#define SFX_SUSTAIN 100

void initSound(void);
void playLoserSound(void);
void playWinnerSound(void);
void playSelectSound(void);
void playErrorSound(void);
void playGameAction(void);
void playMenuSelectSound(void);
void playMenuBackSound(void);
void playMenuAcknowlege(void);
void playStartSound(void);

void setSoundOn(int value);
int isSoundOn(void);
void deInitSound(void);
void incVolumeSound(void);
void decVolumeSound(void);
#endif