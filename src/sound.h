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

#ifndef _TMW_SOUND_H
#define _TMW_SOUND_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <map>
#include <string>
#include <fstream>

typedef short SOUND_SID;

/** Sound engine
 *
 * \ingroup CORE
 */
class Sound {
    public:
        /**
         * \brief Install the sound engine
         *
         * \param voices     Overall reserved voices
         * \param mod_voices Voices dedicated for mod-playback
         *
         * Overall voices must not be less or equal to the specified amount of
         * mod_voices!  if mod-voices is too low some mods will not sound
         * correctly since a couple of tracks are not going to be played along
         * w/ the others. so missing ins- truments can be a result.  32/20
         * sounds realistic here.
         */
        void init(int voices, int mod_voices);

        /**
         * \brief Deinstall all sound functionality
         *
         * Normally you won't need to call this since this is done by SDL when
         * shutting itself down. but if you find a reason to delete the
         * sound-engine from memory (e.g. garbage-collection) feel free to use
         * it. :-P
         */
        void close();

        /**
         * \brief Start background music
         *
         * \param in   Full path to file
         * \param loop The number of times the song is played (-1 = infinite)
         */
        void startBgm(char *in, int loop);

        /**
         * \brief Stop all currently running background music tracks
         *
         * You need to stop all playback when you want to switch from mod to
         * midi. playing a new track is usually simple as calling StartMIDI()
         * or StartMOD() again.  passing NULL to the playing functions only
         * means to make playback stop.
         */
        void stopBgm();
       
        /**
         * \brief Set the volume value-range: 0-128
         * 
         * \param music Volume value
         *
         * All values may only be between 0-128 where 0 means muted.
         */
        void setVolume(int music);

        /**
         * \brief Adjusts current volume
         * \param amusic Volume difference
         */
        void adjustVolume(int amusic);

        /**
         * \brief Preloads a sound-item into buffer
         *
         * \param fpath Full path to file
         *
         * Please make sure that the object is not loaded more than once since
         * the function will not run any checks on its own!
         *
         * The return value should be kept as a reference to the object loaded.
         * if not it is practicaly lost.
         */
        SOUND_SID loadItem(char *fpath);

        /**
         * \brief Plays an item in soundpool
         *
         * \param id     Id returned to the item in the soundpool
         * \param volume Volume the sound should be played with (possible
         *               range: 0-128)
         */
        void startItem(SOUND_SID id, int volume);
        
        /**
         * \brief Wipe all items off the cache
         */
        void clearCache();

        Sound() { isOk = -1; }

        /** if allegro is shut down or object is deleted any BGM is
           stopped and SFX run out */
        ~Sound() { stopBgm(); close(); }

    private:
        /** initial value is -1 which means error or noninitialzed.
           you can only play sounds and bgm if this is 0.
           that should be the case after calling Init() successfully */
        int isOk;

        int pan;
        int vol_music;

        Mix_Music *bgm;        
        
        /** list of preloaded sound data / items */
        std::map<int, Mix_Chunk*> soundpool;
        SOUND_SID items;

        /**
         * \brief checks if value equals min-/maximum volume and returns
         *        <code>true</code> if that's the case.
         */
        bool isMaxVol(int vol);
};

#endif
