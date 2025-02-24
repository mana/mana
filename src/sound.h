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

#pragma once

#include "resources/resource.h"

#include <SDL_mixer.h>

#include <string>

class Music;
class SoundEffect;

/** Sound engine
 *
 * \ingroup CORE
 */
class Sound
{
    public:
        Sound();
        ~Sound();

        /**
         * Installs the sound engine.
         */
        void init();

        /**
         * Removes all sound functionalities.
         */
        void close();

        /**
         * Starts background music.
         *
         * @param fileName The name of the music file.
         */
        void playMusic(const std::string &fileName);

        /**
         * Stops currently running background music track.
         */
        void stopMusic();

        /**
         * Fades in background music.
         *
         * @param fileName The name of the music file.
         * @param ms       Duration of fade-in effect (ms)
         */
        void fadeInMusic(const std::string &fileName, int ms = 1000);

        /**
         * Fades out currently running background music track.
         *
         * @param ms   Duration of fade-out effect (ms)
         */
        void fadeOutMusic(int ms = 1000);

        /**
         * Fades out a background music and play a new one.
         *
         * @param fileName The name of the music file.
         * @param ms       Duration of fade-out effect (ms)
         */
        void fadeOutAndPlayMusic(const std::string &fileName, int ms = 1000);

        int getMaxVolume() const
        { return MIX_MAX_VOLUME; }

        void setMusicVolume(int volume);
        void setSfxVolume(int volume);
        void setNotificationsVolume(int volume);

        /**
         * Plays a sound at the specified location.
         *
         * @param path The resource path to the sound file.
         * @param x The vertical distance of the sound in pixels.
         * @param y The horizontal distance of the sound in pixels.
         */
        void playSfx(const std::string &path, int x = 0, int y = 0);

        /**
         * Plays a sound on the notification channel.
         *
         * @param path The resource path to the sound file.
         */
        void playNotification(const std::string &path);

        /**
         * The sound logic.
         *
         * Checks whether the music and sound effects can be freed after they
         * finished playing, and whether new music has to be played.
         */
        void logic();

        enum Channel {
            CHANNEL_NOTIFICATIONS = 0,
            CHANNEL_RESERVED_COUNT,

            CHANNEL_COUNT = 16,
        };

    private:
        /** Logs various info about sound device. */
        void info();

        /** Halts and frees currently playing music. */
        void haltMusic();

        /**
         * When calling fadeOutAndPlayMusic(),
         * the music file below will then be played
         */
        std::string mNextMusicFile;

        bool mInstalled = false;

        int mSfxVolume = 100;
        int mNotificationsVolume = 100;
        int mMusicVolume = 60;

        std::string mCurrentMusicFile;
        ResourceRef<Music> mMusic;
        ResourceRef<SoundEffect> mSounds[CHANNEL_COUNT];
};

extern Sound sound;
