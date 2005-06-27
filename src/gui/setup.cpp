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
#include "button.h"
#include "checkbox.h"
#include "scrollarea.h"
#include "listbox.h"
#include "radiobutton.h"
#include "slider.h"
#include "ok_dialog.h"
#include "../main.h"
#include <sstream>

#define SETUP_WIDTH 240

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
    openGlCheckBox = new CheckBox("OpenGL", false);
    openGlCheckBox->setEnabled(false);
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

    // Set events
    applyButton->setEventId("apply");
    cancelButton->setEventId("cancel");
    alphaSlider->setEventId("guialpha");
    sfxSlider->setEventId("sfx");
    musicSlider->setEventId("music");

    // Set dimensions/positions
    setContentSize(SETUP_WIDTH, 216);
    videoLabel->setPosition(SETUP_WIDTH - videoLabel->getWidth() - 5, 10);
    scrollArea->setDimension(gcn::Rectangle(10, 30, 90, 50));
    modeList->setDimension(gcn::Rectangle(0, 0, 60, 50));
    fsCheckBox->setPosition(110, 30);
    openGlCheckBox->setPosition(110, 50);
    alphaSlider->setDimension(gcn::Rectangle(10, 90, 100, 10));
    alphaLabel->setPosition(20 + alphaSlider->getWidth(), 87);
    audioLabel->setPosition(SETUP_WIDTH - videoLabel->getWidth() - 5, 110);
    soundCheckBox->setPosition(10, 130);
    sfxSlider->setDimension(gcn::Rectangle(10, 150, 100, 10));
    musicSlider->setDimension(gcn::Rectangle(10, 170, 100, 10));
    sfxLabel->setPosition(20 + sfxSlider->getWidth(), 147);
    musicLabel->setPosition(20 + musicSlider->getWidth(), 167);
    cancelButton->setPosition(
            SETUP_WIDTH - 5 - cancelButton->getWidth(),
            216 - 5 - cancelButton->getHeight());
    applyButton->setPosition(
            cancelButton->getX() - 5 - applyButton->getWidth(),
            216 - 5 - applyButton->getHeight());

    // Listen for actions
    applyButton->addActionListener(this);
    cancelButton->addActionListener(this);
    alphaSlider->addActionListener(this);
    sfxSlider->addActionListener(this);
    musicSlider->addActionListener(this);

    // Assemble dialog
    add(videoLabel);
    add(scrollArea);
    add(fsCheckBox);
    add(openGlCheckBox);
    add(audioLabel);
    add(soundCheckBox);
    add(applyButton);
    add(cancelButton);
    add(alphaSlider);
    add(alphaLabel);
    add(sfxSlider);
    add(musicSlider);
    add(sfxLabel);
    add(musicLabel);

    setLocationRelativeTo(getParent());

    // Load default settings
    modeList->setSelected(-1);
    if (config.getValue("screen", 0) == 1) {
        fsCheckBox->setMarked(true);
    }
    soundCheckBox->setMarked(config.getValue("sound", 0));
    alphaSlider->setValue(config.getValue("guialpha", 0.8));
    sfxSlider->setValue(config.getValue("sfxVolume", 100));
    musicSlider->setValue(config.getValue("musicVolume", 60));
}

Setup::~Setup()
{
    delete modeListModel;
    delete modeList;
    delete scrollArea;
    delete fsCheckBox;
    delete soundCheckBox;
    delete audioLabel;
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
    else if (eventId == "apply")
    {
        setVisible(false);
        bool changed = false;

        if (fsCheckBox->isMarked() && config.getValue("screen", 0) == 0) {
            // Fullscreen
            config.setValue("screen", 1);
            displayFlags |= SDL_FULLSCREEN;
            changed = true;
        }
        else if(!fsCheckBox->isMarked() && config.getValue("screen", 0) == 1) {
            // Windowed
            config.setValue("screen", 0);
            displayFlags &= ~SDL_FULLSCREEN;
            changed = true;
        }
        
        if(changed) {
            displayFlags |= SDL_DOUBLEBUF;
            if (useOpenGL) {
                displayFlags |= SDL_OPENGL;
                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            }
            
            screen = SDL_SetVideoMode(screenW, screenH, bitDepth, displayFlags);
            if (screen == NULL) {
                std::cerr << "Couldn't set " << screenW << "x" <<
                    screenH << "x" << bitDepth << " video mode: " <<
                    SDL_GetError() << std::endl;
            exit(1);
            }
        }

        // Sound settings
        if (soundCheckBox->isMarked()) {
            config.setValue("sound", 1);
            try {
                sound.init();
            }
            catch (const char *err) {
                new OkDialog(this, "Sound Engine", err);
                logger->log("Warning: %s", err);
            }
        } else {
            config.setValue("sound", 0);
            sound.close();
        }
    } else if (eventId == "cancel") {
        setVisible(false);
    }
}
