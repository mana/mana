/*
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

#ifdef WIN32
	#pragma warning(disable:4312)
#endif

#include "sound.h"

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
void TmwSound::Init(int voices, int mod_voices) {
	isOk = -1;

	if(mod_voices >= voices)
		throw("No voices left for SFX! Sound will be disabled!");

	install_timer();
	reserve_voices (voices, -1);

	#ifdef WIN32
		if (install_sound (DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) < 0)
	#else
		if (install_sound (DIGI_AUTODETECT, MIDI_NONE, NULL) < 0)
	#endif
			throw("Could not initialize sound... :-(");


	if (install_mod (mod_voices) < 0)
		throw("Could not install MOD player... :-(");

	mod = NULL;
	mid = NULL;
	sfx = NULL;

	pan = 128;
	pitch=1000;

	items = 0;

	isOk = 0;
}

/**
	\brief set the volume value-range: 0-255
	\param digi for digital playback
	\param mid for midi playback
	\param mod for... aw, you guess ^^

	NOTE:
		all values may only be between 0-255 where 0 means
		muted.
*/
void TmwSound::SetVol(int digi, int mid, int mod) {
	if(isOk==-1)
		return;
	set_volume(digi, mid);
	set_mod_volume(mod);
	set_hardware_volume(digi, mid);

	if(isMaxVol(vol_digi + digi)==false) vol_digi += digi;
	if(isMaxVol(vol_midi + mid) ==false) vol_midi += mid;
	if(isMaxVol(vol_mod  + mod) ==false) vol_mod  += mod;
}

/**
	\brief adjusts current volume
	\param adigi for digital playback
	\param amid for midi playback
	\param amod for... aw, you guess ^^

	NOTE:
		all values may only be between 0-255 where 0 means
		muted.
*/
void TmwSound::SetAdjVol(int adigi, int amid, int amod) {
	if(isOk==-1)
		return;
	set_volume(vol_digi + adigi, vol_midi + amid);
	set_mod_volume(vol_mod + amod);

	if(isMaxVol(vol_digi + adigi)==false) vol_digi += adigi;
	if(isMaxVol(vol_midi + amid) ==false) vol_midi += amid;
	if(isMaxVol(vol_mod  + amod) ==false) vol_mod  += amod;
}

/**
	\brief start BGM using a midi file
	\param in full path of midi file
	\param loop how many times should the midi be looped? (-1 = infinite)

	NOTE:
		playing midi does not steal away any voices but
		does not work w/ most soundcards w/o software
		emulation. this means that *nix-users will most
		probably be left out. do not use this unless we
		find a way to always get it to work. :-)

		at this point of time only standard RMI midi files
		can be played. so no m$ extensions like GS and such.
*/
void TmwSound::StartMIDI(char *in, int loop) {
	if(isOk==-1)
		return;

	mid = load_midi(in);
	if (!mid) {
		isOk=-1;
		throw("Could not load MIDI file!");
	}

	play_midi(mid, TRUE);
}

/**
	\brief start BGM using a mod file
	\param in full path of mod file
	\param loop how many times should the midi be looped? (-1 = infinite)

	NOTE:
		playing mod is a pretty good choice. most of the work
		is being done by the cpu so it's not dependend on the
		sound-card how things sound. if it works, it just
		works! ;-)

		JGMOD supports several formats:
				MOD
				S3M
				XM
				Unreal
				and S3M (in UMX extension)
*/
void TmwSound::StartMOD(char * in, int loop) {
	if(isOk==-1)
		return;

	mod = load_mod(in);
	if(!mod) {
		isOk=-1;
		throw("Error reading MOD file...");
	}
	play_mod(mod, TRUE);
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
void TmwSound::StopBGM() {
	if(isOk==-1)
		return;

	play_midi(NULL,-1);
	stop_mod();

	mod = NULL;
	mid = NULL;
}

/**
	\brief play short sample usually for sfx
	\param in full path to the sample file
	\param pan panning of the sound, values can be 0-255 where 128 is the middle

	NOTE:
		later on this will be a subsequent call to another
		function that preloads all wavs corresponding to
		the current area (e.g. monster screams) to memory.
		right now the function loads the file from hdd
		everytime you want it to be played. this is kind of
		resource intensive even though most OS'ses cache a
		already loaded file for some time.

		allegro supports different formats but this is not
		stated clear enough - these will work for sure:
				WAV
				VOC

		i don't know what kind of samples are necessary so we
		need to test this thoroughly.
*/
void TmwSound::StartWAV(char * in, int pan) {
	if(isOk==-1)
		return;

	sfx = load_sample(in);
	if (!sfx)
		throw("Error reading WAV file...");

	play_sample(sfx, vol_digi, pan, pitch, FALSE);
}

/**
	\brief preloads a sound-item into buffer
	\param fpath full path to file
	\param type type of item (TMWSOUND_MOD, TMWSOUND_MID, TMWSOUND_SFX)

	NOTE:
		only TMWSOUND_SFX items get preloaded. everything
		else will only store the full path to the file.

		please make sure that the object is not loaded more
		than once since the function will not be able to run
		checks for its own!

		the return value should be kept as a reference to the
		object loaded. if not it is practicaly lost.
*/
TMWSOUND_SID TmwSound::LoadItem(char *fpath, char type) {
	POOL_ITEM item;
	if(type == TMWSOUND_SFX) {
		if(!(item.data = (void*)load_sample(fpath)))
			throw(sprintf("Unable to load sample: %s\n", fpath));
	}

	items++;
	item.id = items;
	item.type = type;
	item.fname = fpath;

	soundpool.push_front(item);
	return item.id;
}

/**
	\brief unloads an item from the soundpool
	\param id id returned by LoadItem()
*/
void TmwSound::UnloadItem(TMWSOUND_SID id) {
	int cnt = 0;
	POOL_ITEM item;
	for(sounditem = soundpool.begin(); sounditem != soundpool.end(); sounditem++) {
		item = *sounditem;
		if(item.id == id) {
			destroy_sample((SAMPLE*)item.data);
			soundpool.erase(sounditem);
			return;
		}
		cnt++;
	}
}

/**
	\brief plays an item in soundpool
	\param id id returned by LoadItem()
	\param loop loop n times (-1 is infinite)
*/
void TmwSound::PlayItem(TMWSOUND_SID id, int loop) {
	POOL_ITEM item;
	for(sounditem = soundpool.begin(); sounditem != soundpool.end(); sounditem++) {
		item = *sounditem;
		if(item.id = id) {
			switch(item.type) {
				case TMWSOUND_SFX :
					play_sample((SAMPLE*)item.data, vol_digi, pan, pitch, FALSE);
					break;
				case TMWSOUND_MOD :
					StartMOD((char*)item.fname.c_str(),loop);
					break;
				case TMWSOUND_MID :
					StartMIDI((char*)item.fname.c_str(),loop);
					break;
			}
		}
	}
}

/**
	\brief deinstall all sound functionality

	NOTE:
		normally you won't need to call this since this is
		done by allegro when shutting itself down. but if
		you find a reason to delete the sound-engine from
		memory (e.g. garbage-collection) feel free to use
		it. :-P
*/
void TmwSound::Close(void) {
	mod = NULL;
	mid = NULL;
	sfx = NULL;

	remove_mod();
	remove_sound();
	isOk = -1;
}

/**
	\brief checks if value equals min-/maximum volume and returns
	true if that's the case.
*/
bool TmwSound::isMaxVol(int vol) {
	if( vol > 0 && vol < 255 ) return false;
	else return true;
}
