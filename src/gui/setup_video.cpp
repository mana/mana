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

#include "setup_video.h"

#include <string>
#include <vector>
#include <SDL.h>

#include <guichan/listmodel.hpp>

#include <guichan/widgets/label.hpp>

#include "checkbox.h"
#include "listbox.h"
#include "ok_dialog.h"
#include "scrollarea.h"
#include "slider.h"

#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"

#include "../utils/tostring.h"

extern Graphics *graphics;

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
    /* Get available fullscreen/hardware modes */
    SDL_Rect **modes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_HWSURFACE);

    /* Check which modes are available */
    if (modes == (SDL_Rect **)0) {
        logger->log("No modes available");
    } else if (modes == (SDL_Rect **)-1) {
        logger->log("All resolutions available");
    } else {
        //logger->log("Available Modes");
        for (int i = 0; modes[i]; ++i) {
            const std::string modeString =
                toString((int)modes[i]->w) + "x" + toString((int)modes[i]->h);
            //logger->log(modeString.c_str());
            mVideoModes.push_back(modeString);
        }
    }
}

Setup_Video::Setup_Video():
    mModeListModel(new ModeListModel()),
    mModeList(new ListBox(mModeListModel)),
    mFsCheckBox(new CheckBox("Full screen", false)),
    mOpenGLCheckBox(new CheckBox("OpenGL", false)),
    mCustomCursorCheckBox(new CheckBox("Custom cursor")),
    mAlphaSlider(new Slider(0.2, 1.0)),
    mFullScreenEnabled(config.getValue("screen", 0)),
    mOpenGLEnabled(config.getValue("opengl", 0)),
    mCustomCursorEnabled(config.getValue("customcursor", 1)),
    mOpacity(config.getValue("guialpha", 0.8))
{
    setOpaque(false);

    ScrollArea *scrollArea = new ScrollArea(mModeList);
    gcn::Label *alphaLabel = new gcn::Label("Gui opacity");

    mModeList->setEnabled(false);
#ifndef USE_OPENGL
    mOpenGLCheckBox->setEnabled(false);
#endif

    mModeList->setDimension(gcn::Rectangle(0, 0, 60, 50));
    scrollArea->setDimension(gcn::Rectangle(10, 10, 90, 50));
    mFsCheckBox->setPosition(110, 10);
    mOpenGLCheckBox->setPosition(110, 30);
    mCustomCursorCheckBox->setPosition(110, 50);
    mAlphaSlider->setDimension(gcn::Rectangle(10, 80, 100, 10));
    alphaLabel->setPosition(20 + mAlphaSlider->getWidth(), mAlphaSlider->getY());

    mModeList->setSelected(-1);
    mFsCheckBox->setMarked(mFullScreenEnabled);
    mOpenGLCheckBox->setMarked(mOpenGLEnabled);
    mCustomCursorCheckBox->setMarked(mCustomCursorEnabled);
    mAlphaSlider->setValue(mOpacity);

    mCustomCursorCheckBox->setEventId("customcursor");
    mAlphaSlider->setEventId("guialpha");

    mCustomCursorCheckBox->addActionListener(this);
    mAlphaSlider->addActionListener(this);

    add(scrollArea);
    add(mFsCheckBox);
    add(mOpenGLCheckBox);
    add(mCustomCursorCheckBox);
    add(mAlphaSlider);
    add(alphaLabel);
}

Setup_Video::~Setup_Video()
{
    delete mModeListModel;
}

void Setup_Video::apply()
{
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
                    "Restart needed for changes to take effect.");
        }
        config.setValue("screen", fullscreen ? 1 : 0);
    }

    // OpenGL change
    if (mOpenGLCheckBox->isMarked() != mOpenGLEnabled)
    {
        config.setValue("opengl", mOpenGLCheckBox->isMarked() ? 1 : 0);

        // OpenGL can currently only be changed by restarting, notify user.
        new OkDialog("Changing OpenGL",
                "Applying change to OpenGL requires restart.");
    }

    // We sync old and new values at apply time
    mFullScreenEnabled = config.getValue("screen", 0);
    mCustomCursorEnabled = config.getValue("customcursor", 1);
    mOpacity = config.getValue("guialpha", 0.8);
    mOpenGLEnabled = config.getValue("opengl", 0);
}

void Setup_Video::cancel()
{
    mFsCheckBox->setMarked(mFullScreenEnabled);
    mOpenGLCheckBox->setMarked(mOpenGLEnabled);
    mCustomCursorCheckBox->setMarked(mCustomCursorEnabled);
    mAlphaSlider->setValue(mOpacity);

    config.setValue("screen", mFullScreenEnabled ? 1 : 0);
    config.setValue("customcursor", mCustomCursorEnabled ? 1 : 0);
    config.setValue("guialpha", mOpacity);
    config.setValue("opengl", mOpenGLEnabled ? 1 : 0);
}

void Setup_Video::action(const std::string &event)
{
    if (event == "guialpha")
    {
        config.setValue("guialpha", mAlphaSlider->getValue());
    }
    else if (event == "customcursor")
    {
        config.setValue("customcursor",
                mCustomCursorCheckBox->isMarked() ? 1 : 0);
    }
}
