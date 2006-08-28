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

#include <guichan/key.hpp>
#include <guichan/listmodel.hpp>

#include <guichan/widgets/label.hpp>

#include "checkbox.h"
#include "listbox.h"
#include "ok_dialog.h"
#include "scrollarea.h"
#include "slider.h"
#include "textfield.h"

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
    mFullScreenEnabled(config.getValue("screen", 0)),
    mOpenGLEnabled(config.getValue("opengl", 0)),
    mCustomCursorEnabled(config.getValue("customcursor", 1)),
    mOpacity(config.getValue("guialpha", 0.8)),
    mFps((int)config.getValue("fpslimit", 50)),
    mModeListModel(new ModeListModel()),
    mModeList(new ListBox(mModeListModel)),
    mFsCheckBox(new CheckBox("Full screen", mFullScreenEnabled)),
    mOpenGLCheckBox(new CheckBox("OpenGL", mOpenGLEnabled)),
    mCustomCursorCheckBox(new CheckBox("Custom cursor", mCustomCursorEnabled)),
    mAlphaSlider(new Slider(0.2, 1.0)),
    mFpsCheckBox(new CheckBox("FPS Limit: ")),
    mFpsSlider(new Slider(10, 200)),
    mFpsField(new TextField()),
    mOriginalScrollLaziness((int) config.getValue("ScrollLaziness", 32)),
    mScrollLazinessSlider(new Slider(1, 64)),
    mScrollLazinessField(new TextField()),
    mOriginalScrollRadius((int) config.getValue("ScrollRadius", 32)),
    mScrollRadiusSlider(new Slider(0, 128)),
    mScrollRadiusField(new TextField())
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
    alphaLabel->setPosition(20 + mAlphaSlider->getWidth(),
                            mAlphaSlider->getY());
    mFpsCheckBox->setPosition(90, 100);
    mFpsSlider->setDimension(gcn::Rectangle(10, 100, 75, 10));
    mFpsField->setPosition(100 + mFpsCheckBox->getWidth(), 100);
    mFpsField->setWidth(30);

    mModeList->setSelected(-1);
    mAlphaSlider->setValue(mOpacity);

    mFpsField->setText(toString(mFps));
    mFpsField->setEnabled(mFps > 0);
    mFpsSlider->setValue(mFps);
    mFpsSlider->setEnabled(mFps > 0);
    mFpsCheckBox->setMarked(mFps > 0);

    mCustomCursorCheckBox->setEventId("customcursor");
    mAlphaSlider->setEventId("guialpha");
    mFpsCheckBox->setEventId("fpslimitcheckbox");
    mFpsSlider->setEventId("fpslimitslider");
    mScrollRadiusSlider->setEventId("scrollradiusslider");
    mScrollRadiusField->setEventId("scrollradiusfield");
    mScrollLazinessSlider->setEventId("scrolllazinessslider");
    mScrollLazinessField->setEventId("scrolllazinessfield");

    mCustomCursorCheckBox->addActionListener(this);
    mAlphaSlider->addActionListener(this);
    mFpsCheckBox->addActionListener(this);
    mFpsSlider->addActionListener(this);
    mFpsField->addKeyListener(this);
    mScrollRadiusSlider->addActionListener(this);
    mScrollRadiusField->addKeyListener(this);
    mScrollLazinessSlider->addActionListener(this);
    mScrollLazinessField->addKeyListener(this);

    mScrollRadiusSlider->setDimension(gcn::Rectangle(10, 120, 75, 10));
    gcn::Label *scrollRadiusLabel = new gcn::Label("Scroll radius");
    scrollRadiusLabel->setPosition(90, 120);
    mScrollRadiusField->setPosition(180, 120);
    mScrollRadiusField->setWidth(30);
    mScrollRadiusField->setText(toString(mOriginalScrollRadius));
    mScrollRadiusSlider->setValue(mOriginalScrollRadius);

    mScrollLazinessSlider->setDimension(gcn::Rectangle(10, 140, 75, 10));
    gcn::Label *scrollLazinessLabel = new gcn::Label("Scroll laziness");
    scrollLazinessLabel->setPosition(90, 140);
    mScrollLazinessField->setPosition(180, 140);
    mScrollLazinessField->setWidth(30);
    mScrollLazinessField->setText(toString(mOriginalScrollLaziness));
    mScrollLazinessSlider->setValue(mOriginalScrollLaziness);

    add(scrollArea);
    add(mFsCheckBox);
    add(mOpenGLCheckBox);
    add(mCustomCursorCheckBox);
    add(mAlphaSlider);
    add(alphaLabel);
    add(mFpsCheckBox);
    add(mFpsSlider);
    add(mFpsField);
    add(mScrollRadiusSlider);
    add(scrollRadiusLabel);
    add(mScrollRadiusField);
    add(mScrollLazinessSlider);
    add(scrollLazinessLabel);
    add(mScrollLazinessField);
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

    // FPS change
    config.setValue("fpslimit", mFps);

    // We sync old and new values at apply time
    mFullScreenEnabled = config.getValue("screen", 0);
    mCustomCursorEnabled = config.getValue("customcursor", 1);
    mOpacity = config.getValue("guialpha", 0.8);
    mOpenGLEnabled = config.getValue("opengl", 0);
}

