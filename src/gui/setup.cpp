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
#include "checkbox.h"
#include "listbox.h"
#include "ok_dialog.h"
#include "scrollarea.h"
#include "slider.h"
#include "tabbedcontainer.h"

#include "../configuration.h"
#include "../graphics.h"
#include "../joystick.h"
#include "../log.h"
#include "../main.h"
#include "../sound.h"

extern Graphics *graphics;

extern Joystick *joystick;

extern Window *statusWindow;
extern Window *minimap;
extern Window *chatWindow;
extern Window *inventoryWindow;
extern Window *equipmentWindow;
extern Window *helpWindow;
extern Window *skillDialog;

/**
 * The list model for mode list.
 *
 * \ingroup Interface
 */
class ModeListModel : public gcn::ListModel
{
    public:
        /**
         * Constructor.
         */
        ModeListModel();

        /**
         * Destructor.
         */
        virtual ~ModeListModel() { }

        /**
         * Returns the number of elements in container.
         */
        int getNumberOfElements() { return mVideoModes.size(); }

        /**
         * Returns element from container.
         */
        std::string getElementAt(int i) { return mVideoModes[i]; }

    private:
        std::vector<std::string> mVideoModes;
};

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
            mVideoModes.push_back(mode.str());
        }
    }
}

Setup::Setup():
    Window("Setup")
{
    mModeListModel = new ModeListModel();
    mModeList = new ListBox(mModeListModel);
    mModeList->setEnabled(false);
    ScrollArea *scrollArea = new ScrollArea(mModeList);
    mFsCheckBox = new CheckBox("Full screen", false);
    mOpenGLCheckBox = new CheckBox("OpenGL", false);
#ifndef USE_OPENGL
    mOpenGLCheckBox->setEnabled(false);
#endif
    mCustomCursorCheckBox = new CheckBox("Custom cursor");
    gcn::Label *alphaLabel = new gcn::Label("Gui opacity");
    mAlphaSlider = new Slider(0.2, 1.0);
    mSoundCheckBox = new CheckBox("Sound", false);
    mSfxSlider = new Slider(0, 128);
    mMusicSlider = new Slider(0, 128);
    gcn::Label *sfxLabel = new gcn::Label("Sfx volume");
    gcn::Label *musicLabel = new gcn::Label("Music volume");
    mCalibrateLabel = new gcn::Label("Press the button to start calibration");
    mCalibrateButton = new Button("Calibrate", "calibrate", this);
    Button *applyButton = new Button("Apply", "apply", this);
    Button *cancelButton = new Button("Cancel", "cancel", this);
    Button *resetWinsToDefault = new Button("Reset Windows", "winsToDefault", this);

    // Set events
    mAlphaSlider->setEventId("guialpha");
    mSfxSlider->setEventId("sfx");
    mMusicSlider->setEventId("music");
    mCustomCursorCheckBox->setEventId("customcursor");

    // Set dimensions/positions
    int width = 230;
    int height = 185;
    setContentSize(width, height);

    scrollArea->setDimension(gcn::Rectangle(10, 10, 90, 50));
    mModeList->setDimension(gcn::Rectangle(0, 0, 60, 50));
    mFsCheckBox->setPosition(110, 10);
    mOpenGLCheckBox->setPosition(110, 30);
    mCustomCursorCheckBox->setPosition(110, 50);
    mAlphaSlider->setDimension(gcn::Rectangle(10, 80, 100, 10));
    alphaLabel->setPosition(20 + mAlphaSlider->getWidth(), mAlphaSlider->getY());

    mSoundCheckBox->setPosition(10, 10);
    mSfxSlider->setDimension(gcn::Rectangle(10, 30, 100, 10));
    mMusicSlider->setDimension(gcn::Rectangle(10, 50, 100, 10));
    sfxLabel->setPosition(20 + mSfxSlider->getWidth(), 27);
    musicLabel->setPosition(20 + mMusicSlider->getWidth(), 47);
    
    mCalibrateLabel->setPosition(5, 10);
    mCalibrateButton->setPosition(10, 20 + mCalibrateLabel->getHeight());
    
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
    mAlphaSlider->addActionListener(this);
    mSfxSlider->addActionListener(this);
    mMusicSlider->addActionListener(this);
    mCustomCursorCheckBox->addActionListener(this);

    // Assemble dialog
    gcn::Container *video = new gcn::Container();
    video->setOpaque(false);
    video->add(scrollArea);
    video->add(mFsCheckBox);
    video->add(mOpenGLCheckBox);
    video->add(mCustomCursorCheckBox);
    video->add(mAlphaSlider);
    video->add(alphaLabel);

    gcn::Container *audio = new gcn::Container();
    audio->setOpaque(false);
    audio->add(mSoundCheckBox);
    audio->add(mSfxSlider);
    audio->add(mMusicSlider);
    audio->add(sfxLabel);
    audio->add(musicLabel);
    
    gcn::Container *input = new gcn::Container();
    input->setOpaque(false);
    input->add(mCalibrateLabel);
    input->add(mCalibrateButton);
    
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
    mModeList->setSelected(-1);

    // Full Screen
    mFullScreenEnabled = config.getValue("screen", 0);
    mFsCheckBox->setMarked(mFullScreenEnabled);

    // Sound
    mSoundEnabled = config.getValue("sound", 0);
    mSoundCheckBox->setMarked(mSoundEnabled);

    mSfxVolume = (int)config.getValue("sfxVolume", 100);
    mSfxSlider->setValue(mSfxVolume);

    mMusicVolume = (int)config.getValue("musicVolume", 60);
    mMusicSlider->setValue(mMusicVolume);

    // Graphics
    mCustomCursorEnabled = config.getValue("customcursor", 1);
    mCustomCursorCheckBox->setMarked(mCustomCursorEnabled);

    mOpacity = config.getValue("guialpha", 0.8);
    mAlphaSlider->setValue(mOpacity);

    mOpenGLEnabled = config.getValue("opengl", 0);
    mOpenGLCheckBox->setMarked(mOpenGLEnabled);
}

