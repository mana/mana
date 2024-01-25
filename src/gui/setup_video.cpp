/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/setup_video.h"

#include "client.h"
#include "configuration.h"
#include "game.h"
#include "graphics.h"
#include "localplayer.h"
#include "particle.h"

#include "gui/okdialog.h"

#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/slider.h"
#include "gui/widgets/spacer.h"

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
        ModeListModel();

        virtual ~ModeListModel() { }

        /**
         * Returns the number of elements in container.
         */
        int getNumberOfElements() { return mVideoModes.size(); }

        /**
         * Returns element from container.
         */
        std::string getElementAt(int i) { return mVideoModes[i]; }

        /**
         * Returns the index corresponding to the given video mode.
         * E.g.: "800x600".
         * or -1 if not found.
         */
        int getIndexOf(const std::string &widthXHeightMode);

    private:
        std::vector<std::string> mVideoModes;
};

ModeListModel::ModeListModel()
{
    /* Get available fullscreen/hardware modes */
    const int numModes = SDL_GetNumDisplayModes(0);
    for (int i = 0; i < numModes; i++)
    {
        SDL_DisplayMode mode;
        if (SDL_GetDisplayMode(0, i, &mode) != 0)
            continue;

        // Skip the unreasonably small modes
        if (mode.w < 640 || mode.h < 480)
            continue;

        // TODO_SDL2: Modes now dinstinguish between pixel format and refresh rate as well
        // TODO_SDL2: Fullscreen mode needs display selection

        mVideoModes.push_back(toString(mode.w) + "x" + toString(mode.h));
    }
}

int ModeListModel::getIndexOf(const std::string &widthXHeightMode)
{
    for (unsigned i = 0; i < mVideoModes.size(); i++)
        if (mVideoModes.at(i) == widthXHeightMode)
            return i;

    return -1;
}

const char *Setup_Video::overlayDetailToString(int detail)
{
    if (detail == -1)
        detail = config.getIntValue("OverlayDetail");

    switch (detail)
    {
        case 0: return _("off");
        case 1: return _("low");
        case 2: return _("high");
    }
    return "";
}

