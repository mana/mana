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

#include "widgets/layouthelper.h"

#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"
#include "../main.h"
#include "../particle.h"

#include "../utils/gettext.h"
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
    mFps((int) config.getValue("fpslimit", 0)),
    mModeListModel(new ModeListModel),
    mModeList(new ListBox(mModeListModel)),
    mFsCheckBox(new CheckBox(_("Full screen"), mFullScreenEnabled)),
    mOpenGLCheckBox(new CheckBox(_("OpenGL"), mOpenGLEnabled)),
    mCustomCursorCheckBox(new CheckBox(_("Custom cursor"), mCustomCursorEnabled)),
    mAlphaSlider(new Slider(0.2, 1.0)),
    mFpsCheckBox(new CheckBox(_("FPS Limit:"))),
    mFpsSlider(new Slider(10, 200)),
    mFpsField(new TextField),
    mOriginalScrollLaziness((int) config.getValue("ScrollLaziness", 16)),
    mScrollLazinessSlider(new Slider(1, 64)),
    mScrollLazinessField(new TextField),
    mOriginalScrollRadius((int) config.getValue("ScrollRadius", 0)),
    mScrollRadiusSlider(new Slider(0, 128)),
    mScrollRadiusField(new TextField),
    mOverlayDetail((int) config.getValue("OverlayDetail", 2)),
    mOverlayDetailSlider(new Slider(0, 2)),
    mOverlayDetailField(new gcn::Label("")),
    mParticleDetail(3 - (int) config.getValue("particleEmitterSkip", 1)),
    mParticleDetailSlider(new Slider(0, 3)),
    mParticleDetailField(new gcn::Label(""))
{
    setOpaque(false);

    ScrollArea *scrollArea = new ScrollArea(mModeList);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    gcn::Label *alphaLabel = new gcn::Label(_("Gui opacity"));

    gcn::Label *scrollRadiusLabel = new gcn::Label(_("Scroll radius"));
    gcn::Label *scrollLazinessLabel = new gcn::Label(_("Scroll laziness"));
    gcn::Label *overlayDetailLabel = new gcn::Label(_("Ambient FX"));
    gcn::Label *particleDetailLabel = new gcn::Label(_("Particle Detail"));

    mModeList->setEnabled(false);
#ifndef USE_OPENGL
    mOpenGLCheckBox->setEnabled(false);
#endif

    mModeList->setSelected(-1);
    mAlphaSlider->setValue(mOpacity);
    mAlphaSlider->setWidth(90);

    mFpsField->setText(toString(mFps));
    mFpsField->setEnabled(mFps > 0);
    mFpsField->setWidth(30);
    mFpsSlider->setValue(mFps);
    mFpsSlider->setEnabled(mFps > 0);
    mFpsCheckBox->setSelected(mFps > 0);

    mCustomCursorCheckBox->setActionEventId("customcursor");
    mAlphaSlider->setActionEventId("guialpha");
    mFpsCheckBox->setActionEventId("fpslimitcheckbox");
    mFpsSlider->setActionEventId("fpslimitslider");
    mScrollRadiusSlider->setActionEventId("scrollradiusslider");
    mScrollRadiusField->setActionEventId("scrollradiusfield");
    mScrollLazinessSlider->setActionEventId("scrolllazinessslider");
    mScrollLazinessField->setActionEventId("scrolllazinessfield");
    mOverlayDetailSlider->setActionEventId("overlaydetailslider");
    mOverlayDetailField->setActionEventId("overlaydetailfield");
    mParticleDetailSlider->setActionEventId("particledetailslider");
    mParticleDetailField->setActionEventId("particledetailfield");

    mCustomCursorCheckBox->addActionListener(this);
    mAlphaSlider->addActionListener(this);
    mFpsCheckBox->addActionListener(this);
    mFpsSlider->addActionListener(this);
    mFpsField->addKeyListener(this);
    mScrollRadiusSlider->addActionListener(this);
    mScrollRadiusField->addKeyListener(this);
    mScrollLazinessSlider->addActionListener(this);
    mScrollLazinessField->addKeyListener(this);
    mOverlayDetailSlider->addActionListener(this);
    mOverlayDetailField->addKeyListener(this);
    mParticleDetailSlider->addActionListener(this);
    mParticleDetailField->addKeyListener(this);

    mScrollRadiusField->setText(toString(mOriginalScrollRadius));
    mScrollRadiusSlider->setValue(mOriginalScrollRadius);

    mScrollLazinessField->setText(toString(mOriginalScrollLaziness));
    mScrollLazinessSlider->setValue(mOriginalScrollLaziness);

    switch (mOverlayDetail)
    {
        case 0:
            mOverlayDetailField->setCaption(_("off"));
            break;
        case 1:
            mOverlayDetailField->setCaption(_("low"));
            break;
        case 2:
            mOverlayDetailField->setCaption(_("high"));
            break;
    }
    mOverlayDetailSlider->setValue(mOverlayDetail);

    switch (mParticleDetail)
    {
        case 0:
            mParticleDetailField->setCaption(_("low"));
            break;
        case 1:
            mParticleDetailField->setCaption(_("medium"));
            break;
        case 2:
            mParticleDetailField->setCaption(_("high"));
            break;
        case 3:
            mParticleDetailField->setCaption(_("max"));
            break;
    }
    mParticleDetailSlider->setValue(mParticleDetail);

    // Do the layout
    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, scrollArea, 1, 4).setPadding(2);
    place(1, 0, mFsCheckBox, 3);
    place(1, 1, mOpenGLCheckBox, 3);
    place(1, 2, mCustomCursorCheckBox, 3);

    place(0, 4, mAlphaSlider);
    place(0, 5, mFpsSlider);
    place(0, 6, mScrollRadiusSlider);
    place(0, 7, mScrollLazinessSlider);
    place(0, 8, mOverlayDetailSlider);
    place(0, 9, mParticleDetailSlider);

    place(1, 4, alphaLabel, 2);
    place(1, 5, mFpsCheckBox).setPadding(3);
    place(1, 6, scrollRadiusLabel);
    place(1, 7, scrollLazinessLabel);
    place(1, 8, overlayDetailLabel);
    place(1, 9, particleDetailLabel);

    place(2, 5, mFpsField).setPadding(1);
    place(2, 6, mScrollRadiusField).setPadding(1);
    place(2, 7, mScrollLazinessField).setPadding(1);
    place(2, 8, mOverlayDetailField, 2).setPadding(2);
    place(2, 9, mParticleDetailField, 2).setPadding(2);

    setDimension(gcn::Rectangle(0, 0, 295, 250));
}

