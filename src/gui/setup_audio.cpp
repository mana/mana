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

#include "gui/setup_audio.h"

#include "configuration.h"
#include "log.h"
#include "sound.h"

#include "gui/okdialog.h"

#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/slider.h"

#include "utils/gettext.h"

Setup_Audio::Setup_Audio():
    mMusicVolume(config.getIntValue("musicVolume")),
    mSfxVolume(config.getIntValue("sfxVolume")),
    mSoundEnabled(config.getBoolValue("sound")),
    mDownloadEnabled(config.getBoolValue("download-music")),
    mSoundCheckBox(new CheckBox(_("Sound"), mSoundEnabled)),
    mDownloadMusicCheckBox(new CheckBox(_("Download music"), mDownloadEnabled)),
    mSfxSlider(new Slider(0, sound.getMaxVolume())),
    mMusicSlider(new Slider(0, sound.getMaxVolume()))
{
    setName(_("Audio"));
    setDimension(gcn::Rectangle(0, 0, 250, 200));

    gcn::Label *sfxLabel = new Label(_("Sfx volume"));
    gcn::Label *musicLabel = new Label(_("Music volume"));

    mSfxSlider->setActionEventId("sfx");
    mMusicSlider->setActionEventId("music");

    mSfxSlider->addActionListener(this);
    mMusicSlider->addActionListener(this);

    mSoundCheckBox->setPosition(10, 10);

    mSfxSlider->setValue(mSfxVolume);
    mMusicSlider->setValue(mMusicVolume);

    mSfxSlider->setWidth(90);
    mMusicSlider->setWidth(90);

    // Do the layout
    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, mSoundCheckBox);
    place(0, 1, mSfxSlider);
    place(1, 1, sfxLabel);
    place(0, 2, mMusicSlider);
    place(1, 2, musicLabel);
    place(0, 3, mDownloadMusicCheckBox);

    setDimension(gcn::Rectangle(0, 0, 370, 280));
}

void Setup_Audio::apply()
{
    mSoundEnabled = mSoundCheckBox->isSelected();
    mDownloadEnabled = mDownloadMusicCheckBox->isSelected();
    mSfxVolume = config.getIntValue("sfxVolume");
    mMusicVolume = config.getIntValue("musicVolume");

    config.setValue("sound", mSoundEnabled);

    // Display a message if user has selected to download music,
    // And if downloadmusic is not already enabled
    if (mDownloadEnabled && !config.getBoolValue("download-music"))
    {
        new OkDialog(_("Notice"),_("You may have to restart your client if you want to download new music"));
    }
    config.setValue("download-music", mDownloadEnabled);

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

    config.setValue("sound", mSoundEnabled);
    config.setValue("download-music", mDownloadEnabled);
    config.setValue("sfxVolume", mSfxVolume);
    config.setValue("musicVolume", mMusicVolume);
}

void Setup_Audio::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "sfx")
    {
        config.setValue("sfxVolume", (int) mSfxSlider->getValue());
        sound.setSfxVolume((int) mSfxSlider->getValue());
    }
    else if (event.getId() == "music")
    {
        config.setValue("musicVolume", (int) mMusicSlider->getValue());
        sound.setMusicVolume((int) mMusicSlider->getValue());
    }
}
