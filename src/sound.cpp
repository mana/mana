/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <SDL.h>

#include "configuration.h"
#include "game.h"
#include "localplayer.h"
#include "log.h"
#include "sound.h"

#include "resources/resourcemanager.h"
#include "resources/soundeffect.h"

/**
 * This will be set to true, when a music can be freed after a fade out
 * Currently used by fadeOutCallBack()
 */
static bool sFadingOutEnded = false;

/**
 * Callback used at end of fadeout.
 * It is called by Mix_MusicFadeFinished().
 */
static void fadeOutCallBack()
{
    sFadingOutEnded = true;
}

Sound::Sound():
    mInstalled(false),
    mSfxVolume(100),
    mMusicVolume(60),
    mMusic(NULL)
{
    // This set up our callback function used to
    // handle fade outs endings.
    sFadingOutEnded = false;
    Mix_HookMusicFinished(fadeOutCallBack);
}

Sound::~Sound()
{
    // Unlink the callback function.
    Mix_HookMusicFinished(NULL);
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
    switch (audioFormat)
    {
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
    std::string path = resman->getPath(paths.getStringValue("music") + filename);

    if (path.find(".zip/") != std::string::npos ||
        path.find(".zip\\") != std::string::npos)
    {
        // Music file is a virtual file inside a zip archive - we have to copy
        // it to a temporary physical file so that SDL_mixer can stream it.
        logger->log("Loading music \"%s\" from temporary file tempMusic.ogg",
                    path.c_str());
        bool success = resman->copyFile(
                               paths.getStringValue("music")
                               + filename, "tempMusic.ogg");
        if (success)
            path = resman->getPath("tempMusic.ogg");
        else
            return NULL;
    }
    else
    {
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

    haltMusic();
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

    if (mMusic)
    {
        Mix_FadeOutMusic(ms);
        // Note: The fadeOutCallBack handler will take care about freeing
        // the music file at fade out ending.
    }
    else
    {
        sFadingOutEnded = true;
    }
}

void Sound::fadeOutAndPlayMusic(const std::string &path, int ms)
{
    mNextMusicPath = path;
    fadeOutMusic(ms);
}

void Sound::logic()
{
    if (sFadingOutEnded)
    {
        if (mMusic)
        {
            Mix_FreeMusic(mMusic);
            mMusic = NULL;
        }
        sFadingOutEnded = false;

        if (!mNextMusicPath.empty())
        {
            playMusic(mNextMusicPath);
            mNextMusicPath.clear();
        }
    }
}

void Sound::playSfx(const std::string &path, int x, int y)
{
    if (!mInstalled || path.empty())
        return;

    std::string tmpPath;
    if (!path.find("sfx/"))
        tmpPath = path;
    else
        tmpPath = paths.getValue("sfx", "sfx/") + path;

    ResourceManager *resman = ResourceManager::getInstance();

    if (SoundEffect *sample = resman->getSoundEffect(tmpPath))
    {
        logger->log("Sound::playSfx() Playing: %s", path.c_str());
        int vol = 120;

        if (local_player && (x > 0 || y > 0))
        {
            const Vector &pos = local_player->getPosition();
            const int dx = std::abs((int) pos.x - x);
            const int dy = std::abs((int) pos.y - y);
            const int dist = std::max(dx, dy);

            // Volume goes down one level with each 4 pixels
            vol -= std::min(120, dist / 4);
        }

        sample->play(0, vol);
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
