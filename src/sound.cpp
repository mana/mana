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
#include "log.h"
#include "main.h"

void Sound::init(int voices, int mod_voices)
{
    // Don't initialize sound engine twice
    if (isOk == 0) return;

    bgm = NULL;
    int audio_rate = 44100;
    Uint16 audio_format = AUDIO_S16; // 16-bit stereo
    int audio_channels = 2;
    int audio_buffers = 4096;

    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
    {
#ifndef __DEBUG
        throw("Unable to open audio device!");
#else
        throw(Mix_GetError());
#endif
    }
    
    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
    char *format_str="Unknown";
    switch(audio_format) {
        case AUDIO_U8: format_str="U8"; break;
        case AUDIO_S8: format_str="S8"; break;
        case AUDIO_U16LSB: format_str="U16LSB"; break;
        case AUDIO_S16LSB: format_str="S16LSB"; break;
        case AUDIO_U16MSB: format_str="U16MSB"; break;
        case AUDIO_S16MSB: format_str="S16MSB"; break;
    }
    
    pan = 128;
    items = -1;
    isOk = 0;

    logger.log("Sound::init() Initializing Sound");
    char driver[40];
    logger.log("Sound::init() Driver name: %s", SDL_AudioDriverName(driver, 40));
    logger.log("Sound::init() Spec: %i %s %i", audio_rate, format_str,
            audio_channels);
}

void Sound::setVolume(int music)
{
    if (isOk == -1) return;
    
    if (!isMaxVol(music)) {
        vol_music = music;
        Mix_VolumeMusic(vol_music);
    }
}

void Sound::adjustVolume(int amusic)
{
    if (isOk == -1) return;
    
    if (!isMaxVol(vol_music + amusic)) {
        vol_music += amusic;
        Mix_VolumeMusic(vol_music);
    }
}

void Sound::startBgm(char *in, int loop)
{
    if (isOk == -1) return;
        
    if (bgm != NULL) {
        stopBgm();
    }
    
    logger.log("Sound::startBgm() playing \"%s\" %d times", in, loop);

    bgm = Mix_LoadMUS(in);
    if (bgm) {
        Mix_PlayMusic(bgm, loop);
    }
    else {
        logger.log("Sound::startBgm() warning: error loading file.");
    }
}

void Sound::stopBgm()
{
    if (isOk == -1) return;

    logger.log("Sound::stopBgm()");
        
    if (bgm != NULL) {
        Mix_HaltMusic();
        Mix_FreeMusic(bgm);
        bgm = NULL;
    }
}

SOUND_SID Sound::loadItem(char *fpath)
{
    logger.log("Sound::loadItem() precaching \"%s\"", fpath);

    Mix_Chunk *newItem;
    if ((newItem = Mix_LoadWAV(fpath))) {
        soundpool[++items] = newItem;
        logger.log("Sound::loadItem() success SOUND_SID = %d", items);
        return items;
    }
        
    return 0;
}

void Sound::startItem(SOUND_SID id, int volume)
{
    if (soundpool[id]) {
        logger.log("Sound::startItem() playing SOUND_SID = %d", id);
        Mix_VolumeChunk(soundpool[id], volume);
        Mix_PlayChannel(-1, soundpool[id], 0);
    }
}

void Sound::clearCache()
{
    for (SOUND_SID i = 0; i == items; i++) {
        Mix_FreeChunk(soundpool[i]);
        soundpool[i] = NULL;
    }
    
    soundpool.clear();
    logger.log("Sound::clearCache() wiped all items off the cache");
}

void Sound::close()
{
    isOk = -1;
    clearCache();
    Mix_CloseAudio();
    logger.log("Sound::close() shutting down Sound");
}

bool Sound::isMaxVol(int vol)
{
    if (vol > 0 && vol < 128) return false;
    else return true;
}
