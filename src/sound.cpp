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
#include "resources/resourcemanager.h"

Sound::Sound():
    installed(false),
    sfxVolume(100),
    musicVolume(60)
{
}

Sound::~Sound()
{
    stopMusic();
    close();
}

void Sound::init()
{
    // Don't initialize sound engine twice
    if (installed) return;

    logger->log("Sound::init() Initializing sound...");

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
        logger->log("Sound::init() Failed to initialize audio subsystem");
        return;
    }

    const size_t audioBuffer = 4096;

    const int res = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT,
            2, audioBuffer);
    if (res >= 0) {
        Mix_AllocateChannels(16);
    } else {
        logger->log("Sound::init Could not initialize audio: %s",
                Mix_GetError());
        return;
    }

    info();

    music = NULL;

    installed = true;
}

void Sound::info()
{
    SDL_version compiledVersion;
    const SDL_version *linkedVersion;
    char driver[40] = "Unknown";
    char *format = "Unknown";
    int rate = 0;
    Uint16 audioFormat = 0;
    int channels = 0;

    MIX_VERSION(&compiledVersion);
    linkedVersion = Mix_Linked_Version();

    SDL_AudioDriverName(driver, 40);

    Mix_QuerySpec(&rate, &audioFormat, &channels);
    switch (audioFormat) {
        case AUDIO_U8: format = "U8"; break;
        case AUDIO_S8: format = "S8"; break;
        case AUDIO_U16LSB: format = "U16LSB"; break;
        case AUDIO_S16LSB: format = "S16LSB"; break;
        case AUDIO_U16MSB: format = "U16MSB"; break;
        case AUDIO_S16MSB: format = "S16MSB"; break;
    }

    logger->log("Sound::info() SDL_mixer: %i.%i.%i (compiled)",
            compiledVersion.major,
            compiledVersion.minor,
            compiledVersion.patch);
    logger->log("Sound::info() SDL_mixer: %i.%i.%i (linked)",
            linkedVersion->major,
            linkedVersion->minor,
            linkedVersion->patch);
    logger->log("Sound::info() Driver: %s", driver);
    logger->log("Sound::info() Format: %s", format);
    logger->log("Sound::info() Rate: %i", rate);
    logger->log("Sound::info() Channels: %i", channels);
}

void Sound::setMusicVolume(int volume)
{
    if (!installed) return;

    musicVolume = volume;
    Mix_VolumeMusic(volume);
}

void Sound::setSfxVolume(int volume)
{
    if (!installed) return;

    sfxVolume = volume;
    Mix_Volume(-1, volume);
}

void Sound::playMusic(const char *path, int loop)
{
    if (!installed) return;

    if (music != NULL) {
        stopMusic();
    }

    logger->log("Sound::startMusic() Playing \"%s\" %i times", path, loop);

    music = Mix_LoadMUS(path);
    if (music) {
        Mix_PlayMusic(music, loop);
    }
    else {
        logger->log("Sound::startMusic() Warning: error loading file.");
    }
}

void Sound::stopMusic()
{
    if (!installed) return;

    logger->log("Sound::stopMusic()");

    if (music != NULL) {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = NULL;
    }
}

void Sound::fadeInMusic(const char *path, int loop, int ms)
{
    if (!installed) return;

    if (music != NULL) {
        stopMusic();
    }

    logger->log("Sound::fadeInMusic() Fading \"%s\" %i times (%i ms)", path,
            loop, ms);

    music = Mix_LoadMUS(path);
    if (music) {
        Mix_FadeInMusic(music, loop, ms);
    }
    else {
        logger->log("Sound::fadeInMusic() Warning: error loading file.");
    }
}

void Sound::fadeOutMusic(int ms)
{
    if (!installed) return;

    logger->log("Sound::fadeOutMusic() Fading-out (%i ms)", ms);

    if (music != NULL) {
        Mix_FadeOutMusic(ms);
        Mix_FreeMusic(music);
        music = NULL;
    }
}

void Sound::playSfx(const char *path)
{
    if (!installed) return;

    ResourceManager *resman = ResourceManager::getInstance();
    SoundEffect *sample = resman->getSoundEffect(path);
    if (sample) {
        sample->play(0, 120);
        logger->log("Sound::playSfx() Playing: %s", path);
    }
}

void Sound::close()
{
    installed = false;
    Mix_CloseAudio();
    logger->log("Sound::close() Shutting down sound...");
}
