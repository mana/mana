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

#include "setup.h"

#include <iostream>
#include <sstream>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "checkbox.h"
#include "listbox.h"
#include "ok_dialog.h"
#include "scrollarea.h"
#include "slider.h"

#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"
#include "../main.h"
#include "../sound.h"

extern Graphics *graphics;
#include "chat.h"
#include "equipmentwindow.h"
#include "help.h"
#include "inventorywindow.h"
#include "minimap.h"
#include "skill.h"
#include "status.h"

ModeListModel::ModeListModel()
{
    SDL_Rect **modes;

    /* Get available fullscreen/hardware modes */
    modes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_HWSURFACE);

    /* Check is there are any modes available */
    if (modes == (SDL_Rect **)0) {
        logger->log("No modes available");
    }

    /* Check if our resolution is restricted */
    if (modes == (SDL_Rect **)-1) {
        logger->log("All resolutions available");
    }
    else{
        /* Print valid modes */
        /*logger->log("Available Modes");
        for (int i = 0; modes[i]; ++i) {
            logger->log("  %dx%d", modes[i]->w, modes[i]->h);
            std::stringstream mode;
            mode << (int)modes[i]->w << "x" << (int)modes[i]->h;
            videoModes.push_back(mode.str());
        }*/
    }
}

ModeListModel::~ModeListModel()
{
}

int ModeListModel::getNumberOfElements()
{
    return videoModes.size();
}

std::string ModeListModel::getElementAt(int i)
{
    return videoModes[i];
}


Setup::Setup():
    Window("Setup")
{
    videoLabel = new gcn::Label("Video settings");
    modeListModel = new ModeListModel();
    modeList = new ListBox(modeListModel);
    modeList->setEnabled(false);
    scrollArea = new ScrollArea(modeList);
    fsCheckBox = new CheckBox("Full screen", false);
    openGLCheckBox = new CheckBox("OpenGL", false);
    customCursorCheckBox = new CheckBox("Custom cursor");
    alphaLabel = new gcn::Label("Gui opacity");
    alphaSlider = new Slider(0.2, 1.0);
    audioLabel = new gcn::Label("Audio settings");
    soundCheckBox = new CheckBox("Sound", false);
    sfxSlider = new Slider(0, 128);
    musicSlider = new Slider(0, 128);
    sfxLabel = new gcn::Label("Sfx volume");
    musicLabel = new gcn::Label("Music volume");
    applyButton = new Button("Apply");
    cancelButton = new Button("Cancel");
    resetWinsToDefault = new Button("Reset Windows");

    // Set events
    applyButton->setEventId("apply");
    cancelButton->setEventId("cancel");
    resetWinsToDefault->setEventId("winsToDefault");
    alphaSlider->setEventId("guialpha");
    sfxSlider->setEventId("sfx");
    musicSlider->setEventId("music");
    customCursorCheckBox->setEventId("customcursor");

    // Set dimensions/positions
    setContentSize(240, 246);

    videoLabel->setPosition(getWidth() - videoLabel->getWidth() - 5, 10);
    scrollArea->setDimension(gcn::Rectangle(10, 30, 90, 50));
    modeList->setDimension(gcn::Rectangle(0, 0, 60, 50));
    fsCheckBox->setPosition(110, 30);
    openGLCheckBox->setPosition(110, 50);
    customCursorCheckBox->setPosition(110, 70);
    alphaSlider->setDimension(gcn::Rectangle(10, 100, 100, 10));
    alphaLabel->setPosition(20 + alphaSlider->getWidth(), 97);

    audioLabel->setPosition(getWidth() - videoLabel->getWidth() - 5, 120);
    soundCheckBox->setPosition(10, 140);
    sfxSlider->setDimension(gcn::Rectangle(10, 160, 100, 10));
    musicSlider->setDimension(gcn::Rectangle(10, 180, 100, 10));
    sfxLabel->setPosition(20 + sfxSlider->getWidth(), 157);
    musicLabel->setPosition(20 + musicSlider->getWidth(), 177);
    resetWinsToDefault->setPosition(20, 197);
    cancelButton->setPosition(
            getWidth() - 10 - cancelButton->getWidth(),
            getHeight() - 25 - cancelButton->getHeight());
    applyButton->setPosition(
            cancelButton->getX() - 10 - applyButton->getWidth(),
            getHeight() - 25 - applyButton->getHeight());

    // Listen for actions
    applyButton->addActionListener(this);
    cancelButton->addActionListener(this);
    resetWinsToDefault->addActionListener(this);
    alphaSlider->addActionListener(this);
    sfxSlider->addActionListener(this);
    musicSlider->addActionListener(this);
    customCursorCheckBox->addActionListener(this);

    // Assemble dialog
    add(videoLabel);
    add(scrollArea);
    add(fsCheckBox);
    add(openGLCheckBox);
    add(customCursorCheckBox);
    add(audioLabel);
    add(soundCheckBox);
    add(alphaSlider);
    add(alphaLabel);
    add(sfxSlider);
    add(musicSlider);
    add(sfxLabel);
    add(musicLabel);
    add(resetWinsToDefault);
    add(applyButton);
    add(cancelButton);

    setLocationRelativeTo(getParent());

    // Load default settings
    modeList->setSelected(-1);

    // Full Screen
    fullScreenEnabled = config.getValue("screen", 0);
    fsCheckBox->setMarked(fullScreenEnabled);

    // Sound
    soundEnabled = config.getValue("sound", 0);
    soundCheckBox->setMarked(soundEnabled);

    sfxVolume = (int)config.getValue("sfxVolume", 100);
    sfxSlider->setValue(sfxVolume);

    musicVolume = (int)config.getValue("musicVolume", 60);
    musicSlider->setValue(musicVolume);

    // Graphics
    customCursorEnabled = config.getValue("customcursor", 1);
    customCursorCheckBox->setMarked(customCursorEnabled);

    opacity = config.getValue("guialpha", 0.8);
    alphaSlider->setValue(opacity);

    openGLEnabled = config.getValue("opengl", 0);
    openGLCheckBox->setMarked(openGLEnabled);
}

