/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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
#include "localplayer.h"
#include "log.h"
#include "sound.h"

#include "resources/music.h"
#include "resources/resourcemanager.h"
#include "resources/soundeffect.h"

/**
 * This will be set to true when the music that was playing can be freed.
 */
static bool sMusicFinished;
static bool sChannelFinished[Sound::CHANNEL_COUNT];

static void musicFinishedCallBack()
{
    sMusicFinished = true;
}

static void channelFinishedCallBack(int channel)
{
    sChannelFinished[channel] = true;
}

Sound::Sound()
{
    Mix_HookMusicFinished(musicFinishedCallBack);
    Mix_ChannelFinished(channelFinishedCallBack);
}

Sound::~Sound()
{
    Mix_HookMusicFinished(nullptr);
    Mix_ChannelFinished(nullptr);
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

    Mix_AllocateChannels(CHANNEL_COUNT);
    Mix_ReserveChannels(CHANNEL_RESERVED_COUNT);
    Mix_VolumeMusic(mMusicVolume);
    Mix_Volume(-1, mSfxVolume);
    Mix_Volume(CHANNEL_NOTIFICATIONS, mNotificationsVolume);

    info();

    mInstalled = true;

    if (!mCurrentMusicFile.empty())
        playMusic(mCurrentMusicFile);
}

void Sound::info()
{
    SDL_version compiledVersion;
    const SDL_version *linkedVersion;
    const char *format = "Unknown";
    int rate = 0;
    Uint16 audioFormat = 0;
    int channels = 0;

    MIX_VERSION(&compiledVersion);
    linkedVersion = Mix_Linked_Version();

    const char *driver = SDL_GetCurrentAudioDriver();

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
    {
        Mix_Volume(-1, mSfxVolume);
        Mix_Volume(CHANNEL_NOTIFICATIONS, mNotificationsVolume);
    }
}

void Sound::setNotificationsVolume(int volume)
{
    mNotificationsVolume = volume;

    if (mInstalled)
        Mix_Volume(CHANNEL_NOTIFICATIONS, mNotificationsVolume);
}

void Sound::playMusic(const std::string &fileName)
{
    fadeInMusic(fileName, 0);
}

void Sound::stopMusic()
{
    if (!mInstalled)
        return;

    logger->log("Sound::stopMusic()");

    haltMusic();
}

void Sound::fadeInMusic(const std::string &fileName, int ms)
{
    mCurrentMusicFile = fileName;

    if (!mInstalled)
        return;

    haltMusic();

    ResourceManager *resman = ResourceManager::getInstance();
    mMusic = resman->getMusic(paths.getStringValue("music") + fileName);

    if (mMusic)
        mMusic->play(-1, ms);
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
        // Note: The musicFinishedCallBack will take care about freeing
        // the music file at fade out ending.
    }
    else
    {
        sMusicFinished = true;
    }
}

void Sound::fadeOutAndPlayMusic(const std::string &fileName, int ms)
{
    mNextMusicFile = fileName;
    fadeOutMusic(ms);
}

void Sound::logic()
{
    if (sMusicFinished)
    {
        sMusicFinished = false;
        mMusic = nullptr;

        if (!mNextMusicFile.empty())
        {
            playMusic(mNextMusicFile);
            mNextMusicFile.clear();
        }
    }

    for (int i = 0; i < CHANNEL_COUNT; i++)
    {
        if (sChannelFinished[i])
        {
            sChannelFinished[i] = false;
            mSounds[i] = nullptr;
        }
    }
}

void Sound::playSfx(const std::string &path, int x, int y)
{
    if (!mInstalled || path.empty())
        return;

    std::string tmpPath;
    if (!path.compare(0, 4, "sfx/"))
        tmpPath = path;
    else
        tmpPath = paths.getValue("sfx", "sfx/") + path;

    ResourceManager *resman = ResourceManager::getInstance();

    if (ResourceRef<SoundEffect> sound = resman->getSoundEffect(tmpPath))
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

        int channel = sound->play(0, vol);
        if (channel != -1)
            mSounds[channel] = sound;
    }
}

void Sound::playNotification(const std::string &path)
{
    const std::string fullPath = paths.getValue("sfx", "sfx/") + path;

    ResourceManager *resman = ResourceManager::getInstance();
    if (ResourceRef<SoundEffect> sound = resman->getSoundEffect(fullPath))
    {
        int channel = sound->play(0, MIX_MAX_VOLUME, CHANNEL_NOTIFICATIONS);
        if (channel != -1)
            mSounds[channel] = sound;
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
    mMusic = nullptr;
}
