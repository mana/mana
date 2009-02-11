/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <SDL.h>

#include "log.h"
#include "sound.h"

#include "resources/resourcemanager.h"
#include "resources/soundeffect.h"

Sound::Sound():
    mInstalled(false),
    mSfxVolume(100),
    mMusicVolume(60)
{
}

Sound::~Sound()
{
}

void Sound::init()
{
    // Don't initialize sound engine twice
    if (mInstalled) return;

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

    mMusic = NULL;

    mInstalled = true;
}

void Sound::info()
{
    SDL_version compiledVersion;
    const SDL_version *linkedVersion;
    char driver[40] = "Unknown";
    const char *format = "Unknown";
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
    if (!mInstalled) return;

    mMusicVolume = volume;
    Mix_VolumeMusic(volume);
}

void Sound::setSfxVolume(int volume)
{
    if (!mInstalled) return;

    mSfxVolume = volume;
    Mix_Volume(-1, volume);
}

void Sound::playMusic(const std::string &filename, int loop)
{
    if (!mInstalled) return;

    if (mMusic) {
        stopMusic();
    }

    ResourceManager *resman = ResourceManager::getInstance();
    std::string path = resman->getPath("music/" + filename);

    logger->log("Sound::startMusic() Playing \"%s\" %i times", path.c_str(),
                loop);

    mMusic = Mix_LoadMUS(path.c_str());
    if (mMusic) {
        Mix_PlayMusic(mMusic, loop);
    }
    else {
        logger->log("Sound::startMusic() Warning: error loading file: %s",
                Mix_GetError());
    }
}

void Sound::stopMusic()
{
    if (!mInstalled) return;

    logger->log("Sound::stopMusic()");

    if (mMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(mMusic);
        mMusic = NULL;
    }
}

void Sound::fadeInMusic(const std::string &path, int loop, int ms)
{
    if (!mInstalled) return;

    if (mMusic) {
        stopMusic();
    }

    logger->log("Sound::fadeInMusic() Fading \"%s\" %i times (%i ms)",
                path.c_str(),
                loop, ms);

    mMusic = Mix_LoadMUS(path.c_str());
    if (mMusic) {
        Mix_FadeInMusic(mMusic, loop, ms);
    }
    else {
        logger->log("Sound::fadeInMusic() Warning: error loading file.");
    }
}

void Sound::fadeOutMusic(int ms)
{
    if (!mInstalled) return;

    logger->log("Sound::fadeOutMusic() Fading-out (%i ms)", ms);

    if (mMusic) {
        Mix_FadeOutMusic(ms);
        Mix_FreeMusic(mMusic);
        mMusic = NULL;
    }
}

void Sound::playSfx(const std::string &path)
{
    if (!mInstalled || path.length() == 0) return;

    ResourceManager *resman = ResourceManager::getInstance();
    SoundEffect *sample = resman->getSoundEffect(path);
    if (sample) {
        logger->log("Sound::playSfx() Playing: %s", path.c_str());
        sample->play(0, 120);
    }
}

void Sound::close()
{
    stopMusic();

    mInstalled = false;
    logger->log("Sound::close() Shutting down sound...");
    Mix_CloseAudio();
}
