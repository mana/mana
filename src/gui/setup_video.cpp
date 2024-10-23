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
#include "gui/widgets/dropdown.h"
#include "localplayer.h"
#include "particle.h"

#include "gui/okdialog.h"

#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/slider.h"
#include "gui/widgets/spacer.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/key.hpp>
#include <guichan/listmodel.hpp>

#include <SDL.h>

#include <numeric>
#include <string>
#include <vector>

/**
 * A list model for a given list of strings.
 *
 * \ingroup Interface
 */
class StringListModel : public gcn::ListModel
{
public:
    StringListModel(std::vector<std::string> strings)
        : mStrings(std::move(strings))
    {}

    int getNumberOfElements() override
    {
        return mStrings.size();
    }

    std::string getElementAt(int i) override
    {
        return mStrings[i];
    }

private:
    const std::vector<std::string> mStrings;
};

/**
 * The list model for mode list.
 *
 * \ingroup Interface
 */
class ResolutionListModel : public gcn::ListModel
{
public:
    ResolutionListModel()
    {
        mDisplayModes = Client::getVideo().displayModes();

        // Add a dummy mode for "current window size"
        mDisplayModes.insert(mDisplayModes.begin(), DisplayMode());
    }

    int getNumberOfElements() override
    {
        return mDisplayModes.size();
    }

    std::string getElementAt(int i) override
    {
        if (i == 0)
            return _("Custom");

        const auto &mode = getModeAt(i);
        auto result = toString(mode.width) + "x" + toString(mode.height);

        // Append the aspect ratio
        const int gcd = std::gcd(mode.width, mode.height);
        int aspectWidth = mode.width / gcd;
        int aspectHeight = mode.height / gcd;
        if (aspectWidth == 8 && aspectHeight == 5)
        {
            aspectWidth = 16;
            aspectHeight = 10;
        }
        if (aspectWidth == 7 && aspectHeight == 3)
        {
            aspectWidth = 21;
            aspectHeight = 9;
        }
        if (aspectWidth <= 32)
            result += "  (" + toString(aspectWidth) + ":" + toString(aspectHeight) + ")";

        return result;
    }

    const DisplayMode &getModeAt(int i) const
    {
        return mDisplayModes.at(i);
    }

    /**
     * Returns the index corresponding to the given video resolution
     * or 0 ("Custom") if not found.
     */
    int getIndexOf(int width, int height) const
    {
        for (unsigned i = 1; i < mDisplayModes.size(); i++) {
            const auto &mode = mDisplayModes[i];
            if (mode.width == width && mode.height == height)
                return i;
        }

        return 0;
    }

private:
    std::vector<DisplayMode> mDisplayModes;
};

/**
 * The list model for choosing the scale.
 *
 * \ingroup Interface
 */
class ScaleListModel : public gcn::ListModel
{
public:
    ScaleListModel(const VideoSettings &videoSettings)
        : mVideoSettings(videoSettings)
    {}

    void setVideoSettings(const VideoSettings &videoSettings)
    {
        mVideoSettings = videoSettings;
    }

    int getNumberOfElements() override
    {
        return mVideoSettings.maxScale() + 1;
    }

    std::string getElementAt(int i) override
    {
        if (i == 0)
            return strprintf(_("Auto (%dx)"), mVideoSettings.autoScale());

        return strprintf(_("%dx"), i);
    }

private:
    VideoSettings mVideoSettings;
};


static const char *overlayDetailToString(int detail)
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

