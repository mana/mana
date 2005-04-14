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

#include <guichan.hpp>
#include <SDL.h>
#include <SDL_mixer.h>
#include <map>

#include "resources/resourcemanager.h"

typedef short SOUND_ID;

/** Sound engine
 *
 * \ingroup CORE
 */
class Sound {
    public:
        /**
         * Constructor.
         */
        Sound();

        /**
         * Destructor.
         */
        ~Sound();

        /**
         * Installs the sound engine.
         */
        void init();
        
        /**
         * Logs various info about sound device.
         */
        void info();

        /**
         * Removes all sound functionalities.
         */
        void close();

        /**
         * Starts background music.
         *
         * \param in   Full path to file
         * \param loop The number of times the song is played (-1 = infinite)
         */
        void playMusic(const char *path, int loop = -1);

        /**
         * Stops currently running background music track.
         */
        void stopMusic();
        
        /**
         * Fades in background music.
         *
         * \param in   Full path to file
         * \param loop The number of times the song is played (-1 = infinite)
         * \param ms   Duration of fade-in effect (ms)
         */
        void fadeInMusic(const char *path, int loop = -1, int ms = 2000);

        /**
         * Fades out currently running background music track.
         *
         * \param ms   Duration of fade-out effect (ms)
         */
        void fadeOutMusic(int ms);
       
        /**
         * Sets music volume.
         * 
         * \param volume Volume value
         */
        void setMusicVolume(int volume);

        /**
         * Sets sfx volume.
         *
         * \param volume Volume value
         */
        void setSfxVolume(int volume);

        /**
         * Preloads a sound-item into buffer.
         *
         * \param path Full path to file
         */
        SOUND_ID loadSfx(const char *path);

        /**
         * Plays an item in soundpool.
         *
         * \param id     Id returned to the item in the soundpool
         */
        void playSfx(SOUND_ID id);
        
        /**
         * Plays an item.
         *
         * \param path     Full path to file
         */
        void playSfx(const char *path);
        
        /**
         * Wipe all items off the cache
         */
        void clearCache();

    private:
        bool installed;

        int musicVolume, sfxVolume;

        Mix_Music *music;
        
        /** list of preloaded sound data / items */
        std::map<int, SoundEffect*> soundPool;
        SOUND_ID items;
};

#endif
