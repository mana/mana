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

#include "gui/setup_video.h"

#include "gui/okdialog.h"

#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/slider.h"
#include "gui/widgets/textfield.h"
#include "gui/widgets/dropdown.h"

#include "configuration.h"
#include "engine.h"
#include "graphics.h"
#include "localplayer.h"
#include "log.h"
#include "main.h"
#include "particle.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/key.hpp>
#include <guichan/listmodel.hpp>

#include <SDL.h>
#include <string>
#include <vector>

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
    if (modes == (SDL_Rect **)0)
        logger->log("No modes available");
    else if (modes == (SDL_Rect **)-1)
        logger->log("All resolutions available");
    else
    {
        //logger->log("Available Modes");
        for (int i = 0; modes[i]; ++i)
        {
            const std::string modeString =
                toString((int)modes[i]->w) + "x" + toString((int) modes[i]->h);
            //logger->log(modeString.c_str());
            mVideoModes.push_back(modeString);
        }
    }
}

const char *SIZE_NAME[4] =
{
    N_("Tiny"),
    N_("Small"),
    N_("Medium"),
    N_("Large"),
};

class FontSizeChoiceListModel : public gcn::ListModel
{
public:
    virtual ~FontSizeChoiceListModel() { }

    virtual int getNumberOfElements()
    {
        return 4;
    }

    virtual std::string getElementAt(int i)
    {
        if (i >= getNumberOfElements())
            return _("???");

        return SIZE_NAME[i];
    }
};

static const char *speechModeToString(Being::Speech mode)
{
    switch (mode)
    {
        case Being::NO_SPEECH:         return _("No text");
        case Being::TEXT_OVERHEAD:     return _("Text");
        case Being::NO_NAME_IN_BUBBLE: return _("Bubbles, no names");
        case Being::NAME_IN_BUBBLE:    return _("Bubbles with names");
    }
    return "";
}

static const char *overlayDetailToString(int detail)
{
    switch (detail)
    {
        case 0: return _("off");
        case 1: return _("low");
        case 2: return _("high");
    }
    return "";
}

static const char *particleDetailToString(int detail)
{
    switch (detail)
    {
        case 0: return _("low");
        case 1: return _("medium");
        case 2: return _("high");
        case 3: return _("max");
    }
    return "";
}

