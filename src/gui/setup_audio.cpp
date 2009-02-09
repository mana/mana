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

#include <guichan/widgets/label.hpp>

#include "checkbox.h"
#include "ok_dialog.h"
#include "setup_audio.h"
#include "slider.h"

#include "widgets/layouthelper.h"

#include "../configuration.h"
#include "../log.h"
#include "../sound.h"

#include "../utils/gettext.h"

Setup_Audio::Setup_Audio():
    mMusicVolume((int)config.getValue("musicVolume", 60)),
    mSfxVolume((int)config.getValue("sfxVolume", 100)),
    mSoundEnabled(config.getValue("sound", 0)),
    mSoundCheckBox(new CheckBox(_("Sound"), mSoundEnabled)),
    mSfxSlider(new Slider(0, 128)),
    mMusicSlider(new Slider(0, 128))
{
    setOpaque(false);

    gcn::Label *sfxLabel = new gcn::Label(_("Sfx volume"));
    gcn::Label *musicLabel = new gcn::Label(_("Music volume"));

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

    setDimension(gcn::Rectangle(0, 0, 290, 250));
}

void Setup_Audio::apply()
{
    if (mSoundCheckBox->isSelected())
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
    mSoundCheckBox->setSelected(mSoundEnabled);

    sound.setSfxVolume(mSfxVolume);
    mSfxSlider->setValue(mSfxVolume);

    sound.setMusicVolume(mMusicVolume);
    mMusicSlider->setValue(mMusicVolume);

    config.setValue("sound", mSoundEnabled ? true : false);
    config.setValue("sfxVolume", mSfxVolume);
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
