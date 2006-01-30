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

#include <sstream>

#include <guichan/widgets/container.hpp>
#include <guichan/widgets/label.hpp>

#include "button.h"
#include "chat.h"
#include "checkbox.h"
#include "equipmentwindow.h"
#include "help.h"
#include "inventorywindow.h"
#include "listbox.h"
#include "ok_dialog.h"
#include "minimap.h"
#include "scrollarea.h"
#include "skill.h"
#include "slider.h"
#include "status.h"
#include "tabbedcontainer.h"

#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"
#include "../main.h"
#include "../sound.h"

extern Graphics *graphics;

extern SDL_Joystick *joypad;

extern SDL_Joystick *joypad;

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
        //logger->log("Available Modes");
        for (int i = 0; modes[i]; ++i) {
            //logger->log("  %dx%d", modes[i]->w, modes[i]->h);
            std::stringstream mode;
            mode << (int)modes[i]->w << "x" << (int)modes[i]->h;
            videoModes.push_back(mode.str());
        }
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
    Window("Setup"), mCalibrating(false), leftTolerance(0), rightTolerance(0),
    upTolerance(0), downTolerance(0)
{
    modeListModel = new ModeListModel();
    modeList = new ListBox(modeListModel);
    modeList->setEnabled(false);
    scrollArea = new ScrollArea(modeList);
    fsCheckBox = new CheckBox("Full screen", false);
    openGLCheckBox = new CheckBox("OpenGL", false);
#ifndef USE_OPENGL
    openGLCheckBox->setEnabled(false);
#endif
    customCursorCheckBox = new CheckBox("Custom cursor");
    alphaLabel = new gcn::Label("Gui opacity");
    alphaSlider = new Slider(0.2, 1.0);
    soundCheckBox = new CheckBox("Sound", false);
    sfxSlider = new Slider(0, 128);
    musicSlider = new Slider(0, 128);
    sfxLabel = new gcn::Label("Sfx volume");
    musicLabel = new gcn::Label("Music volume");
    calibrateLabel = new gcn::Label("Press the button to start calibration");
    calibrateButton = new Button("Calibrate");
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
    calibrateButton->setEventId("calibrate");

    // Set dimensions/positions
    int width = 230;
    int height = 185;
    setContentSize(width, height);

    scrollArea->setDimension(gcn::Rectangle(10, 10, 90, 50));
    modeList->setDimension(gcn::Rectangle(0, 0, 60, 50));
    fsCheckBox->setPosition(110, 10);
    openGLCheckBox->setPosition(110, 30);
    customCursorCheckBox->setPosition(110, 50);
    alphaSlider->setDimension(gcn::Rectangle(10, 80, 100, 10));
    alphaLabel->setPosition(20 + alphaSlider->getWidth(), alphaSlider->getY());

    soundCheckBox->setPosition(10, 10);
    sfxSlider->setDimension(gcn::Rectangle(10, 30, 100, 10));
    musicSlider->setDimension(gcn::Rectangle(10, 50, 100, 10));
    sfxLabel->setPosition(20 + sfxSlider->getWidth(), 27);
    musicLabel->setPosition(20 + musicSlider->getWidth(), 47);
    
    calibrateLabel->setPosition(5, 10);
    calibrateButton->setPosition(10, 20 + calibrateLabel->getHeight());
    
    cancelButton->setPosition(
            width - cancelButton->getWidth() - 5,
            height - cancelButton->getHeight() - 5);
    applyButton->setPosition(
            cancelButton->getX() - applyButton->getWidth() - 5,
            cancelButton->getY());
    resetWinsToDefault->setPosition(
            applyButton->getX() - resetWinsToDefault->getWidth() - 5,
            applyButton->getY());
               
    // Listen for actions
    applyButton->addActionListener(this);
    cancelButton->addActionListener(this);
    resetWinsToDefault->addActionListener(this);
    alphaSlider->addActionListener(this);
    sfxSlider->addActionListener(this);
    musicSlider->addActionListener(this);
    customCursorCheckBox->addActionListener(this);
    calibrateButton->addActionListener(this);

    // Assemble dialog
    gcn::Container *video = new gcn::Container();
    video->setOpaque(false);
    video->add(scrollArea);
    video->add(fsCheckBox);
    video->add(openGLCheckBox);
    video->add(customCursorCheckBox);
    video->add(alphaSlider);
    video->add(alphaLabel);

    gcn::Container *audio = new gcn::Container();
    audio->setOpaque(false);
    audio->add(soundCheckBox);
    audio->add(sfxSlider);
    audio->add(musicSlider);
    audio->add(sfxLabel);
    audio->add(musicLabel);
    
    gcn::Container *input = new gcn::Container();
    input->setOpaque(false);
    input->add(calibrateLabel);
    input->add(calibrateButton);
    
    TabbedContainer *panel = new TabbedContainer();
    panel->setDimension(gcn::Rectangle(5, 5, 220, 130));
    panel->setOpaque(false);
    panel->addTab(video, "Video");
    panel->addTab(audio, "Audio");
    panel->addTab(input, "Input");
    add(panel);
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
    delete modeListModel;
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
    else if (eventId == "calibrate" && joypad != NULL)
    {
        if (mCalibrating)
        {
            calibrateButton->setCaption("Calibrate");
            calibrateLabel->setCaption("Press the button to start calibration");
            config.setValue("leftTolerance", leftTolerance);
            config.setValue("rightTolerance", rightTolerance);
            config.setValue("upTolerance", upTolerance);
            config.setValue("downTolerance", downTolerance);            
        }
        else
        {
            calibrateButton->setCaption("Stop");
            calibrateLabel->setCaption("Rotate the stick");          
            leftTolerance = 0;
            rightTolerance = 0;
            upTolerance = 0;
            downTolerance = 0;  
        }
        mCalibrating = !mCalibrating;
    }
    else if (eventId == "apply")
    {
        setVisible(false);

        // Full screen changes
        bool fullscreen = fsCheckBox->isMarked();
        if (fullscreen != (config.getValue("screen", 0) == 1)) 
        {
            // checks for opengl usage
            if (!(config.getValue("opengl", 0) == 1))
            {
                if (!graphics->setFullscreen(fullscreen))
                {
                    fullscreen = !fullscreen;
                    if (!graphics->setFullscreen(fullscreen))
                    {
                        std::stringstream error;
                        error << "Failed to switch to " <<
                        (fullscreen ? "windowed" : "fullscreen") <<
                        "mode and restoration of old mode also failed!" <<
                        std::endl;
                        logger->error(error.str());
                    }
                }
            } else {
            new OkDialog("Switching to FullScreen",
                    "Restart needed for changes to take effect.", NULL, this);
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
                new OkDialog("Sound Engine", err, NULL, this);
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
            new OkDialog("Changing OpenGL",
                    "Applying change to OpenGL requires restart.", NULL, this);
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

void Setup::logic()
{
    Window::logic();
    if (mCalibrating)
    {
        SDL_JoystickUpdate();
        int position = SDL_JoystickGetAxis(joypad, 0);
        if (position > rightTolerance)
        {
            rightTolerance = position;
        }
        else if (position < leftTolerance)
        {
            leftTolerance = position;
        }
        
        position = SDL_JoystickGetAxis(joypad, 1);
        if (position > downTolerance)
        {
            downTolerance = position;
        }
        else if (position < upTolerance)
        {
            upTolerance = position;
        }
    }
}
