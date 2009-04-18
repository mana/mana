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
    mMusicVolume(60),
    mMusic(NULL)
{
}

Sound::~Sound()
{
}

void Sound::init()
{
    // Don't initialize sound engine twice
    if (mInstalled)
        return;

    logger->log("Sound::init() Initializing sound...");

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1)
    {
        logger->log("Sound::init() Failed to initialize audio subsystem");
        return;
    }

    const size_t audioBuffer = 4096;

    const int res = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT,
                                  MIX_DEFAULT_CHANNELS, audioBuffer);
    if (res < 0)
    {
        logger->log("Sound::init Could not initialize audio: %s",
                    Mix_GetError());
        return;
    }

    Mix_AllocateChannels(16);
    Mix_VolumeMusic(mMusicVolume);
    Mix_Volume(-1, mSfxVolume);

    info();

    mInstalled = true;

    if (!mCurrentMusicFile.empty())
        playMusic(mCurrentMusicFile);
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

int Sound::getMaxVolume() const
{
    return MIX_MAX_VOLUME;
}

void Sound::setMusicVolume(int volume)
{
    mMusicVolume = volume;

    if (mInstalled)
        Mix_VolumeMusic(mMusicVolume);
}

void Sound::setSfxVolume(int volume)
{
    mSfxVolume = volume;

    if (mInstalled)
        Mix_Volume(-1, mSfxVolume);
}

static Mix_Music *loadMusic(const std::string &filename)
{
    ResourceManager *resman = ResourceManager::getInstance();
    std::string path = resman->getPath("music/" + filename);

    if (path.find(".zip/") != std::string::npos ||
        path.find(".zip\\") != std::string::npos)
    {
        // Music file is a virtual file inside a zip archive - we have to copy
        // it to a temporary physical file so that SDL_mixer can stream it.
        logger->log("Loading music \"%s\" from temporary file tempMusic.ogg",
                    path.c_str());
        bool success = resman->copyFile("music/" + filename, "tempMusic.ogg");
        if (success)
        {
            path = resman->getPath("tempMusic.ogg");
        } else {
            return NULL;
        }
    } else {
        logger->log("Loading music \"%s\"", path.c_str());
    }

    Mix_Music *music = Mix_LoadMUS(path.c_str());

    if (!music)
    {
        logger->log("Mix_LoadMUS() Error loading '%s': %s", path.c_str(),
                    Mix_GetError());
    }

    return music;
}

void Sound::playMusic(const std::string &filename)
{
    mCurrentMusicFile = filename;

    if (!mInstalled)
        return;

    haltMusic();

    if ((mMusic = loadMusic(filename)))
        Mix_PlayMusic(mMusic, -1); // Loop forever
}

void Sound::stopMusic()
{
    if (!mInstalled)
        return;

    logger->log("Sound::stopMusic()");

    if (mMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(mMusic);
        mMusic = NULL;
    }
}

void Sound::fadeInMusic(const std::string &path, int ms)
{
    mCurrentMusicFile = path;

    if (!mInstalled)
        return;

    haltMusic();

    if ((mMusic = loadMusic(path.c_str())))
        Mix_FadeInMusic(mMusic, -1, ms); // Loop forever
}

void Sound::fadeOutMusic(int ms)
{
    mCurrentMusicFile.clear();

    if (!mInstalled)
        return;

    logger->log("Sound::fadeOutMusic() Fading-out (%i ms)", ms);

    if (mMusic) {
        Mix_FadeOutMusic(ms);
        Mix_FreeMusic(mMusic);
        mMusic = NULL;
    }
}

void Sound::playSfx(const std::string &path)
{
    if (!mInstalled || path.empty())
        return;

    ResourceManager *resman = ResourceManager::getInstance();
    SoundEffect *sample = resman->getSoundEffect(path);
    if (sample) {
        logger->log("Sound::playSfx() Playing: %s", path.c_str());
        sample->play(0, 120);
    }
}

void Sound::close()
{
    if (!mInstalled)
        return;

    haltMusic();
    logger->log("Sound::close() Shutting down sound...");
    Mix_CloseAudio();

    mInstalled = false;
}

void Sound::haltMusic()
{
    if (!mMusic)
        return;

    Mix_HaltMusic();
    Mix_FreeMusic(mMusic);
    mMusic = NULL;
}
