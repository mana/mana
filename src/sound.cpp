/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include "sound.h"

#ifdef __DEBUG
#include <iostream>
#endif

/**
    \brief install the sound engine
    \param voices overall reserved voices
    \param mod_voices voices dedicated for mod-playback

        NOTE:
            overall voices must not be less or equal to the
            specified amount of mod_voices!
            if mod-voices is too low some mods will not sound
            correctly since a couple of tracks are not going
            to be played along w/ the others. so missing ins-
            truments can be a result.
            32/20 sounds realistic here.
*/
void Sound::init(int voices, int mod_voices)
{
    if (isOk == 0) {
        throw("Sound engine cannot be initialized twice!\n");
    }

    bgm = NULL;
    int audio_rate = 44100;
    Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
    int audio_channels = 2;
    int audio_buffers = 4096;

    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
#ifndef __DEBUG
        throw("Unable to open audio device!\n");
#else
        throw(Mix_GetError());
#endif
    }
    
    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
    
    pan = 128;
    items = -1;
    isOk = 0;

#ifdef __DEBUG
    std::cout << "Sound::Init() Initializing Sound\n";
#endif
}

/**
    \brief set the volume value-range: 0-128
    \param music volume value
    
    NOTE:
        all values may only be between 0-128 where 0 means
        muted.
*/
void Sound::setVolume(int music)
{
    if (isOk == -1)
        return;
    
    if (isMaxVol(music) == false) {
        vol_music = music;
        Mix_VolumeMusic(vol_music);
    }
}

/**
    \brief adjusts current volume
    \param amusic volume difference
*/
void Sound::adjustVolume(int amusic)
{
    if (isOk == -1)
        return;
    
    if (!isMaxVol(vol_music + amusic)) {
        vol_music += amusic;
        Mix_VolumeMusic(vol_music);
    }
}

/**
    \brief start BGM
    \param in full path to file
    \param loop how many times should the midi be looped? (-1 = infinite)
*/
void Sound::startBgm(char * in, int loop)
{
    if (isOk == -1)
        return;
        
    if (bgm != NULL) {
        stopBgm();
    }
    
    bgm = Mix_LoadMUS(in);
    Mix_PlayMusic(bgm, loop);
#ifdef __DEBUG
    std::cout << "Sound::startBgm() Playing \"" << in << "\" " << loop << " times\n";
#endif
}

/**
    \brief stop all currently running BGM tracks

    NOTE:
        you need to stop all playback when you want to
        switch from mod to midi. playing a new track is
        usually simple as calling StartMIDI() or StartMOD() again.
        passing NULL to the playing functions only means to make
        playback stop.
*/
void Sound::stopBgm()
{
    if (isOk == -1) {
        return;
    }

#ifdef __DEBUG
    std::cout << "Sound::stopBgm()\n";
#endif
        
    if (bgm != NULL) {
        Mix_HaltMusic();
        Mix_FreeMusic(bgm);
        bgm = NULL;
    }
}

/**
    \brief preloads a sound-item into buffer
    \param fpath full path to file
    \param type type of item (SOUND_MOD, SOUND_MID, SOUND_SFX)

    NOTE:
        please make sure that the object is not loaded more
        than once since the function will not run any checks
        on its own!

        the return value should be kept as a reference to the
        object loaded. if not it is practicaly lost.
*/
SOUND_SID Sound::loadItem(char *fpath)
{
#ifdef __DEBUG
    std::cout << "Sound::loadItem() precaching \"" << fpath << "\"\n";
#endif
    Mix_Chunk *newItem;
    if ((newItem = Mix_LoadWAV(fpath))) {
        soundpool[++items] = newItem;
#ifdef __DEBUG
        std::cout << "Sound::loadItem() success SOUND_SID = " << items << std::endl;
#endif
        return items;
    }
        
    return 0;
}

/**
    \brief plays an item in soundpool
    \param id id returned to the item in the soundpool
    \param volume volume the sound should be played with (possible range: 0-128)
*/
void Sound::startItem(SOUND_SID id, int volume)
{
    if (soundpool[id]) {
#ifdef __DEBUG
        std::cout << "Sound::startItem() playing SOUND_SID = " << id << std::endl;
#endif
        Mix_VolumeChunk(soundpool[id], volume);
        Mix_PlayChannel(-1, soundpool[id], 0);
    }
}

/**
    \brief wipe all items off the cache
*/
void Sound::clearCache()
{
    for(SOUND_SID i = 0; i == items; i++) {
        Mix_FreeChunk(soundpool[i]);
        soundpool[i] = NULL;
    }
    
    soundpool.clear();
#ifdef __DEBUG
    std::cout << "Sound::clearCache() wiped all items off the cache\n";
#endif
}

/**
    \brief deinstall all sound functionality

    NOTE:
        normally you won't need to call this since this is
        done by SDL when shutting itself down. but if
        you find a reason to delete the sound-engine from
        memory (e.g. garbage-collection) feel free to use
        it. :-P
*/
void Sound::close(void)
{
    isOk = -1;
    clearCache();
    Mix_CloseAudio();
#ifdef __DEBUG
    std::cout << "Sound::close() shutting down Sound\n";
#endif
}

/**
    \brief checks if value equals min-/maximum volume and returns
    true if that's the case.
*/
bool Sound::isMaxVol(int vol)
{
    if (vol > 0 && vol < 128) return false;
    else return true;
}
