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
 */

#ifndef __SOUND_H
#define __SOUND_H

#ifndef WIN32

#ifdef WIN32
    #pragma warning(disable:4312)
#endif
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <map>
#include <string>
#include <fstream>

#ifdef __DEBUG
    #include <iostream>
#endif

typedef short SOUND_SID ;

/**
 * Sound engine
 *
 * \ingroup CORE
 */
class Sound {
    public:
        void  init(int, int);
        void  close();

        void  startBgm(char *, int);
        void  stopBgm();
       
        void  setVolume(int);
        void  adjustVolume(int);

        SOUND_SID loadItem(char *);
        void      startItem(SOUND_SID, int);
        
        void      clearCache();

        Sound() {isOk=-1;}

        /** if allegro is shut down or object is deleted any BGM is
           stopped and SFX run out */
        ~Sound() {stopBgm(); close();};
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

        bool isMaxVol(int);
};

#endif /* not WIN32 */
#endif