int
Setup_Video::updateSlider(gcn::Slider *slider, gcn::TextField *field,
                          const std::string &configName)
{
    int value;
    std::stringstream temp(field->getText());
    temp >> value;
    if (value < slider->getScaleStart())
    {
        value = (int) slider->getScaleStart();
    }
    else if (value > slider->getScaleEnd())
    {
        value = (int) slider->getScaleEnd();
    }
    field->setText(toString(value));
    slider->setValue(value);
    config.setValue(configName, value);
    return value;
}

void Setup_Video::cancel()
{
    mFsCheckBox->setMarked(mFullScreenEnabled);
    mOpenGLCheckBox->setMarked(mOpenGLEnabled);
    mCustomCursorCheckBox->setMarked(mCustomCursorEnabled);
    mAlphaSlider->setValue(mOpacity);

    mScrollRadiusField->setText(toString(mOriginalScrollRadius));
    mScrollLazinessField->setText(toString(mOriginalScrollLaziness));
    updateSlider(mScrollRadiusSlider, mScrollRadiusField, "ScrollRadius");
    updateSlider(mScrollLazinessSlider, mScrollLazinessField, "ScrollLaziness");

    config.setValue("screen", mFullScreenEnabled ? 1 : 0);
    config.setValue("customcursor", mCustomCursorEnabled ? 1 : 0);
    config.setValue("guialpha", mOpacity);
    config.setValue("opengl", mOpenGLEnabled ? 1 : 0);
}

void Setup_Video::action(const std::string &event, gcn::Widget *widget)
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
    else if (event == "fpslimitslider")
    {
        mFps = (int)mFpsSlider->getValue();
        mFpsField->setText(toString(mFps));
    }
    else if (event == "scrollradiusslider")
    {
        int val = (int)mScrollRadiusSlider->getValue();
        mScrollRadiusField->setText(toString(val));
        config.setValue("ScrollRadius", val);
    }
    else if (event == "scrolllazinessslider")
    {
        int val = (int)mScrollLazinessSlider->getValue();
        mScrollLazinessField->setText(toString(val));
        config.setValue("ScrollLaziness", val);
    }
    else if (event == "fpslimitcheckbox")
    {
        if (mFpsCheckBox->isMarked())
        {
            mFps = (int)mFpsSlider->getValue();
        }
        else
        {
            mFps = 0;
        }
        mFpsField->setEnabled(mFps > 0);
        mFpsField->setText(toString(mFps));
        mFpsSlider->setValue(mFps);
        mFpsSlider->setEnabled(mFps > 0);
    }
}

void Setup_Video::keyPress(const gcn::Key &key)
{
    std::stringstream tempFps(mFpsField->getText());

    if (tempFps >> mFps)
    {
        if (mFps < 10)
        {
            mFps = 10;
        }
        else if (mFps > 200)
        {
            mFps = 200;
        }
        mFpsField->setText(toString(mFps));
        mFpsSlider->setValue(mFps);
    }
    else
    {
        mFpsField->setText("");
        mFps = 0;
    }
    updateSlider(mScrollRadiusSlider, mScrollRadiusField, "ScrollRadius");
    updateSlider(mScrollLazinessSlider, mScrollLazinessField, "ScrollLaziness");
}
