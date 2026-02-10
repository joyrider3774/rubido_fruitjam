#include <stdint.h>
#include <string.h>
#include "sound.h"
#include "commonvars.h"
#include "i2stones.h"

#define MAX_VOL 20

int sound_on = 0, sound_vol = 3, sound_init_success = 0;
const float musModifier = (60.0f / 45.0f);
const float sfxSustain = (100.0f * 15.0f / 18.0f) / 1000.0f;

void incVolumeSound(void)
{
    sound_vol++;
    if(sound_vol > MAX_VOL)
        sound_vol = MAX_VOL;
}

void decVolumeSound(void)
{
    sound_vol--;
    if(sound_vol < 0)
        sound_vol = 0;
}

void setSoundOn(int value)
{
    sound_on = value;
}

int isSoundOn(void)
{
    return sound_on;
}


void initSound(void)
{
    
    if (!setupI2SAudio(SAMPLERATE, AUDIO_OUT_BOTH, AUDIO_BUFFER_SIZE)) {
        Serial.println("Failed to initialize I2S audio!");
        sound_init_success = false;
    }
    else
    {
        Serial.println("I2S Audio initialized successfully!");
        sound_init_success = true;
    }
}

void deInitSound(void)
{
}

void playSelectSound(void)
{
    if(!sound_on || !sound_init_success)
        return;
    playTone(1250, sound_vol * 255/MAX_VOL, sfxSustain, 0);
}


void playErrorSound(void)
{
    if(!sound_on || !sound_init_success)
        return;
    playTone(210, sound_vol * 255/MAX_VOL, sfxSustain, 0);
}


void playGameAction(void)
{
    if(!sound_on || !sound_init_success)
        return;
    playTone(600, sound_vol * 255/MAX_VOL, sfxSustain, 0);
}

void playMenuSelectSound(void)
{
    if(!sound_on || !sound_init_success)
        return;
    playTone(1250, sound_vol * 255/MAX_VOL, sfxSustain, 0);
}

void playMenuBackSound(void)
{
    if(!sound_on || !sound_init_success)
        return;
    playTone(1000, sound_vol * 255/MAX_VOL, sfxSustain, 0);
}

void playMenuAcknowlege(void)
{
    if(!sound_on || !sound_init_success)
        return;
    playTone(900, sound_vol * 255/MAX_VOL, sfxSustain, 0);
}

void playWinnerSound(void)
{
    if(!sound_on || !sound_init_success)
        return;
    playTone(523, sound_vol * 255/MAX_VOL, (100.0f / musModifier) / 1000.0f, (150.0f + 0.0f / musModifier) / 1000.0f);
    playTone(659, sound_vol * 255/MAX_VOL, (100.0f / musModifier) / 1000.0f, (150.0f + 100.0f / musModifier) / 1000.0f);
    playTone(783, sound_vol * 255/MAX_VOL, (100.0f / musModifier) / 1000.0f, (150.0f + 2.0f * 100.0f / musModifier) / 1000.0f);
    playTone(1046, sound_vol * 255/MAX_VOL, (300.0f / musModifier) / 1000.0f, (150.0f + 3.0f * 100.0f / musModifier) / 1000.0f); 
    playTone(1318, sound_vol * 255/MAX_VOL, (500.0f / musModifier) / 1000.0f, (150.0f + (3.0f * 100.0f / musModifier) + (300.0f / musModifier)) / 1000.0f); 
}

void playLoserSound(void)
{
    if(!sound_on || !sound_init_success)
        return;
    playTone(392, sound_vol * 255/MAX_VOL, (200.0f / musModifier) / 1000.0f, (150.0f + 0.0f / musModifier) / 1000.0f);
    playTone(369, sound_vol * 255/MAX_VOL, (200.0f / musModifier) / 1000.0f, (150.0f + 200.0f / musModifier) / 1000.0f);
    playTone(329, sound_vol * 255/MAX_VOL, (300.0f / musModifier) / 1000.0f, (150.0f + 2.0f * 200.0f / musModifier) / 1000.0f);
    playTone(293, sound_vol * 255/MAX_VOL, (300.0f / musModifier) / 1000.0f, (150.0f + (2.0f * 200.0f / musModifier) + (300.0f / musModifier)) / 1000.0f);
    playTone(277, sound_vol * 255/MAX_VOL, (500.0f / musModifier) / 1000.0f, (150.0f + (2.0f * 200.0f / musModifier) + (2.0f * 300.0f / musModifier)) / 1000.0f);
}

void playStartSound(void)
{
    if(!sound_on || !sound_init_success)
        return;
    playTone(784, sound_vol * 255/MAX_VOL * 1.0f, (150.0f / musModifier) / 1000.0f, (150.0f + 0.0f / musModifier) / 1000.0f);
    playTone(523, sound_vol * 255/MAX_VOL * 0.9f, (150.0f / musModifier) / 1000.0f, (150.0f + 150.0f / musModifier) / 1000.0f);
    playTone(659, sound_vol * 255/MAX_VOL * 0.8f, (200.0f / musModifier) / 1000.0f, (150.0f + 300.0f / musModifier) / 1000.0f);
    playTone(1047, sound_vol * 255/MAX_VOL * 0.7f, (300.0f / musModifier) / 1000.0f, (150.0f + 500.0f / musModifier) / 1000.0f);
}