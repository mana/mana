/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "configuration.h"
#include "game.h"
#include "graphics.h"
#include "localplayer.h"
#include "log.h"
#include "main.h"
#include "particle.h"

#include "gui/okdialog.h"

#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/slider.h"
#include "gui/widgets/textfield.h"
#include "gui/widgets/dropdown.h"

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

int ModeListModel::getIndexOf(const std::string &widthXHeightMode)
{
    std::string currentMode = "";
    for (int i = 0; i < getNumberOfElements(); i++)
    {
        currentMode = getElementAt(i);
        if (currentMode == widthXHeightMode)
        {
            return i;
        }
    }
    return -1;
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

const char *SERVLIST_ORDER_BY[2] =
{
    N_("Last usage"),
    N_("Name")
};

class ServerListOrderListModel : public gcn::ListModel
{
public:
    virtual ~ServerListOrderListModel() { }

    virtual int getNumberOfElements()
    {
        return 2;
    }

    virtual std::string getElementAt(int i)
    {
        if (i >= getNumberOfElements())
            return _("???");

        return SERVLIST_ORDER_BY[i];
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
    mShowMonsterDamageEnabled(config.getBoolValue("showMonstersTakedDamage")),
    mVisibleNamesEnabled(config.getBoolValue("visiblenames")),
    mParticleEffectsEnabled(config.getBoolValue("particleeffects")),
    mNameEnabled(config.getBoolValue("showownname")),
    mNPCLogEnabled(config.getBoolValue("logNpcInGui")),
    mPickupChatEnabled(config.getBoolValue("showpickupchat")),
    mPickupParticleEnabled(config.getBoolValue("showpickupparticle")),
    mOpacity(config.getFloatValue("guialpha")),
    mFps(config.getIntValue("fpslimit")),
    mSDLTransparencyDisabled(config.getBoolValue("disableTransparency")),
    mSpeechMode(static_cast<Being::Speech>(config.getIntValue("speech"))),
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
    mNameCheckBox(new CheckBox(_("Show own name"), mNameEnabled)),
    mNPCLogCheckBox(new CheckBox(_("Log NPC dialogue"), mNPCLogEnabled)),
    mPickupNotifyLabel(new Label(_("Show pickup notification"))),
    // TRANSLATORS: Refers to "Show pickup notification"
    mPickupChatCheckBox(new CheckBox(_("in chat"), mPickupChatEnabled)),
    // TRANSLATORS: Refers to "Show pickup notification"
    mPickupParticleCheckBox(new CheckBox(_("as particle"),
                                         mPickupParticleEnabled)),
    mSpeechSlider(new Slider(0, 3)),
    mSpeechLabel(new Label("")),
    mAlphaSlider(new Slider(0.2, 1.0)),
    mFpsCheckBox(new CheckBox(_("FPS limit:"))),
    mFpsSlider(new Slider(10, 120)),
    mFpsLabel(new Label),
    mOverlayDetail(config.getIntValue("OverlayDetail")),
    mOverlayDetailSlider(new Slider(0, 2)),
    mOverlayDetailField(new Label),
    mParticleDetail(3 - config.getIntValue("particleEmitterSkip")),
    mParticleDetailSlider(new Slider(0, 3)),
    mParticleDetailField(new Label),
    mFontSize(config.getIntValue("fontSize")),
    mServerListOrder(config.getIntValue("serverListOrder")),
    mDisableSDLTransparencyCheckBox(
                          new CheckBox(_("Disable transparency (Low CPU mode)"),
                                       mSDLTransparencyDisabled))
{
    setName(_("Video"));

    mShowMonsterDamageCheckBox = new CheckBox(_("Show damage"),
                                              mShowMonsterDamageEnabled);

    ScrollArea *scrollArea = new ScrollArea(mModeList);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    speechLabel = new Label(_("Overhead text"));
    alphaLabel = new Label(_("Gui opacity"));
    overlayDetailLabel = new Label(_("Ambient FX"));
    particleDetailLabel = new Label(_("Particle detail"));
    fontSizeLabel = new Label(_("Font size"));
    serverListOrderLabel = new Label(_("Order servers by"));

    mFontSizeListModel = new FontSizeChoiceListModel;
    mFontSizeDropDown = new DropDown(mFontSizeListModel);

    mServerListOrderListModel = new ServerListOrderListModel;
    mServerListOrderDropDown = new DropDown(mServerListOrderListModel);

    mModeList->setEnabled(true);

#ifndef USE_OPENGL
    mOpenGLCheckBox->setEnabled(false);
#endif

    mAlphaSlider->setValue(mOpacity);
    mAlphaSlider->setWidth(90);
    mAlphaSlider->setEnabled(!mSDLTransparencyDisabled);

    mFpsLabel->setCaption(mFps > 0 ? toString(mFps) : _("None"));
    mFpsLabel->setWidth(60);
    mFpsSlider->setValue(mFps);
    mFpsSlider->setEnabled(mFps > 0);
    mFpsCheckBox->setSelected(mFps > 0);

    // If the openGL Mode is enabled, disabling the transaprency
    // is irrelevant.
    mDisableSDLTransparencyCheckBox->setEnabled(!mOpenGLEnabled);

    // Pre-select the current video mode.
    std::string videoMode = toString(graphics->getWidth()) + "x"
                            + toString(graphics->getHeight());
    mModeList->setSelected(mModeListModel->getIndexOf(videoMode));

    mModeList->setActionEventId("videomode");
    mCustomCursorCheckBox->setActionEventId("customcursor");
    mShowMonsterDamageCheckBox->setActionEventId("monsterdamage");
    mVisibleNamesCheckBox->setActionEventId("visiblenames");
    mParticleEffectsCheckBox->setActionEventId("particleeffects");
    mPickupChatCheckBox->setActionEventId("pickupchat");
    mPickupParticleCheckBox->setActionEventId("pickupparticle");
    mNameCheckBox->setActionEventId("showownname");
    mNPCLogCheckBox->setActionEventId("lognpc");
    mAlphaSlider->setActionEventId("guialpha");
    mFpsCheckBox->setActionEventId("fpslimitcheckbox");
    mSpeechSlider->setActionEventId("speech");
    mFpsSlider->setActionEventId("fpslimitslider");
    mOverlayDetailSlider->setActionEventId("overlaydetailslider");
    mOverlayDetailField->setActionEventId("overlaydetailfield");
    mOpenGLCheckBox->setActionEventId("opengl");
    mParticleDetailSlider->setActionEventId("particledetailslider");
    mParticleDetailField->setActionEventId("particledetailfield");
    mDisableSDLTransparencyCheckBox->setActionEventId("disableTransparency");

    mModeList->addActionListener(this);
    mCustomCursorCheckBox->addActionListener(this);
    mShowMonsterDamageCheckBox->addActionListener(this);
    mVisibleNamesCheckBox->addActionListener(this);
    mOpenGLCheckBox->addActionListener(this);
    mParticleEffectsCheckBox->addActionListener(this);
    mPickupChatCheckBox->addActionListener(this);
    mPickupParticleCheckBox->addActionListener(this);
    mNameCheckBox->addActionListener(this);
    mNPCLogCheckBox->addActionListener(this);
    mAlphaSlider->addActionListener(this);
    mFpsCheckBox->addActionListener(this);
    mSpeechSlider->addActionListener(this);
    mFpsSlider->addActionListener(this);
    mOverlayDetailSlider->addActionListener(this);
    mOverlayDetailField->addKeyListener(this);
    mParticleDetailSlider->addActionListener(this);
    mParticleDetailField->addKeyListener(this);
    mDisableSDLTransparencyCheckBox->addActionListener(this);

    mSpeechLabel->setCaption(speechModeToString(mSpeechMode));
    mSpeechSlider->setValue(mSpeechMode);

    mOverlayDetailField->setCaption(overlayDetailToString(mOverlayDetail));
    mOverlayDetailSlider->setValue(mOverlayDetail);

    mParticleDetailField->setCaption(particleDetailToString(mParticleDetail));
    mParticleDetailSlider->setValue(mParticleDetail);

    mFontSizeDropDown->setSelected(mFontSize - 10);
    mFontSizeDropDown->adjustHeight();

    mServerListOrderDropDown->setSelected(mServerListOrder);
    mServerListOrderDropDown->adjustHeight();

    // Do the layout
    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, scrollArea, 1, 5).setPadding(2);
    place(1, 0, mFsCheckBox, 2);
    place(3, 0, mOpenGLCheckBox, 1);

    place(1, 1, mCustomCursorCheckBox, 3);
    place(3, 1, mShowMonsterDamageCheckBox, 3);

    place(1, 2, mVisibleNamesCheckBox, 3);
    place(3, 2, mNameCheckBox, 1);

    place(1, 3, mParticleEffectsCheckBox, 3);
    place(3, 3, mNPCLogCheckBox, 1);

    place(1, 4, mPickupNotifyLabel, 4);

    place(1, 5, mPickupChatCheckBox, 1);
    place(2, 5, mPickupParticleCheckBox, 2);

    place(0, 6, fontSizeLabel, 3);
    place(1, 6, mFontSizeDropDown, 2);

    place(0, 7, mAlphaSlider);
    place(1, 7, alphaLabel, 3);

    place(0, 8, mFpsSlider);
    place(1, 8, mFpsCheckBox).setPadding(3);
    place(2, 8, mFpsLabel).setPadding(1);

    place(0, 9, mSpeechSlider);
    place(1, 9, speechLabel);
    place(2, 9, mSpeechLabel, 3).setPadding(2);

    place(0, 10, mOverlayDetailSlider);
    place(1, 10, overlayDetailLabel);
    place(2, 10, mOverlayDetailField, 3).setPadding(2);

    place(0, 11, mParticleDetailSlider);
    place(1, 11, particleDetailLabel);
    place(2, 11, mParticleDetailField, 3).setPadding(2);

    place(0, 12, mDisableSDLTransparencyCheckBox, 4);

    place(0, 13, serverListOrderLabel, 3);
    place(1, 13, mServerListOrderDropDown, 2);

    setDimension(gcn::Rectangle(0, 0, 365, 300));
}

Setup_Video::~Setup_Video()
{
    delete mModeListModel;
    delete mModeList;
    delete mFontSizeListModel;
    delete mServerListOrderListModel;
}

void Setup_Video::apply()
{
    // Full screen changes
    bool fullscreen = mFsCheckBox->isSelected();
    if (fullscreen != config.getBoolValue("screen"))
    {
        /* The OpenGL test is only necessary on Windows, since switching
         * to/from full screen works fine on Linux. On Windows we'd have to
         * reinitialize the OpenGL state and reload all textures.
         *
         * See http://libsdl.org/cgi/docwiki.cgi/SDL_SetVideoMode
         */

#if defined(WIN32) || defined(__APPLE__)
        // checks for opengl usage
        if (!config.getBoolValue("opengl"))
        {
#endif
            if (!graphics->setFullscreen(fullscreen))
            {
                fullscreen = !fullscreen;
                if (!graphics->setFullscreen(fullscreen))
                {
                    std::stringstream errorMessage;
                    if (fullscreen)
                    {
                        errorMessage << _("Failed to switch to windowed mode "
                                          "and restoration of old mode also "
                                          "failed!") << std::endl;
                    }
                    else
                    {
                        errorMessage << _("Failed to switch to fullscreen mode "
                                          "and restoration of old mode also "
                                          "failed!") << std::endl;
                    }
                    logger->error(errorMessage.str());
                }
            }
#if defined(WIN32) || defined(__APPLE__)
        }
        else
        {
            new OkDialog(_("Switching to Full Screen"),
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
        if (mOpenGLCheckBox->isSelected())
        {
            new OkDialog(_("Changing to OpenGL"),
                         _("Applying change to OpenGL requires restart. "
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
    config.setValue("fontSize", mFontSizeDropDown->getSelected() + 10);
    config.setValue("serverListOrder", mServerListOrderDropDown->getSelected());

    // We sync old and new values at apply time
    mFullScreenEnabled = config.getBoolValue("screen");
    mCustomCursorEnabled = config.getBoolValue("customcursor");
    mShowMonsterDamageEnabled = config.getBoolValue("showMonstersTakedDamage");
    mVisibleNamesEnabled = config.getBoolValue("visiblenames");
    mParticleEffectsEnabled = config.getBoolValue("particleeffects");
    mNameEnabled = config.getBoolValue("showownname");
    mNPCLogEnabled = config.getBoolValue("logNpcInGui");
    mSpeechMode = static_cast<Being::Speech>(config.getIntValue("speech"));
    mOpacity = config.getFloatValue("guialpha");
    mOverlayDetail = config.getIntValue("OverlayDetail");
    mOpenGLEnabled = config.getBoolValue("opengl");
    mPickupChatEnabled = config.getBoolValue("showpickupchat");
    mPickupParticleEnabled = config.getBoolValue("showpickupparticle");
    mSDLTransparencyDisabled = config.getBoolValue("disableTransparency");
}

void Setup_Video::cancel()
{
    mFpsCheckBox->setSelected(mFps > 0);
    mFsCheckBox->setSelected(mFullScreenEnabled);
    mOpenGLCheckBox->setSelected(mOpenGLEnabled);
    mCustomCursorCheckBox->setSelected(mCustomCursorEnabled);
    mShowMonsterDamageCheckBox->setSelected(mShowMonsterDamageEnabled);
    mVisibleNamesCheckBox->setSelected(mVisibleNamesEnabled);
    mParticleEffectsCheckBox->setSelected(mParticleEffectsEnabled);
    mFpsSlider->setValue(mFps);
    mFpsSlider->setEnabled(mFps > 0);
    mSpeechSlider->setValue(mSpeechMode);
    mNameCheckBox->setSelected(mNameEnabled);
    mNPCLogCheckBox->setSelected(mNPCLogEnabled);
    mAlphaSlider->setValue(mOpacity);
    mAlphaSlider->setEnabled(!mSDLTransparencyDisabled);
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
    config.setValue("screenwidth", graphics->getWidth());
    config.setValue("screenheight", graphics->getHeight());

    config.setValue("customcursor", mCustomCursorEnabled);
    config.setValue("showMonstersTakedDamage", mShowMonsterDamageEnabled);
    config.setValue("visiblenames", mVisibleNamesEnabled);
    config.setValue("particleeffects", mParticleEffectsEnabled);
    config.setValue("speech", mSpeechMode);
    config.setValue("showownname", mNameEnabled);
    if (player_node)
        player_node->setCheckNameSetting(true);
    config.setValue("logNpcInGui", mNPCLogEnabled);
    config.setValue("guialpha", mOpacity);
    config.setValue("opengl", mOpenGLEnabled);
    config.setValue("showpickupchat", mPickupChatEnabled);
    config.setValue("showpickupparticle", mPickupParticleEnabled);
    config.setValue("disableTransparency", mSDLTransparencyDisabled);
}

void Setup_Video::action(const gcn::ActionEvent &event)
{
    const std::string &id = event.getId();

    if (id == "videomode")
    {
        const std::string mode = mModeListModel->getElementAt(mModeList->getSelected());
        const int width = atoi(mode.substr(0, mode.find("x")).c_str());
        const int height = atoi(mode.substr(mode.find("x") + 1).c_str());

          // TODO: Find out why the drawing area doesn't resize without a restart.
        if (width != graphics->getWidth() || height != graphics->getHeight())
        {
            if (width < graphics->getWidth() || height < graphics->getHeight())
                new OkDialog(_("Screen Resolution Changed"),
                       _("Restart your client for the change to take effect.")
                       + std::string("\n") +
                _("Some windows may be moved to fit the lowered resolution."));
            else
                new OkDialog(_("Screen Resolution Changed"),
                     _("Restart your client for the change to take effect."));
        }

        config.setValue("screenwidth", width);
        config.setValue("screenheight", height);
    }
    else if (id == "guialpha")
    {
        config.setValue("guialpha", mAlphaSlider->getValue());
    }
    else if (id == "customcursor")
    {
        config.setValue("customcursor", mCustomCursorCheckBox->isSelected());
    }
    else if (id == "monsterdamage")
    {
        config.setValue("showMonstersTakedDamage", mShowMonsterDamageCheckBox->isSelected());
    }
    else if (id == "visiblenames")
    {
        config.setValue("visiblenames", mVisibleNamesCheckBox->isSelected());
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
    else if (id == "pickupchat")
    {
        config.setValue("showpickupchat", mPickupChatCheckBox->isSelected());
    }
    else if (id == "pickupparticle")
    {
        config.setValue("showpickupparticle",
                        mPickupParticleCheckBox->isSelected());
    }
    else if (id == "speech")
    {
        Being::Speech val = (Being::Speech)mSpeechSlider->getValue();
        mSpeechLabel->setCaption(speechModeToString(val));
        mSpeechSlider->setValue(val);
        config.setValue("speech", val);
    }
    else if (id == "showownname")
    {
        // Notify the local player that settings have changed for the name
        // and requires an update
        if (player_node)
            player_node->setCheckNameSetting(true);
        config.setValue("showownname", mNameCheckBox->isSelected());
    }
    else if (id == "lognpc")
    {
        config.setValue("logNpcInGui", mNPCLogCheckBox->isSelected());
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

        // Disable gui opacity slider when disabling transparency.
        if (mDisableSDLTransparencyCheckBox->isEnabled())
            mAlphaSlider->setEnabled(
                                !mDisableSDLTransparencyCheckBox->isSelected());
        else
            mAlphaSlider->setEnabled(true);
    }
}
