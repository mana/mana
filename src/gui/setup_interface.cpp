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

#include "gui/setup_interface.h"

#include "configuration.h"
#include "game.h"
#include "graphics.h"
#include "localplayer.h"
#include "log.h"
#include "main.h"
//#include "particle.h"

#include "gui/okdialog.h"

#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/slider.h"
#include "gui/widgets/spacer.h"
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

Setup_Interface::Setup_Interface():
    mShowMonsterDamageEnabled(config.getBoolValue("showMonstersTakedDamage")),
    mVisibleNamesEnabled(config.getBoolValue("visiblenames")),
    mNameEnabled(config.getBoolValue("showownname")),
    mPickupChatEnabled(config.getBoolValue("showpickupchat")),
    mPickupParticleEnabled(config.getBoolValue("showpickupparticle")),
    mOpacity(config.getFloatValue("guialpha")),
    mSpeechMode(static_cast<Being::Speech>(config.getIntValue("speech"))),
    mVisibleNamesCheckBox(new CheckBox(_("Visible names"),
                                       mVisibleNamesEnabled)),
    mNameCheckBox(new CheckBox(_("Show own name"), mNameEnabled)),
    mPickupNotifyLabel(new Label(_("Show pickup notification:"))),
    // TRANSLATORS: Refers to "Show pickup notification"
    mPickupChatCheckBox(new CheckBox(_("in chat"), mPickupChatEnabled)),
    // TRANSLATORS: Refers to "Show pickup notification"
    mPickupParticleCheckBox(new CheckBox(_("as particle"),
                                         mPickupParticleEnabled)),
    mSpeechSlider(new Slider(0, 3)),
    mSpeechLabel(new Label("")),
    mAlphaSlider(new Slider(0.2, 1.0)),
    mFontSize(config.getIntValue("fontSize"))
{
    setName(_("Interface"));

    // Create widgets
    Spacer *space = new Spacer(0,10);

    mShowMonsterDamageCheckBox = new CheckBox(_("Show damage"),
                                              mShowMonsterDamageEnabled);

    speechLabel = new Label(_("Overhead text:"));
    alphaLabel = new Label(_("Gui opacity"));
    fontSizeLabel = new Label(_("Font size:"));

    mFontSizeListModel = new FontSizeChoiceListModel;
    mFontSizeDropDown = new DropDown(mFontSizeListModel);

    mAlphaSlider->setValue(mOpacity);
    mAlphaSlider->setWidth(90);
    mAlphaSlider->setEnabled(!config.getBoolValue("disableTransparency"));


    // Set actions
    mShowMonsterDamageCheckBox->setActionEventId("monsterdamage");
    mVisibleNamesCheckBox->setActionEventId("visiblenames");
    mPickupChatCheckBox->setActionEventId("pickupchat");
    mPickupParticleCheckBox->setActionEventId("pickupparticle");
    mNameCheckBox->setActionEventId("showownname");
    mAlphaSlider->setActionEventId("guialpha");
    mSpeechSlider->setActionEventId("speech");

    // Set Listeners
    mShowMonsterDamageCheckBox->addActionListener(this);
    mVisibleNamesCheckBox->addActionListener(this);
    mPickupChatCheckBox->addActionListener(this);
    mPickupParticleCheckBox->addActionListener(this);
    mNameCheckBox->addActionListener(this);
    mAlphaSlider->addActionListener(this);
    mSpeechSlider->addActionListener(this);

    mSpeechLabel->setCaption(speechModeToString(mSpeechMode));
    mSpeechSlider->setValue(mSpeechMode);

    mFontSizeDropDown->setSelected(mFontSize - 10);
    mFontSizeDropDown->adjustHeight();

    // Do the layout
    place(0, 0, mVisibleNamesCheckBox, 3);
    place(3, 0, mNameCheckBox, 3);

    place(0, 1, mShowMonsterDamageCheckBox, 3);

    place(0, 2, space, 1, 1);

    place(0, 3, mPickupNotifyLabel, 6);

    place(0, 4, mPickupChatCheckBox, 3);
    place(3, 4, mPickupParticleCheckBox, 3);

    place(0, 5, space, 1, 1);

    place(0, 6, fontSizeLabel, 2);
    place(2, 6, mFontSizeDropDown, 2);

    place(0, 7, space, 1, 1);

    place(0, 8, mAlphaSlider, 2);
    place(2, 8, alphaLabel, 2);

    place(0, 9, mSpeechSlider, 2);
    place(2, 9, speechLabel, 2);
    place(4, 9, mSpeechLabel, 2).setPadding(2);
}

Setup_Interface::~Setup_Interface()
{
    delete mFontSizeListModel;
}

void Setup_Interface::apply()
{
    config.setValue("fontSize", mFontSizeDropDown->getSelected() + 10);

    mShowMonsterDamageEnabled = config.getBoolValue("showMonstersTakedDamage");
    mVisibleNamesEnabled = config.getBoolValue("visiblenames");
    mNameEnabled = config.getBoolValue("showownname");
    mSpeechMode = static_cast<Being::Speech>(config.getIntValue("speech"));
    mOpacity = config.getFloatValue("guialpha");
    mPickupChatEnabled = config.getBoolValue("showpickupchat");
    mPickupParticleEnabled = config.getBoolValue("showpickupparticle");
}

void Setup_Interface::cancel()
{
    mShowMonsterDamageCheckBox->setSelected(mShowMonsterDamageEnabled);
    mVisibleNamesCheckBox->setSelected(mVisibleNamesEnabled);
    mSpeechSlider->setValue(mSpeechMode);
    mNameCheckBox->setSelected(mNameEnabled);
    mAlphaSlider->setValue(mOpacity);
    //mAlphaSlider->setEnabled(!mSDLTransparencyDisabled);

    config.setValue("showMonstersTakedDamage", mShowMonsterDamageEnabled);
    config.setValue("visiblenames", mVisibleNamesEnabled);
    config.setValue("speech", mSpeechMode);
    config.setValue("showownname", mNameEnabled);
    if (local_player)
        local_player->setCheckNameSetting(true);
    config.setValue("guialpha", mOpacity);
    config.setValue("showpickupchat", mPickupChatEnabled);
    config.setValue("showpickupparticle", mPickupParticleEnabled);
}

void Setup_Interface::action(const gcn::ActionEvent &event)
{
    const std::string &id = event.getId();

    if (id == "guialpha")
    {
        config.setValue("guialpha", mAlphaSlider->getValue());
    }
    else if (id == "monsterdamage")
    {
        config.setValue("showMonstersTakedDamage", mShowMonsterDamageCheckBox->isSelected());
    }
    else if (id == "visiblenames")
    {
        config.setValue("visiblenames", mVisibleNamesCheckBox->isSelected());
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
        if (local_player)
            local_player->setCheckNameSetting(true);
        config.setValue("showownname", mNameCheckBox->isSelected());
    }
}