static const char *particleDetailToString(int detail)
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
    mVideoSettings(Client::getVideo().settings()),
    mCustomCursorEnabled(config.getBoolValue("customcursor")),
    mParticleEffectsEnabled(config.getBoolValue("particleeffects")),
    mFps(config.getIntValue("fpslimit")),
    mSDLTransparencyDisabled(config.getBoolValue("disableTransparency")),
    mWindowModeListModel(new StringListModel({ _("Windowed"), _("Windowed Fullscreen"), _("Fullscreen") })),
    mResolutionListModel(new ResolutionListModel),
    mScaleListModel(new ScaleListModel(mVideoSettings)),
    mWindowModeDropDown(new DropDown(mWindowModeListModel.get())),
    mResolutionDropDown(new DropDown(mResolutionListModel.get())),
    mScaleDropDown(new DropDown(mScaleListModel.get())),
    mVSyncCheckBox(new CheckBox(_("VSync"), mVideoSettings.vsync)),
    mOpenGLCheckBox(new CheckBox(_("OpenGL (Legacy)"), mVideoSettings.openGL)),
    mCustomCursorCheckBox(new CheckBox(_("Custom cursor"), mCustomCursorEnabled)),
    mParticleEffectsCheckBox(new CheckBox(_("Particle effects"), mParticleEffectsEnabled)),
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

    auto overlayDetailLabel = new Label(_("Ambient FX:"));
    auto particleDetailLabel = new Label(_("Particle detail:"));

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
    mDisableSDLTransparencyCheckBox->setEnabled(!mVideoSettings.openGL);

    // Pre-select the current video mode.
    mWindowModeDropDown->setSelected(static_cast<int>(mVideoSettings.windowMode));
    mResolutionDropDown->setSelected(mResolutionListModel->getIndexOf(mVideoSettings.width,
                                                                      mVideoSettings.height));
    mResolutionDropDown->setEnabled(mVideoSettings.windowMode != WindowMode::WindowedFullscreen);
    mScaleDropDown->setSelected(mVideoSettings.userScale);

    // Set actions
    mWindowModeDropDown->setActionEventId("windowmode");
    mResolutionDropDown->setActionEventId("resolution");
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
    mWindowModeDropDown->addActionListener(this);
    mResolutionDropDown->addActionListener(this);
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

    place(0, 0, new Label(_("Window mode:")));
    place(1, 0, mWindowModeDropDown, 2).setPadding(2);
    place(0, 1, new Label(_("Resolution:")));
    place(1, 1, mResolutionDropDown, 2).setPadding(2);
    place(0, 2, new Label(_("Scale:")));
    place(1, 2, mScaleDropDown, 2).setPadding(2);
    place(0, 3, mVSyncCheckBox, 4);
    place(0, 4, mOpenGLCheckBox, 4);

    place = getPlacer(0, 1);
    place.getCell().setHAlign(LayoutCell::FILL);

    place(0, 0, new Spacer(), 4);
    place(0, 1, mCustomCursorCheckBox, 4);
    place(0, 2, mDisableSDLTransparencyCheckBox, 4);

    place(0, 3, mFpsCheckBox);
    place(1, 3, mFpsSlider, 2);
    place(3, 3, mFpsLabel);

    place(0, 4, mParticleEffectsCheckBox, 4);

    place(0, 5, particleDetailLabel);
    place(1, 5, mParticleDetailSlider, 2);
    place(3, 5, mParticleDetailField);

    place(0, 6, overlayDetailLabel);
    place(1, 6, mOverlayDetailSlider, 2);
    place(3, 6, mOverlayDetailField);
}

Setup_Video::~Setup_Video() = default;

void Setup_Video::apply()
{
    // Video mode changes
    auto &video = Client::getVideo();
    mVideoSettings = video.settings();

    mVideoSettings.windowMode = static_cast<WindowMode>(mWindowModeDropDown->getSelected());

    if (mResolutionDropDown->getSelected() > 0)
    {
        const auto &mode = mResolutionListModel->getModeAt(mResolutionDropDown->getSelected());
        mVideoSettings.width = mode.width;
        mVideoSettings.height = mode.height;
    }

    mVideoSettings.userScale = std::max(0, mScaleDropDown->getSelected());
    mVideoSettings.vsync = mVSyncCheckBox->isSelected();

    if (video.apply(mVideoSettings))
    {
        config.setValue("windowmode", static_cast<int>(mVideoSettings.windowMode));
        config.setValue("scale", mVideoSettings.userScale);
        config.setValue("vsync", mVideoSettings.vsync);
        config.setValue("screenwidth", mVideoSettings.width);
        config.setValue("screenheight", mVideoSettings.height);

        Client::instance()->checkGraphicsSize();
    }
    else
    {
        new OkDialog(_("Error"), _("Failed to change video mode."));
    }

    // OpenGL change
    if (mOpenGLCheckBox->isSelected() != mVideoSettings.openGL)
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
        if (!config.getBoolValue("disableTransparency"))
        {
            new OkDialog(_("Transparency disabled"),
                         _("You must restart to apply changes."));
        }
    }
    else
    {
        if (config.getBoolValue("disableTransparency"))
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
    mVideoSettings.windowMode = static_cast<WindowMode>(config.getIntValue("windowmode"));
    mVideoSettings.vsync = config.getBoolValue("vsync");
    mVideoSettings.openGL = config.getBoolValue("opengl");
    mCustomCursorEnabled = config.getBoolValue("customcursor");
    mParticleEffectsEnabled = config.getBoolValue("particleeffects");
    mOverlayDetail = config.getIntValue("OverlayDetail");
    mSDLTransparencyDisabled = config.getBoolValue("disableTransparency");
}