Setup_Video::Setup_Video():
    mFullScreenEnabled(config.getValue("screen", false)),
    mOpenGLEnabled(config.getValue("opengl", false)),
    mCustomCursorEnabled(config.getValue("customcursor", true)),
    mVisibleNamesEnabled(config.getValue("visiblenames", true)),
    mParticleEffectsEnabled(config.getValue("particleeffects", true)),
    mNameEnabled(config.getValue("showownname", false)),
    mPickupChatEnabled(config.getValue("showpickupchat", true)),
    mPickupParticleEnabled(config.getValue("showpickupparticle", false)),
    mOpacity(config.getValue("guialpha", 0.8)),
    mFps((int) config.getValue("fpslimit", 60)),
    mSpeechMode(static_cast<Being::Speech>(
            config.getValue("speech", Being::TEXT_OVERHEAD))),
    mModeListModel(new ModeListModel),
    mModeList(new ListBox(mModeListModel)),
    mFsCheckBox(new CheckBox(_("Full screen"), mFullScreenEnabled)),
    mOpenGLCheckBox(new CheckBox(_("OpenGL"), mOpenGLEnabled)),
    mCustomCursorCheckBox(new CheckBox(_("Custom cursor"),
                                       mCustomCursorEnabled)),
    mVisibleNamesCheckBox(new CheckBox(_("Visible names"),
                                       mVisibleNamesEnabled)),
    mParticleEffectsCheckBox(new CheckBox(_("Particle effects"),
                                          mParticleEffectsEnabled)),
    mNameCheckBox(new CheckBox(_("Show name"), mNameEnabled)),
    mPickupNotifyLabel(new Label(_("Show pickup notification"))),
    mPickupChatCheckBox(new CheckBox(_("in chat"), mPickupChatEnabled)),
    mPickupParticleCheckBox(new CheckBox(_("as particle"),
                                         mPickupParticleEnabled)),
    mSpeechSlider(new Slider(0, 3)),
    mSpeechLabel(new Label("")),
    mAlphaSlider(new Slider(0.2, 1.0)),
    mFpsCheckBox(new CheckBox(_("FPS Limit:"))),
    mFpsSlider(new Slider(10, 120)),
    mFpsField(new TextField),
    mOverlayDetail((int) config.getValue("OverlayDetail", 2)),
    mOverlayDetailSlider(new Slider(0, 2)),
    mOverlayDetailField(new Label("")),
    mParticleDetail(3 - (int) config.getValue("particleEmitterSkip", 1)),
    mParticleDetailSlider(new Slider(0, 3)),
    mParticleDetailField(new Label("")),
    mFontSize((int) config.getValue("fontSize", 11))
{
    setName(_("Video"));

    ScrollArea *scrollArea = new ScrollArea(mModeList);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    speechLabel = new Label(_("Overhead text"));
    alphaLabel = new Label(_("Gui opacity"));
    overlayDetailLabel = new Label(_("Ambient FX"));
    particleDetailLabel = new Label(_("Particle Detail"));
    fontSizeLabel = new Label(_("Font size"));

    mFontSizeDropDown = new DropDown(new FontSizeChoiceListModel);

    mModeList->setEnabled(true);

#ifndef USE_OPENGL
    mOpenGLCheckBox->setEnabled(false);
#endif

    mAlphaSlider->setValue(mOpacity);
    mAlphaSlider->setWidth(90);

    mFpsField->setText(toString(mFps));
    mFpsField->setEnabled(mFps > 0);
    mFpsField->setWidth(30);
    mFpsSlider->setValue(mFps);
    mFpsSlider->setEnabled(mFps > 0);
    mFpsCheckBox->setSelected(mFps > 0);

    mModeList->setActionEventId("videomode");
    mCustomCursorCheckBox->setActionEventId("customcursor");
    mVisibleNamesCheckBox->setActionEventId("visiblenames");
    mParticleEffectsCheckBox->setActionEventId("particleeffects");
    mPickupChatCheckBox->setActionEventId("pickupchat");
    mPickupParticleCheckBox->setActionEventId("pickupparticle");
    mNameCheckBox->setActionEventId("showownname");
    mAlphaSlider->setActionEventId("guialpha");
    mFpsCheckBox->setActionEventId("fpslimitcheckbox");
    mSpeechSlider->setActionEventId("speech");
    mFpsSlider->setActionEventId("fpslimitslider");
    mOverlayDetailSlider->setActionEventId("overlaydetailslider");
    mOverlayDetailField->setActionEventId("overlaydetailfield");
    mParticleDetailSlider->setActionEventId("particledetailslider");
    mParticleDetailField->setActionEventId("particledetailfield");

    mModeList->addActionListener(this);
    mCustomCursorCheckBox->addActionListener(this);
    mVisibleNamesCheckBox->addActionListener(this);
    mParticleEffectsCheckBox->addActionListener(this);
    mPickupChatCheckBox->addActionListener(this);
    mPickupParticleCheckBox->addActionListener(this);
    mNameCheckBox->addActionListener(this);
    mAlphaSlider->addActionListener(this);
    mFpsCheckBox->addActionListener(this);
    mSpeechSlider->addActionListener(this);
    mFpsSlider->addActionListener(this);
    mOverlayDetailSlider->addActionListener(this);
    mOverlayDetailField->addKeyListener(this);
    mParticleDetailSlider->addActionListener(this);
    mParticleDetailField->addKeyListener(this);

    mSpeechLabel->setCaption(speechModeToString(mSpeechMode));
    mSpeechSlider->setValue(mSpeechMode);

    mOverlayDetailField->setCaption(overlayDetailToString(mOverlayDetail));
    mOverlayDetailSlider->setValue(mOverlayDetail);

    mParticleDetailField->setCaption(particleDetailToString(mParticleDetail));
    mParticleDetailSlider->setValue(mParticleDetail);

    mFontSizeDropDown->setSelected(mFontSize - 10);
    mFontSizeDropDown->adjustHeight();

    // Do the layout
    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, scrollArea, 1, 5).setPadding(2);
    place(1, 0, mFsCheckBox, 2);
    place(3, 0, mOpenGLCheckBox, 1);

    place(1, 1, mCustomCursorCheckBox, 3);

    place(1, 2, mVisibleNamesCheckBox, 3);
    place(3, 2, mNameCheckBox, 1);

    place(1, 3, mParticleEffectsCheckBox, 3);

    place(1, 4, mPickupNotifyLabel, 4);

    place(1, 5, mPickupChatCheckBox, 1);
    place(2, 5, mPickupParticleCheckBox, 2);

    place(0, 6, fontSizeLabel, 3);
    place(1, 6, mFontSizeDropDown, 2);

    place(0, 7, mAlphaSlider);
    place(1, 7, alphaLabel, 3);

    place(0, 8, mFpsSlider);
    place(1, 8, mFpsCheckBox).setPadding(3);
    place(2, 8, mFpsField).setPadding(1);

    place(0, 9, mSpeechSlider);
    place(1, 9, speechLabel);
    place(2, 9, mSpeechLabel, 3).setPadding(2);

    place(0, 10, mOverlayDetailSlider);
    place(1, 10, overlayDetailLabel);
    place(2, 10, mOverlayDetailField, 3).setPadding(2);

    place(0, 11, mParticleDetailSlider);
    place(1, 11, particleDetailLabel);
    place(2, 11, mParticleDetailField, 3).setPadding(2);

    setDimension(gcn::Rectangle(0, 0, 365, 300));
}