Setup_Video::~Setup_Video()
{
    delete mModeListModel;
}

void Setup_Video::apply()
{
    // Full screen changes
    bool fullscreen = mFsCheckBox->isSelected();
    if (fullscreen != (config.getValue("screen", 0) == 1))
    {
        /* The OpenGL test is only necessary on Windows, since switching
         * to/from full screen works fine on Linux. On Windows we'd have to
         * reinitialize the OpenGL state and reload all textures.
         *
         * See http://libsdl.org/cgi/docwiki.cgi/SDL_SetVideoMode
         */

#if defined(WIN32) || defined(__APPLE__)
        // checks for opengl usage
        if (!(config.getValue("opengl", 0) == 1))
        {
#endif
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
#if defined(WIN32) || defined(__APPLE__)
        } else {
            new OkDialog(_("Switching to full screen"),
                    _("Restart needed for changes to take effect."));
        }
#endif
        config.setValue("screen", fullscreen ? 1 : 0);
    }

    // OpenGL change
    if (mOpenGLCheckBox->isSelected() != mOpenGLEnabled)
    {
        config.setValue("opengl", mOpenGLCheckBox->isSelected() ? 1 : 0);

        // OpenGL can currently only be changed by restarting, notify user.
        new OkDialog(_("Changing OpenGL"),
                _("Applying change to OpenGL requires restart."));
    }

    // FPS change
    config.setValue("fpslimit", mFps);

    // We sync old and new values at apply time
    mFullScreenEnabled = config.getValue("screen", 0);
    mCustomCursorEnabled = config.getValue("customcursor", 1);
    mOpacity = config.getValue("guialpha", 0.8);
    mOverlayDetail = (int)config.getValue("OverlayDetail", 2);
    mOpenGLEnabled = config.getValue("opengl", 0);
}

