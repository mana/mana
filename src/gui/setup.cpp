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

/*
 *  This module takes care of everything relating to the
 *  setup dialog.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

SDL_Rect **modes;

ModeListModel::ModeListModel()
{
    int i;

    modes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_HWSURFACE);

    // Check if any modes available
    if (modes == (SDL_Rect**)0) {
        nmode = 0;
        logger.log("No modes");
    }

    // Check if modes restricted 
    if (modes == (SDL_Rect**)-1) {
        logger.log("Modes unrestricted");
    }

    for (nmode = 0; modes[nmode]; ++nmode);

    mode = (char**)calloc(nmode, sizeof(char*));

    for (i = 0; modes[i]; ++i) {
        char *temp = (char*)malloc(20 * sizeof(char));
        mode[i] = temp;
        if (sprintf(mode[i], "%d x %d", modes[i]->w, modes[i]->h) == -1) {
            logger.log("Cannot allocate mode list");
        }
    }
}

ModeListModel::~ModeListModel()
{
    int i;

    // Cleanup 
    for (i = 0; i < nmode; i++) {
        free(mode[i]);
    }

    free(mode);
}

int ModeListModel::getNumberOfElements()
{
    return nmode;
}

std::string ModeListModel::getElementAt(int i)
{
    return mode[i];
}

Setup::Setup():
    Window("Setup")
{
    modeListModel = new ModeListModel();
    displayLabel = new gcn::Label("Display settings");
    modeList = new ListBox(modeListModel);
    scrollArea = new ScrollArea(modeList);
    fsCheckBox = new CheckBox("Full screen", false);
    soundLabel = new gcn::Label("Sound settings");
    soundCheckBox = new CheckBox("Sound", false);
    disabledRadio = new RadioButton("Disabled", "Modes", false);
    applyButton = new Button("Apply");
    cancelButton = new Button("Cancel");
    alphaLabel = new gcn::Label("GUI opacity:");
    alphaSlider = new Slider(0.2, 1.0);

    // Set selections
    last_sel = 0;
    sel = 1;

    // Set events
    applyButton->setEventId("apply");
    cancelButton->setEventId("cancel");
    alphaSlider->setEventId("guialpha");

    // Set dimensions/positions
    setContentSize(240, 216);
    scrollArea->setDimension(gcn::Rectangle(10, 40, 90, 50));
    modeList->setDimension(gcn::Rectangle(0, 0, 60, 50));
    displayLabel->setDimension(gcn::Rectangle(10, 10, 100,16));
    cancelButton->setPosition(
            240 - 5 - cancelButton->getWidth(),
            216 - 5 - cancelButton->getHeight());
    applyButton->setPosition(
            cancelButton->getX() - 5 - applyButton->getWidth(),
            216 - 5 - applyButton->getHeight());
    soundLabel->setPosition(10, 110);
    fsCheckBox->setPosition(120, 36);
    soundCheckBox->setPosition(10, 130);
    disabledRadio->setPosition(10, 140);
    alphaLabel->setPosition(10, 157);
    alphaSlider->setDimension(gcn::Rectangle(
                alphaLabel->getWidth() + 20, 160, 100, 10));

    // Listen for actions
    applyButton->addActionListener(this);
    cancelButton->addActionListener(this);
    alphaSlider->addActionListener(this);

    // Assemble dialog
    add(scrollArea);
    add(displayLabel);
    add(fsCheckBox);
    add(soundLabel);
    add(soundCheckBox);
    //add(disabledRadio);
    add(applyButton);
    add(cancelButton);
    add(alphaLabel);
    add(alphaSlider);

    setLocationRelativeTo(getParent());

    // load default settings
    modeList->setSelected(1);
    if (config.getValue("screen", 0) == 1) {
        fsCheckBox->setMarked(true);
    }
    soundCheckBox->setMarked(config.getValue("sound", 0));
    alphaSlider->setValue(config.getValue("guialpha", 0.8));
}

Setup::~Setup()
{
    delete modeListModel;
    delete modeList;
    delete scrollArea;
    delete fsCheckBox;
    delete soundCheckBox;
    delete soundLabel;
    delete displayLabel;
    delete applyButton;
    delete cancelButton;
    delete alphaLabel;
    delete alphaSlider;
}

void Setup::action(const std::string &eventId)
{
    if (eventId == "guialpha")
    {
        config.setValue("guialpha", alphaSlider->getValue());
    }
    else if (eventId == "apply")
    {
        setVisible(false);

        // Select video mode
        sel = modeList->getSelected();

        if (sel != last_sel) {
            last_sel = sel;
            screen = SDL_SetVideoMode(modes[sel]->w, modes[sel]->h, 32,
                    SDL_FULLSCREEN | SDL_HWSURFACE);
        }

        // Display settings
        if (fsCheckBox->isMarked() && config.getValue("screen", 0) == 0)
        {
            config.setValue("screen", 1);
#if __USE_UNIX98
            SDL_WM_ToggleFullScreen(screen);
#else
            int displayFlags = 0;
            displayFlags |= SDL_FULLSCREEN;
            if ((int)config.getValue("hwaccel", 0)) {
                displayFlags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
            }
            else {
                displayFlags |= SDL_SWSURFACE;
            }
            screen = SDL_SetVideoMode(modes[sel]->w, modes[sel]->h, 32,
                    displayFlags);
#endif

        }
        else if (!fsCheckBox->isMarked() && config.getValue("screen", 0) == 1)
        {
            config.setValue("screen", 0);
#if __USE_UNIX98
            SDL_WM_ToggleFullScreen(screen);
#else
            int displayFlags = 0;
            if ((int)config.getValue("hwaccel", 0)) {
                displayFlags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
            }
            else {
                displayFlags |= SDL_SWSURFACE;
            }
            screen = SDL_SetVideoMode(modes[sel]->w, modes[sel]->h, 32,
                    displayFlags);
#endif
        }

        // Sound settings
        if (soundCheckBox->isMarked()) {
            config.setValue("sound", 1);
            try {
                sound.init(32, 20);
            }
            catch (const char *err) {
                new OkDialog(this, "Sound Engine", err);
                logger.log("Warning: %s", err);   
            }
        } else {
            config.setValue("sound", 0);
            sound.close();
        }
    } else if (eventId == "cancel") {
        setVisible(false);
    }
}