void Setup_Video::cancel()
{
    // Set back to the current video mode.
    mVideoSettings = Client::getVideo().settings();
    mWindowModeDropDown->setSelected(static_cast<int>(mVideoSettings.windowMode));
    mResolutionDropDown->setSelected(mResolutionListModel->getIndexOf(mVideoSettings.width,
                                                                      mVideoSettings.height));
    mScaleDropDown->setSelected(mVideoSettings.userScale);
    mVSyncCheckBox->setSelected(mVideoSettings.vsync);
    mOpenGLCheckBox->setSelected(mVideoSettings.openGL);
    mCustomCursorCheckBox->setSelected(mCustomCursorEnabled);
    mParticleEffectsCheckBox->setSelected(mParticleEffectsEnabled);
    mFpsCheckBox->setSelected(mFps > 0);
    mFpsSlider->setValue(mFps);
    mFpsSlider->setEnabled(mFps > 0);
    mOverlayDetailSlider->setValue(mOverlayDetail);
    mParticleDetailSlider->setValue(mParticleDetail);
    std::string text = mFpsCheckBox->isSelected() ? toString(mFps) : _("None");
    mFpsLabel->setCaption(text);
    mDisableSDLTransparencyCheckBox->setSelected(mSDLTransparencyDisabled);
    mDisableSDLTransparencyCheckBox->setEnabled(!mVideoSettings.openGL);

    config.setValue("windowmode", static_cast<int>(mVideoSettings.windowMode));

    config.setValue("customcursor", mCustomCursorEnabled);
    config.setValue("particleeffects", mParticleEffectsEnabled);
    config.setValue("opengl", mVideoSettings.openGL);
    config.setValue("disableTransparency", mSDLTransparencyDisabled);
}

void Setup_Video::action(const gcn::ActionEvent &event)
{
    const std::string &id = event.getId();

    if (id == "windowmode" || id == "resolution")
    {
        auto windowMode = static_cast<WindowMode>(mWindowModeDropDown->getSelected());

        // When the window mode is "windowed fullscreen" we should select the
        // desktop resolution and disable the option to change it
        if (windowMode == WindowMode::WindowedFullscreen)
        {
            const auto &desktop = Client::getVideo().desktopDisplayMode();
            mResolutionDropDown->setSelected(
                        mResolutionListModel->getIndexOf(desktop.width, desktop.height));
            mResolutionDropDown->setEnabled(false);
        }
        else
        {
            mResolutionDropDown->setEnabled(true);
        }

        refreshScaleList();
    }
    else if (id == "resolution")
    {
        refreshScaleList();
    }
    else if (id == "customcursor")
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

void Setup_Video::refreshScaleList()
{
    if (mResolutionDropDown->getSelected() > 0)
    {
        const auto &mode = mResolutionListModel->getModeAt(mResolutionDropDown->getSelected());
        mVideoSettings.width = mode.width;
        mVideoSettings.height = mode.height;
    }
    else
    {
        auto &videoSettings = Client::getVideo().settings();
        mVideoSettings.width = videoSettings.width;
        mVideoSettings.height = videoSettings.height;
    }

    mScaleListModel->setVideoSettings(mVideoSettings);
    mScaleDropDown->setListModel(mScaleListModel.get());
    mScaleDropDown->setSelected(mVideoSettings.userScale);
}
