/**

	The Mana World
	Copyright 2004 The Mana World Development Team

    This file is part of The Mana World.

    The Mana World is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    The Mana World is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Mana World; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef __SOUND_H
#define __SOUND_H

#ifdef WIN32
  #pragma warning(disable:4312)
#endif
#include <allegro.h>
#include <jgmod.h>
#include <string>
using namespace std;

/**
  rewrite of non-existend sdl-soundengine using allegro
  
  Author: kth5 aka Alexander Baldeck
    pipe your question, suggestions and flames to: kth5@gawab.com

  NOTE:
      i documented all functions in their implementation. ;-)
*/

class TmwSound {
  public:
    void  Init(int, int);
    int   Close();
    
    void  StartMIDI(char *, int);
    void  StartMOD(char *, int);
    void  StopBGM(); 

    void  StartWAV(char *, int);
    void  SetVol(int, int, int);
    void  SetAdjVol(int adigi, int amid, int amod);
    
    TmwSound() {isOk=-1;}
    ~TmwSound() {StopBGM(); Close();};  // if allegros shuts down or object is deleted
                                        // any BGM is stopped and SFX runout
  private:
    int isOk;                           // initial value is -1 which means error.
                                        // you can only play sounds and bgm if this is 0.
                                        // should be the case after calling Init()
                                        // successfully
    
    MIDI   * mid;
    JGMOD  * mod;
    SAMPLE * sfx;

    int pan;
    int pitch;
    
    int ret;
    int vol_digi;
    int vol_midi;
    int vol_mod;
    
    bool isMaxVol(int);
};


#endif