Setup::~Setup()
{
    delete mModeListModel;
}

void Setup::action(const std::string &eventId)
{
    if (eventId == "sfx")
    {
        config.setValue("sfxVolume", (int)mSfxSlider->getValue());
        sound.setSfxVolume((int)mSfxSlider->getValue());
    }
    else if (eventId == "music")
    {
        config.setValue("musicVolume", (int)mMusicSlider->getValue());
        sound.setMusicVolume((int)mMusicSlider->getValue());
    }
    else if (eventId == "guialpha")
    {
        config.setValue("guialpha", mAlphaSlider->getValue());
    }
    else if (eventId == "customcursor")
    {
        config.setValue("customcursor",
                mCustomCursorCheckBox->isMarked() ? 1 : 0);
    }
    else if (eventId == "calibrate" && joystick != NULL)
    {
        if (joystick->isCalibrating())
        {
            mCalibrateButton->setCaption("Calibrate");
            mCalibrateLabel->setCaption("Press the button to start calibration");
            joystick->finishCalibration();
        }
        else
        {
            mCalibrateButton->setCaption("Stop");
            mCalibrateLabel->setCaption("Rotate the stick");
            joystick->startCalibration();
        }
    }
    else if (eventId == "apply")
    {
        setVisible(false);

        // Full screen changes
        bool fullscreen = mFsCheckBox->isMarked();
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
                    "Restart needed for changes to take effect.", this);
            }
            config.setValue("screen", fullscreen ? 1 : 0);
        }

        // Sound settings changes
        if (mSoundCheckBox->isMarked())
        {
            config.setValue("sound", 1);
            try {
                sound.init();
            }
            catch (const char *err)
            {
                new OkDialog("Sound Engine", err, this);
                logger->log("Warning: %s", err);
            }
        }
        else
        {
            config.setValue("sound", 0);
            sound.close();
        }

        // OpenGL change
        if (mOpenGLCheckBox->isMarked() != mOpenGLEnabled)
        {
            config.setValue("opengl", mOpenGLCheckBox->isMarked() ? 1 : 0);

            // OpenGL can currently only be changed by restarting, notify user.
            new OkDialog("Changing OpenGL",
                    "Applying change to OpenGL requires restart.", this);
        }

        // We sync old and new values at apply time
        // Screen
        mFullScreenEnabled = config.getValue("screen", 0);

        // Sound
        mSoundEnabled = config.getValue("sound", 0);
        mSfxVolume = (int)config.getValue("sfxVolume", 100);
        mMusicVolume = (int)config.getValue("musicVolume", 60);

        // Graphics
        mCustomCursorEnabled = config.getValue("customcursor", 1);
        mOpacity = config.getValue("guialpha", 0.8);
        mOpenGLEnabled = config.getValue("opengl", 0);
    }
    else if (eventId == "cancel")
    {
        setVisible(false);

        // Restoring old values when cancelling
        // Screen
        config.setValue("screen", mFullScreenEnabled ? 1 : 0);
        mFsCheckBox->setMarked(mFullScreenEnabled);

        // Sound
        config.getValue("sound", mSoundEnabled ? 1 : 0);
        mSoundCheckBox->setMarked(mSoundEnabled);

        config.getValue("sfxVolume", mSfxVolume ? 1 : 0);
        sound.setSfxVolume(mSfxVolume);
        mSfxSlider->setValue(mSfxVolume);

        config.setValue("musicVolume", mMusicVolume);
        sound.setMusicVolume(mMusicVolume);
        mMusicSlider->setValue(mMusicVolume);

        // Graphics
        config.setValue("customcursor", mCustomCursorEnabled ? 1 : 0);
        mCustomCursorCheckBox->setMarked(mCustomCursorEnabled);

        config.setValue("guialpha", mOpacity);
        mAlphaSlider->setValue(mOpacity);

        config.setValue("opengl", mOpenGLEnabled ? 1 : 0);
        mOpenGLCheckBox->setMarked(mOpenGLEnabled);
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