void Setup_Video::apply()
{
    // Full screen changes
    bool fullscreen = mFsCheckBox->isSelected();
    if (fullscreen != (config.getValue("screen", false) == 1))
    {
        /* The OpenGL test is only necessary on Windows, since switching
         * to/from full screen works fine on Linux. On Windows we'd have to
         * reinitialize the OpenGL state and reload all textures.
         *
         * See http://libsdl.org/cgi/docwiki.cgi/SDL_SetVideoMode
         */

#if defined(WIN32) || defined(__APPLE__)
        // checks for opengl usage
        if (!(config.getValue("opengl", false) == 1))
        {
#endif
            if (!graphics->setFullscreen(fullscreen))
            {
                fullscreen = !fullscreen;
                if (!graphics->setFullscreen(fullscreen))
                {
                    std::stringstream error;
                    error << _("Failed to switch to ") <<
                        (fullscreen ? _("windowed") : _("fullscreen")) <<
                        _("mode and restoration of old mode also failed!") <<
                        std::endl;
                    logger->error(error.str());
                }
            }
#if defined(WIN32) || defined(__APPLE__)
        }
        else
        {
            new OkDialog(_("Switching to full screen"),
                         _("Restart needed for changes to take effect."));
        }
#endif
        config.setValue("screen", fullscreen);
    }

    // OpenGL change
    if (mOpenGLCheckBox->isSelected() != mOpenGLEnabled)
    {
        config.setValue("opengl", mOpenGLCheckBox->isSelected());

        // OpenGL can currently only be changed by restarting, notify user.
        new OkDialog(_("Changing OpenGL"),
                     _("Applying change to OpenGL requires restart."));
    }

    // FPS change
    config.setValue("fpslimit", mFps);

    config.setValue("fontSize", mFontSizeDropDown->getSelected() + 10);

    // We sync old and new values at apply time
    mFullScreenEnabled = config.getValue("screen", false);
    mCustomCursorEnabled = config.getValue("customcursor", true);
    mVisibleNamesEnabled = config.getValue("visiblenames", true);
    mParticleEffectsEnabled = config.getValue("particleeffects", true);
    mNameEnabled = config.getValue("showownname", false);
    mSpeechMode = static_cast<Being::Speech>(
            config.getValue("speech", Being::TEXT_OVERHEAD));
    mOpacity = config.getValue("guialpha", 0.8);
    mOverlayDetail = (int) config.getValue("OverlayDetail", 2);
    mOpenGLEnabled = config.getValue("opengl", false);
    mPickupChatEnabled = config.getValue("showpickupchat", true);
    mPickupParticleEnabled = config.getValue("showpickupparticle", false);
}