int Setup_Video::updateSlider(gcn::Slider *slider, gcn::TextField *field,
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
    mFsCheckBox->setSelected(mFullScreenEnabled);
    mOpenGLCheckBox->setSelected(mOpenGLEnabled);
    mCustomCursorCheckBox->setSelected(mCustomCursorEnabled);
    mAlphaSlider->setValue(mOpacity);
    mOverlayDetailSlider->setValue(mOverlayDetail);
    mParticleDetailSlider->setValue(mParticleDetail);

    mScrollRadiusField->setText(toString(mOriginalScrollRadius));
    mScrollLazinessField->setText(toString(mOriginalScrollLaziness));
    updateSlider(mScrollRadiusSlider, mScrollRadiusField, "ScrollRadius");
    updateSlider(mScrollLazinessSlider, mScrollLazinessField, "ScrollLaziness");

    config.setValue("screen", mFullScreenEnabled ? 1 : 0);
    config.setValue("customcursor", mCustomCursorEnabled ? 1 : 0);
    config.setValue("guialpha", mOpacity);
    config.setValue("opengl", mOpenGLEnabled ? 1 : 0);
}

void Setup_Video::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "guialpha")
    {
        config.setValue("guialpha", mAlphaSlider->getValue());
    }
    else if (event.getId() == "customcursor")
    {
        config.setValue("customcursor",
                mCustomCursorCheckBox->isSelected() ? 1 : 0);
    }
    else if (event.getId() == "fpslimitslider")
    {
        mFps = (int) mFpsSlider->getValue();
        mFpsField->setText(toString(mFps));
    }
    else if (event.getId() == "scrollradiusslider")
    {
        int val = (int) mScrollRadiusSlider->getValue();
        mScrollRadiusField->setText(toString(val));
        config.setValue("ScrollRadius", val);
    }
    else if (event.getId() == "scrolllazinessslider")
    {
        int val = (int) mScrollLazinessSlider->getValue();
        mScrollLazinessField->setText(toString(val));
        config.setValue("ScrollLaziness", val);
    }
    else if (event.getId() == "overlaydetailslider")
    {
        int val = (int) mOverlayDetailSlider->getValue();
        switch (val)
        {
            case 0:
                mOverlayDetailField->setCaption(_("off"));
                break;
            case 1:
                mOverlayDetailField->setCaption(_("low"));
                break;
            case 2:
                mOverlayDetailField->setCaption(_("high"));
                break;
        }
        config.setValue("OverlayDetail", val);
    }
    else if (event.getId() == "particledetailslider")
    {
        int val = (int) mParticleDetailSlider->getValue();
        switch (val)
        {
            case 0:
                mParticleDetailField->setCaption(_("low"));
                break;
            case 1:
                mParticleDetailField->setCaption(_("medium"));
                break;
            case 2:
                mParticleDetailField->setCaption(_("high"));
                break;
            case 3:
                mParticleDetailField->setCaption(_("max"));
                break;
        }
        config.setValue("particleEmitterSkip", 3 - val);
        Particle::emitterSkip = 4 - val;
    }
    else if (event.getId() == "fpslimitcheckbox")
    {
        if (mFpsCheckBox->isSelected())
        {
            mFps = (int) mFpsSlider->getValue();
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

void Setup_Video::keyPressed(gcn::KeyEvent &event)
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
