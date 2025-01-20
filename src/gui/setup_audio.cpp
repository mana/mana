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

#include "gui/setup_audio.h"

#include "configuration.h"
#include "log.h"
#include "sound.h"

#include "gui/okdialog.h"

#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/slider.h"

#include "utils/gettext.h"

Setup_Audio::Setup_Audio():
    mMusicVolume(config.musicVolume),
    mSfxVolume(config.sfxVolume),
    mNotificationsVolume(config.notificationsVolume),
    mSoundEnabled(config.sound),
    mDownloadEnabled(config.downloadMusic),
    mSoundCheckBox(new CheckBox(_("Sound"), mSoundEnabled)),
    mDownloadMusicCheckBox(new CheckBox(_("Download music"), mDownloadEnabled)),
    mSfxSlider(new Slider(0, sound.getMaxVolume())),
    mNotificationsSlider(new Slider(0, sound.getMaxVolume())),
    mMusicSlider(new Slider(0, sound.getMaxVolume()))
{
    setName(_("Audio"));

    gcn::Label *sfxLabel = new Label(_("SFX volume"));
    gcn::Label *notificationsLabel = new Label(_("Notifications volume"));
    gcn::Label *musicLabel = new Label(_("Music volume"));

    mSfxSlider->setActionEventId("sfx");
    mNotificationsSlider->setActionEventId("notifications");
    mMusicSlider->setActionEventId("music");

    mSfxSlider->addActionListener(this);
    mNotificationsSlider->addActionListener(this);
    mMusicSlider->addActionListener(this);

    mSoundCheckBox->setPosition(10, 10);

    mSfxSlider->setValue(mSfxVolume);
    mNotificationsSlider->setValue(mNotificationsVolume);
    mMusicSlider->setValue(mMusicVolume);

    mSfxSlider->setWidth(90);
    mNotificationsSlider->setWidth(90);
    mMusicSlider->setWidth(90);

    // Do the layout
    place(0, 0, mSoundCheckBox);
    place(0, 1, mSfxSlider);
    place(1, 1, sfxLabel);
    place(0, 2, mNotificationsSlider);
    place(1, 2, notificationsLabel);
    place(0, 3, mMusicSlider);
    place(1, 3, musicLabel);
    place(0, 4, mDownloadMusicCheckBox);
}

void Setup_Audio::apply()
{
    mSoundEnabled = mSoundCheckBox->isSelected();
    mDownloadEnabled = mDownloadMusicCheckBox->isSelected();
    mSfxVolume = config.sfxVolume;
    mNotificationsVolume = config.sfxVolume;
    mMusicVolume = config.musicVolume;

    config.sound = mSoundEnabled;

    // Display a message if user has selected to download music,
    // And if downloadmusic is not already enabled
    if (mDownloadEnabled && !config.downloadMusic)
    {
        new OkDialog(_("Notice"),_("You may have to restart your client if you want to download new music"));
    }
    config.downloadMusic = mDownloadEnabled;

    if (mSoundEnabled)
    {
        try
        {
            sound.init();
        }
        catch (const char *err)
        {
            new OkDialog(_("Sound Engine"), err);
            logger->log("Warning: %s", err);
        }
    }
    else
    {
        sound.close();
    }
}

void Setup_Audio::cancel()
{
    mSoundCheckBox->setSelected(mSoundEnabled);
    mDownloadMusicCheckBox->setSelected(mDownloadEnabled);

    sound.setSfxVolume(mSfxVolume);
    mSfxSlider->setValue(mSfxVolume);

    sound.setMusicVolume(mMusicVolume);
    mMusicSlider->setValue(mMusicVolume);

    config.sound = mSoundEnabled;
    config.downloadMusic = mDownloadEnabled;
    config.sfxVolume = mSfxVolume;
    config.musicVolume = mMusicVolume;
}

void Setup_Audio::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "sfx")
    {
        int volume = (int) mSfxSlider->getValue();
        config.sfxVolume = volume;
        sound.setSfxVolume(volume);
    }
    else if (event.getId() == "notifications")
    {
        int volume = (int) mNotificationsSlider->getValue();
        config.notificationsVolume = volume;
        sound.setNotificationsVolume(volume);
    }
    else if (event.getId() == "music")
    {
        int volume = (int) mMusicSlider->getValue();
        config.musicVolume = volume;
        sound.setMusicVolume(volume);
    }
}