const char *Setup_Video::particleDetailToString(int detail)
{
    if (detail == -1)
        detail = 3 - config.getIntValue("particleEmitterSkip");

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
    mFullScreenEnabled(config.getBoolValue("screen")),
    mOpenGLEnabled(config.getBoolValue("opengl")),
    mCustomCursorEnabled(config.getBoolValue("customcursor")),
    mParticleEffectsEnabled(config.getBoolValue("particleeffects")),
    mFps(config.getIntValue("fpslimit")),
    mSDLTransparencyDisabled(config.getBoolValue("disableTransparency")),
    mModeListModel(new ModeListModel),
    mModeList(new ListBox(mModeListModel)),
    mFsCheckBox(new CheckBox(_("Full screen"), mFullScreenEnabled)),
    mOpenGLCheckBox(new CheckBox(_("OpenGL"), mOpenGLEnabled)),
    mCustomCursorCheckBox(new CheckBox(_("Custom cursor"),
                                       mCustomCursorEnabled)),
    mParticleEffectsCheckBox(new CheckBox(_("Particle effects"),
                                          mParticleEffectsEnabled)),
    mFpsCheckBox(new CheckBox(_("FPS limit:"))),
    mFpsSlider(new Slider(10, 120)),
    mFpsLabel(new Label),
    mOverlayDetail(config.getIntValue("OverlayDetail")),
    mOverlayDetailSlider(new Slider(0, 2)),
    mOverlayDetailField(new Label),
    mParticleDetail(3 - config.getIntValue("particleEmitterSkip")),
    mParticleDetailSlider(new Slider(0, 3)),
    mParticleDetailField(new Label),
    mDisableSDLTransparencyCheckBox(
                          new CheckBox(_("Disable transparency (Low CPU mode)"),
                                       mSDLTransparencyDisabled))
{
    setName(_("Video"));

    Spacer *space = new Spacer(0,10);

    ScrollArea *scrollArea = new ScrollArea(mModeList);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setSize(100, 200);

    overlayDetailLabel = new Label(_("Ambient FX:"));
    particleDetailLabel = new Label(_("Particle detail:"));

    mModeList->setEnabled(true);

#ifndef USE_OPENGL
    mOpenGLCheckBox->setEnabled(false);
#endif

    mFpsLabel->setCaption(mFps > 0 ? toString(mFps) : _("None"));
    mFpsLabel->setWidth(60);
    mFpsSlider->setValue(mFps);
    mFpsSlider->setEnabled(mFps > 0);
    mFpsCheckBox->setSelected(mFps > 0);

    overlayDetailLabel->setAlignment(Graphics::RIGHT);
    particleDetailLabel->setAlignment(Graphics::RIGHT);

    // If the openGL Mode is enabled, disabling the transaprency
    // is irrelevant.
    mDisableSDLTransparencyCheckBox->setEnabled(!mOpenGLEnabled);

    // Pre-select the current video mode.
    std::string videoMode = toString(graphics->getWidth()) + "x"
                            + toString(graphics->getHeight());
    mModeList->setSelected(mModeListModel->getIndexOf(videoMode));

    // Set actions
    mModeList->setActionEventId("videomode");
    mCustomCursorCheckBox->setActionEventId("customcursor");
    mParticleEffectsCheckBox->setActionEventId("particleeffects");
    mDisableSDLTransparencyCheckBox->setActionEventId("disableTransparency");
    mFpsCheckBox->setActionEventId("fpslimitcheckbox");
    mFpsSlider->setActionEventId("fpslimitslider");
    mOverlayDetailSlider->setActionEventId("overlaydetailslider");
    mOverlayDetailField->setActionEventId("overlaydetailfield");
    mOpenGLCheckBox->setActionEventId("opengl");
    mParticleDetailSlider->setActionEventId("particledetailslider");
    mParticleDetailField->setActionEventId("particledetailfield");

    // Set listeners
    mModeList->addActionListener(this);
    mCustomCursorCheckBox->addActionListener(this);
    mOpenGLCheckBox->addActionListener(this);
    mParticleEffectsCheckBox->addActionListener(this);
    mDisableSDLTransparencyCheckBox->addActionListener(this);
    mFpsCheckBox->addActionListener(this);
    mFpsSlider->addActionListener(this);
    mOverlayDetailSlider->addActionListener(this);
    mOverlayDetailField->addKeyListener(this);
    mParticleDetailSlider->addActionListener(this);
    mParticleDetailField->addKeyListener(this);

    mOverlayDetailField->setCaption(overlayDetailToString(mOverlayDetail));
    mOverlayDetailSlider->setValue(mOverlayDetail);

    mParticleDetailField->setCaption(particleDetailToString(mParticleDetail));
    mParticleDetailSlider->setValue(mParticleDetail);

    // Do the layout
    ContainerPlacer place = getPlacer(0, 0);
    place.getCell().setHAlign(LayoutCell::FILL);

    place(0, 0, scrollArea, 1, 4).setPadding(2).setHAlign(LayoutCell::FILL);
    place(1, 0, space, 1, 4);
    place(2, 0, mFsCheckBox);
    place(2, 1, mOpenGLCheckBox);
    place(2, 2, mCustomCursorCheckBox);

    place = getPlacer(0, 1);
    place.getCell().setHAlign(LayoutCell::FILL);

    place(0, 0, space, 3);
    place(0, 1, mDisableSDLTransparencyCheckBox, 4);

    place(0, 2, mFpsCheckBox);
    place(1, 2, mFpsSlider, 2);
    place(3, 2, mFpsLabel);

    place(0, 3, mParticleEffectsCheckBox, 4);

    place(0, 4, particleDetailLabel);
    place(1, 4, mParticleDetailSlider, 2);
    place(3, 4, mParticleDetailField);

    place(0, 5, overlayDetailLabel);
    place(1, 5, mOverlayDetailSlider, 2);
    place(3, 5, mOverlayDetailField);
}

Setup_Video::~Setup_Video()
{
    delete mModeListModel;
    delete mModeList;
}

void Setup_Video::apply()
{
    // Video mode changes
    int screenWidth = graphics->getWidth();
    int screenHeight = graphics->getHeight();

    if (mModeList->getSelected() > -1)
    {
        std::string mode = mModeListModel->getElementAt(mModeList->getSelected());
        screenWidth = atoi(mode.substr(0, mode.find("x")).c_str());
        screenHeight = atoi(mode.substr(mode.find("x") + 1).c_str());
    }

    bool fullscreen = mFsCheckBox->isSelected();

    if (fullscreen != graphics->getFullscreen() ||
            screenWidth != graphics->getWidth() ||
            screenHeight != graphics->getHeight())
    {
        /* The OpenGL test is only necessary on Windows, since switching
         * to/from full screen works fine on Linux. On Windows we'd have to
         * reinitialize the OpenGL state and reload all textures.
         *
         * See http://libsdl.org/cgi/docwiki.cgi/SDL_SetVideoMode
         */

#if defined(_WIN32) || defined(__APPLE__)
        // checks for opengl usage
        if (config.getBoolValue("opengl"))
        {
            new OkDialog(_("Changing Video Mode"),
                         _("Restart needed for changes to take effect."));

            config.setValue("screen", fullscreen);
            config.setValue("screenwidth", screenWidth);
            config.setValue("screenheight", screenHeight);
        }
        else
#endif
        {
            if (!graphics->changeVideoMode(screenWidth,
                                           screenHeight,
                                           fullscreen))
            {
                std::stringstream errorMessage;
                if (fullscreen)
                    errorMessage << _("Failed to switch to fullscreen mode.");
                else
                    errorMessage << _("Failed to switch to windowed mode.");

                new OkDialog(_("Error"), errorMessage.str());
            }
            else
            {
                Client::instance()->videoResized(screenWidth, screenHeight);

                config.setValue("screen", fullscreen);
                config.setValue("screenwidth", screenWidth);
                config.setValue("screenheight", screenHeight);
            }
        }
    }

    // OpenGL change
    if (mOpenGLCheckBox->isSelected() != mOpenGLEnabled)
    {
        config.setValue("opengl", mOpenGLCheckBox->isSelected());

        // OpenGL can currently only be changed by restarting, notify user.
        if (mOpenGLCheckBox->isSelected())
        {
            new OkDialog(_("Changing to OpenGL"),
                         _("Applying change to OpenGL requires restart.\n\n"
                           "In case OpenGL messes up your game graphics, "
                           "restart the game with the command line option "
                           "\"--no-opengl\"."));
        }
        else
        {
            new OkDialog(_("Deactivating OpenGL"),
                         _("Applying change to OpenGL requires restart."));
        }
    }
    // If LowCPU is enabled from a disabled state we warn the user
    else if (mDisableSDLTransparencyCheckBox->isSelected())
    {
        if (config.getValue("disableTransparency", true) == false)
        {
            new OkDialog(_("Transparency disabled"),
                 _("You must restart to apply changes."));
        }
    }
    else
    {
        if (config.getValue("disableTransparency", true) == true)
        {
            new OkDialog(_("Transparency enabled"),
                 _("You must restart to apply changes."));
        }
    }
    config.setValue("disableTransparency",
                                 mDisableSDLTransparencyCheckBox->isSelected());

    mFps = mFpsCheckBox->isSelected() ? (int) mFpsSlider->getValue() : 0;
    mFpsSlider->setEnabled(mFps > 0);

    // FPS change
    config.setValue("fpslimit", mFps);

    // We sync old and new values at apply time
    mFullScreenEnabled = config.getBoolValue("screen");
    mCustomCursorEnabled = config.getBoolValue("customcursor");
    mParticleEffectsEnabled = config.getBoolValue("particleeffects");
    mOverlayDetail = config.getIntValue("OverlayDetail");
    mOpenGLEnabled = config.getBoolValue("opengl");
    mSDLTransparencyDisabled = config.getBoolValue("disableTransparency");
}

void Setup_Video::cancel()
{
    mFpsCheckBox->setSelected(mFps > 0);
    mFsCheckBox->setSelected(mFullScreenEnabled);
    mOpenGLCheckBox->setSelected(mOpenGLEnabled);
    mCustomCursorCheckBox->setSelected(mCustomCursorEnabled);
    mParticleEffectsCheckBox->setSelected(mParticleEffectsEnabled);
    mFpsSlider->setValue(mFps);
    mFpsSlider->setEnabled(mFps > 0);
    mOverlayDetailSlider->setValue(mOverlayDetail);
    mParticleDetailSlider->setValue(mParticleDetail);
    std::string text = mFpsCheckBox->isSelected() ? toString(mFps) : _("None");
    mFpsLabel->setCaption(text);
    mDisableSDLTransparencyCheckBox->setSelected(mSDLTransparencyDisabled);
    mDisableSDLTransparencyCheckBox->setEnabled(!mOpenGLEnabled);

    config.setValue("screen", mFullScreenEnabled);

    // Set back to the current video mode.
    std::string videoMode = toString(graphics->getWidth()) + "x"
                            + toString(graphics->getHeight());
    mModeList->setSelected(mModeListModel->getIndexOf(videoMode));

    config.setValue("customcursor", mCustomCursorEnabled);
    config.setValue("particleeffects", mParticleEffectsEnabled);
    config.setValue("opengl", mOpenGLEnabled);
    config.setValue("disableTransparency", mSDLTransparencyDisabled);
}

void Setup_Video::action(const gcn::ActionEvent &event)
{
    const std::string &id = event.getId();

    if (id == "customcursor")
    {
        config.setValue("customcursor", mCustomCursorCheckBox->isSelected());
    }
    else if (id == "particleeffects")
    {
        config.setValue("particleeffects",
                        mParticleEffectsCheckBox->isSelected());
        Particle::enabled = mParticleEffectsCheckBox->isSelected();

        if (Game::instance())
        {
            new OkDialog(_("Particle Effect Settings Changed."),
                         _("Changes will take effect on map change."));
        }
    }
    else if (id == "overlaydetailslider")
    {
        int val = (int) mOverlayDetailSlider->getValue();
        mOverlayDetailField->setCaption(overlayDetailToString(val));
        config.setValue("OverlayDetail", val);
    }
    else if (id == "particledetailslider")
    {
        int val = (int) mParticleDetailSlider->getValue();
        mParticleDetailField->setCaption(particleDetailToString(val));
        config.setValue("particleEmitterSkip", 3 - val);
        Particle::emitterSkip = 4 - val;
    }
    else if (id == "fpslimitcheckbox" || id == "fpslimitslider")
    {
        int fps = (int) mFpsSlider->getValue();
        fps = fps > 0 ? fps : mFpsSlider->getScaleStart();
        mFps = mFpsCheckBox->isSelected() ? fps : 0;
        const std::string text = mFps > 0 ? toString(mFps) : _("None");

        mFpsLabel->setCaption(text);
        mFpsSlider->setValue(mFps);
        mFpsSlider->setEnabled(mFps > 0);
    }
    else if (id == "opengl" || id == "disableTransparency")
    {
        // Disable transparency disabling when in OpenGL.
        if (mOpenGLCheckBox->isSelected())
        {
            mDisableSDLTransparencyCheckBox->setSelected(false);
            mDisableSDLTransparencyCheckBox->setEnabled(false);
        }
        else
        {
            mDisableSDLTransparencyCheckBox->setEnabled(true);
        }
    }
}
