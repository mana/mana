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

#include "setup_audio.h"

#include <guichan/widgets/label.hpp>

#include "checkbox.h"
#include "ok_dialog.h"
#include "slider.h"

#include "../configuration.h"
#include "../log.h"
#include "../sound.h"

Setup_Audio::Setup_Audio():
    mMusicVolume((int)config.getValue("musicVolume", 60)),
    mSfxVolume((int)config.getValue("sfxVolume", 100)),
    mSoundEnabled(config.getValue("sound", 0)),
    mSoundCheckBox(new CheckBox("Sound", mSoundEnabled)),
    mSfxSlider(new Slider(0, 128)),
    mMusicSlider(new Slider(0, 128))
{
    setOpaque(false);

    gcn::Label *sfxLabel = new gcn::Label("Sfx volume");
    gcn::Label *musicLabel = new gcn::Label("Music volume");

    mSfxSlider->setActionEventId("sfx");
    mMusicSlider->setActionEventId("music");

    mSfxSlider->addActionListener(this);
    mMusicSlider->addActionListener(this);

    mSoundCheckBox->setPosition(10, 10);
    mSfxSlider->setDimension(gcn::Rectangle(10, 30, 100, 10));
    mMusicSlider->setDimension(gcn::Rectangle(10, 50, 100, 10));
    sfxLabel->setPosition(20 + mSfxSlider->getWidth(), 27);
    musicLabel->setPosition(20 + mMusicSlider->getWidth(), 47);

    mSfxSlider->setValue(mSfxVolume);
    mMusicSlider->setValue(mMusicVolume);

    add(mSoundCheckBox);
    add(mSfxSlider);
    add(mMusicSlider);
    add(sfxLabel);
    add(musicLabel);
}

void Setup_Audio::apply()
{
    if (mSoundCheckBox->isMarked())
    {
        config.setValue("sound", 1);
        try {
            sound.init();
        }
        catch (const char *err)
        {
            new OkDialog("Sound Engine", err);
            logger->log("Warning: %s", err);
        }
    }
    else
    {
        config.setValue("sound", 0);
        sound.close();
    }

    mSoundEnabled = config.getValue("sound", 0);
    mSfxVolume = (int)config.getValue("sfxVolume", 100);
    mMusicVolume = (int)config.getValue("musicVolume", 60);
}

void Setup_Audio::cancel()
{
    mSoundCheckBox->setMarked(mSoundEnabled);

    sound.setSfxVolume(mSfxVolume);
    mSfxSlider->setValue(mSfxVolume);

    sound.setMusicVolume(mMusicVolume);
    mMusicSlider->setValue(mMusicVolume);

    config.setValue("sound", mSoundEnabled ? 1 : 0);
    config.setValue("sfxVolume", mSfxVolume ? 1 : 0);
    config.setValue("musicVolume", mMusicVolume);
}

void Setup_Audio::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "sfx")
    {
        config.setValue("sfxVolume", (int)mSfxSlider->getValue());
        sound.setSfxVolume((int)mSfxSlider->getValue());
    }
    else if (event.getId() == "music")
    {
        config.setValue("musicVolume", (int)mMusicSlider->getValue());
        sound.setMusicVolume((int)mMusicSlider->getValue());
    }
}