Setup::~Setup()
{
    delete videoLabel;
    delete modeListModel;
    delete modeList;
    delete scrollArea;
    delete fsCheckBox;
    delete openGLCheckBox;
    delete customCursorCheckBox;
    delete soundCheckBox;
    delete audioLabel;
    delete resetWinsToDefault;
    delete applyButton;
    delete cancelButton;
    delete alphaSlider;
    delete alphaLabel;
    delete sfxSlider;
    delete musicSlider;
    delete sfxLabel;
    delete musicLabel;
}

void Setup::action(const std::string &eventId)
{
    if (eventId == "sfx")
    {
        config.setValue("sfxVolume", (int)sfxSlider->getValue());
        sound.setSfxVolume((int)sfxSlider->getValue());
    }
    else if (eventId == "music")
    {
        config.setValue("musicVolume", (int)musicSlider->getValue());
        sound.setMusicVolume((int)musicSlider->getValue());
    }
    else if (eventId == "guialpha")
    {
        config.setValue("guialpha", alphaSlider->getValue());
    }
    else if (eventId == "customcursor")
    {
        config.setValue("customcursor",
                customCursorCheckBox->isMarked() ? 1 : 0);
    }
    else if (eventId == "apply")
    {
        setVisible(false);

        // Full screen changes
        bool fullscreen = fsCheckBox->isMarked();
        if (fullscreen != (config.getValue("screen", 0) == 1)) 
        {
            if (!graphics->setFullscreen(fullscreen))
            {
                fullscreen = !fullscreen;
                if (!graphics->setFullscreen(fullscreen))
                {
                    std::cerr << "Failed to switch to " <<
                        (fullscreen ? "windowed" : "fullscreen") <<
                        "mode and restoration of old mode also failed!" <<
                        std::endl;
                    exit(1);
                }
            }
            config.setValue("screen", fullscreen ? 1 : 0);
        }

        // Sound settings changes
        if (soundCheckBox->isMarked())
        {
            config.setValue("sound", 1);
            try {
                sound.init();
            }
            catch (const char *err)
            {
                new OkDialog(this, "Sound Engine", err);
                logger->log("Warning: %s", err);
            }
        }
        else
        {
            config.setValue("sound", 0);
            sound.close();
        }

        // OpenGL change
        if (openGLCheckBox->isMarked() != openGLEnabled)
        {
            config.setValue("opengl", openGLCheckBox->isMarked() ? 1 : 0);

            // OpenGL can currently only be changed by restarting, notify user.
            new OkDialog(this, "Changing OpenGL",
                               "Applying change to OpenGL requires restart.");
        }

        // We sync old and new values at apply time
        // Screen
        fullScreenEnabled = config.getValue("screen", 0);

        // Sound
        soundEnabled = config.getValue("sound", 0);
        sfxVolume = (int)config.getValue("sfxVolume", 100);
        musicVolume = (int)config.getValue("musicVolume", 60);

        // Graphics
        customCursorEnabled = config.getValue("customcursor", 1);
        opacity = config.getValue("guialpha", 0.8);
        openGLEnabled = config.getValue("opengl", 0);
    }
    else if (eventId == "cancel")
    {
        setVisible(false);

        // Restoring old values when cancelling
        // Screen
        config.setValue("screen", fullScreenEnabled ? 1 : 0);
        fsCheckBox->setMarked(fullScreenEnabled);

        // Sound
        config.getValue("sound", soundEnabled ? 1 : 0);
        soundCheckBox->setMarked(soundEnabled);

        config.getValue("sfxVolume", sfxVolume ? 1 : 0);
        sound.setSfxVolume(sfxVolume);
        sfxSlider->setValue(sfxVolume);

        config.setValue("musicVolume", musicVolume);
        sound.setMusicVolume(musicVolume);
        musicSlider->setValue(musicVolume);

        // Graphics
        config.setValue("customcursor", customCursorEnabled ? 1 : 0);
        customCursorCheckBox->setMarked(customCursorEnabled);

        config.setValue("guialpha", opacity);
        alphaSlider->setValue(opacity);

        config.setValue("opengl", openGLEnabled ? 1 : 0);
        openGLCheckBox->setMarked(openGLEnabled);
    }
    else if (eventId == "winsToDefault")
    {
        statusWindow->resetToDefaultSize();
        minimap->resetToDefaultSize();
        chatWindow->resetToDefaultSize();
        inventoryWindow->resetToDefaultSize();
        equipmentWindow->resetToDefaultSize();
        helpWindow->resetToDefaultSize();
        skillDialog->resetToDefaultSize();
    }
}
