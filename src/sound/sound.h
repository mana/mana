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


#ifdef WIN32
    #pragma warning(disable:4312)
#endif
#include <allegro.h>
#include <jgmod.h>
#include <list>
#include <string>
#include <fstream>

/** mod file */
#define TMWSOUND_MOD 1
/** midi file */
#define TMWSOUND_MID 2
/** sample file */
#define TMWSOUND_SFX 3

typedef unsigned short TMWSOUND_SID ;

/**
 * Sound engine
 *
 * \ingroup CORE
 */
class TmwSound {
    public:
        void  Init(int, int);
        void  Close();

        void  StartMIDI(char *, int);
        void  StartMOD(char *, int);
        void  StopBGM();

        void  StartWAV(char *, int);
        void  SetVol(int, int, int);
        void  SetAdjVol(int, int, int);

        TMWSOUND_SID LoadItem(char *, char);
        void UnloadItem(TMWSOUND_SID);
        void PlayItem(TMWSOUND_SID, int);

        TmwSound() {isOk=-1;}

        /** if allegro is shut down or object is deleted any BGM is
           stopped and SFX run out */
        ~TmwSound() {StopBGM(); Close();};
    private:
        /** initial value is -1 which means error or noninitialzed.
           you can only play sounds and bgm if this is 0.
           that should be the case after calling Init() successfully */
        int isOk;

        MIDI   * mid;
        JGMOD  * mod;
        SAMPLE * sfx;

        int pan;
        int pitch;

        int ret;
        int vol_digi;
        int vol_midi;
        int vol_mod;

        /** structure can hold a sound item's attributes and data (sample-only) */
        typedef struct POOL_ITEM {
            /** incremental id of pool item */
            TMWSOUND_SID id;
            /** type of item */
            char type;
            /** (file-)name of sfx only kept for human reasons ^_^ */
            std::string fname;
            /** generic data */
            void * data;
        };

        /** list of preloaded sound data / items */
        std::list<POOL_ITEM> soundpool;
        std::list<POOL_ITEM>::iterator sounditem;
        TMWSOUND_SID items;

        bool isMaxVol(int);
};

#endif