void Setup_Video::cancel()
{
    mFsCheckBox->setSelected(mFullScreenEnabled);
    mOpenGLCheckBox->setSelected(mOpenGLEnabled);
    mCustomCursorCheckBox->setSelected(mCustomCursorEnabled);
    mVisibleNamesCheckBox->setSelected(mVisibleNamesEnabled);
    mParticleEffectsCheckBox->setSelected(mParticleEffectsEnabled);
    mSpeechSlider->setValue(mSpeechMode);
    mNameCheckBox->setSelected(mNameEnabled);
    mAlphaSlider->setValue(mOpacity);
    mOverlayDetailSlider->setValue(mOverlayDetail);
    mParticleDetailSlider->setValue(mParticleDetail);

    config.setValue("screen", mFullScreenEnabled);
    config.setValue("customcursor", mCustomCursorEnabled);
    config.setValue("visiblenames", mVisibleNamesEnabled);
    config.setValue("particleeffects", mParticleEffectsEnabled);
    config.setValue("speech", mSpeechMode);
    config.setValue("showownname", mNameEnabled);
    if (player_node)
        player_node->mUpdateName = true;
    config.setValue("guialpha", mOpacity);
    config.setValue("opengl", mOpenGLEnabled);
    config.setValue("showpickupchat", mPickupChatEnabled);
    config.setValue("showpickupparticle", mPickupParticleEnabled);
}

void Setup_Video::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "videomode")
    {
        const std::string mode = mModeListModel->getElementAt(mModeList->getSelected());
        const int width = atoi(mode.substr(0, mode.find("x")).c_str());
        const int height = atoi(mode.substr(mode.find("x") + 1).c_str());

        if (width != graphics->getWidth() || height != graphics->getHeight())
        {
            // TODO: Find out why the drawing area doesn't resize without a restart.
            new OkDialog(_("Screen resolution changed"),
                         _("Restart your client for the change to take effect."));
        }

        config.setValue("screenwidth", width);
        config.setValue("screenheight", height);
    }
    else if (event.getId() == "guialpha")
    {
        config.setValue("guialpha", mAlphaSlider->getValue());
    }
    else if (event.getId() == "customcursor")
    {
        config.setValue("customcursor", mCustomCursorCheckBox->isSelected());
    }
    else if (event.getId() == "visiblenames")
    {
        config.setValue("visiblenames", mVisibleNamesCheckBox->isSelected());
    }
    else if (event.getId() == "particleeffects")
    {
        config.setValue("particleeffects",
                        mParticleEffectsCheckBox->isSelected());
        if (engine)
        {
            new OkDialog(_("Particle effect settings changed."),
                         _("Changes will take effect on map change."));
        }
    }
    else if (event.getId() == "pickupchat")
    {
        config.setValue("showpickupchat", mPickupChatCheckBox->isSelected());
    }
    else if (event.getId() == "pickupparticle")
    {
        config.setValue("showpickupparticle",
                        mPickupParticleCheckBox->isSelected());
    }
    else if (event.getId() == "speech")
    {
        Being::Speech val = (Being::Speech)mSpeechSlider->getValue();
        mSpeechLabel->setCaption(speechModeToString(val));
        mSpeechSlider->setValue(val);
        config.setValue("speech", val);
    }
    else if (event.getId() == "showownname")
    {
        // Notify the local player that settings have changed for the name
        // and requires an update
        if (player_node)
            player_node->mUpdateName = true;
        config.setValue("showownname", mNameCheckBox->isSelected());
    }
    else if (event.getId() == "fpslimitslider")
    {
        mFps = (int) mFpsSlider->getValue();
        mFpsField->setText(toString(mFps));
    }
    else if (event.getId() == "overlaydetailslider")
    {
        int val = (int) mOverlayDetailSlider->getValue();
        mOverlayDetailField->setCaption(overlayDetailToString(val));
        config.setValue("OverlayDetail", val);
    }
    else if (event.getId() == "particledetailslider")
    {
        int val = (int) mParticleDetailSlider->getValue();
        mParticleDetailField->setCaption(particleDetailToString(val));
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
        else if (mFps > 120)
        {
            mFps = 120;
        }
        mFpsField->setText(toString(mFps));
        mFpsSlider->setValue(mFps);
    }
    else
    {
        mFpsField->setText("");
        mFps = 0;
    }
}
