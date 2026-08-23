/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "gui/okdialog.h"
#include "gui/widgets/checkbox.h"
#include "gui/widgets/dropdown.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/slider.h"
#include "gui/widgets/spacer.h"

#include "resources/theme.h"

#include "utils/gettext.h"
#include "utils/language.h"
#include "utils/stringutils.h"

#include <guichan/key.hpp>
#include <guichan/listmodel.hpp>

#include <SDL.h>

#include <algorithm>
#include <string>

class ThemesListModel : public gcn::ListModel
{
public:
    int getNumberOfElements() override
    {
        return gui->getAvailableThemes().size();
    }

    std::string getElementAt(int i) override
    {
        return gui->getAvailableThemes().at(i).getName();
    }

    static int getThemeIndex(const std::string &path)
    {
        auto &themes = gui->getAvailableThemes();
        auto themeIt = std::find_if(themes.begin(),
                                    themes.end(),
                                    [&](const ThemeInfo &theme) {
                                        return theme.getPath() == path;
                                    });
        return themeIt != themes.end() ? std::distance(themes.begin(), themeIt) : 0;
    }
};


/**
 * Lists the available languages, with the system language as first entry.
 */
class LanguageListModel : public gcn::ListModel
{
public:
    int getNumberOfElements() override
    {
        return getAvailableLanguages().size() + 1;
    }

    std::string getElementAt(int i) override
    {
        if (i == 0)
        {
            // TRANSLATORS: Placeholder is the name of a language
            return strprintf(_("Default (%s)"),
                             getSystemLanguage().name.c_str());
        }

        return getAvailableLanguages().at(i - 1).name;
    }

    static int getLanguageIndex(const std::string &code)
    {
        if (code.empty())
            return 0;

        auto &languages = getAvailableLanguages();
        auto it = std::find_if(languages.begin(),
                               languages.end(),
                               [&](const LanguageInfo &language) {
                                   return language.code == code;
                               });
        return it != languages.end() ? std::distance(languages.begin(), it) + 1 : 0;
    }

    static std::string getLanguageCode(int index)
    {
        if (index <= 0 || index > (int) getAvailableLanguages().size())
            return std::string();

        return getAvailableLanguages().at(index - 1).code;
    }
};


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
    int getNumberOfElements() override
    {
        return 4;
    }

    std::string getElementAt(int i) override
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
    mShowMonsterDamageEnabled(config.showMonstersTakedDamage),
    mVisibleNamesEnabled(config.visibleNames),
    mNameEnabled(config.showOwnName),
    mNPCLogEnabled(config.logNpcInGui),
    mPickupChatEnabled(config.showPickupChat),
    mPickupParticleEnabled(config.showPickupParticle),
    mOpacity(config.guiAlpha),
    mSpeechMode(config.speech),
    mVisibleNamesCheckBox(new CheckBox(_("Visible names"),
                                       mVisibleNamesEnabled)),
    mNameCheckBox(new CheckBox(_("Show own name"), mNameEnabled)),
    mNPCLogCheckBox(new CheckBox(_("Log NPC dialogue"), mNPCLogEnabled)),
    mPickupNotifyLabel(new Label(_("Show pickup notification:"))),
    // TRANSLATORS: Refers to "Show pickup notification"
    mPickupChatCheckBox(new CheckBox(_("in chat"), mPickupChatEnabled)),
    // TRANSLATORS: Refers to "Show pickup notification"
    mPickupParticleCheckBox(new CheckBox(_("as particle"),
                                         mPickupParticleEnabled)),
    mSpeechSlider(new Slider(0, 3)),
    mSpeechLabel(new Label(std::string())),
    mAlphaSlider(new Slider(0.2, 1.0))
{
    setName(_("Interface"));

    // Create widgets
    auto *space = new Spacer(0,10);

    mShowMonsterDamageCheckBox = new CheckBox(_("Show damage"),
                                              mShowMonsterDamageEnabled);

    gcn::Label *speechLabel = new Label(_("Overhead text:"));
    gcn::Label *alphaLabel = new Label(_("GUI opacity"));
    gcn::Label *themeLabel = new Label(_("Theme:"));
    gcn::Label *fontSizeLabel = new Label(_("Font size:"));

    // Without any translations there is no language to choose
    if (getAvailableLanguages().size() > 1)
    {
        mLanguageListModel = std::make_unique<LanguageListModel>();
        mLanguageDropDown = new DropDown(mLanguageListModel.get());
    }

    mThemesListModel = std::make_unique<ThemesListModel>();
    mThemeDropDown = new DropDown(mThemesListModel.get());

    mFontSizeListModel = std::make_unique<FontSizeChoiceListModel>();
    mFontSizeDropDown = new DropDown(mFontSizeListModel.get());

    mAlphaSlider->setValue(mOpacity);
    mAlphaSlider->setWidth(90);
    mAlphaSlider->setEnabled(!config.disableTransparency);

    // Set actions
    mShowMonsterDamageCheckBox->setActionEventId("monsterdamage");
    mVisibleNamesCheckBox->setActionEventId("visiblenames");
    mPickupChatCheckBox->setActionEventId("pickupchat");
    mPickupParticleCheckBox->setActionEventId("pickupparticle");
    mNameCheckBox->setActionEventId("showownname");
    mNPCLogCheckBox->setActionEventId("lognpc");
    mThemeDropDown->setActionEventId("theme");
    mAlphaSlider->setActionEventId("guialpha");
    mSpeechSlider->setActionEventId("speech");

    // Set Listeners
    mShowMonsterDamageCheckBox->addActionListener(this);
    mVisibleNamesCheckBox->addActionListener(this);
    mPickupChatCheckBox->addActionListener(this);
    mPickupParticleCheckBox->addActionListener(this);
    mNameCheckBox->addActionListener(this);
    mNPCLogCheckBox->addActionListener(this);
    mThemeDropDown->addActionListener(this);
    mAlphaSlider->addActionListener(this);
    mSpeechSlider->addActionListener(this);

    mSpeechLabel->setCaption(speechModeToString(mSpeechMode));
    mSpeechSlider->setValue(mSpeechMode);

    if (mLanguageDropDown)
    {
        mLanguageDropDown->setSelected(
                    LanguageListModel::getLanguageIndex(config.language));
        mLanguageDropDown->adjustHeight();
    }

    mThemeDropDown->setSelected(ThemesListModel::getThemeIndex(config.theme));

    mFontSizeDropDown->setSelected(config.fontSize - 10);
    mFontSizeDropDown->adjustHeight();

    // Do the layout
    place(0, 0, mVisibleNamesCheckBox, 3);
    place(3, 0, mNameCheckBox, 3);

    place(0, 1, mShowMonsterDamageCheckBox, 3);
    place(3, 1, mNPCLogCheckBox, 3);

    place(0, 2, space, 1, 1);

    place(0, 3, mPickupNotifyLabel, 6);

    place(0, 4, mPickupChatCheckBox, 3);
    place(3, 4, mPickupParticleCheckBox, 3);

    place(0, 5, space, 1, 1);

    int row = 6;

    if (mLanguageDropDown)
    {
        place(0, row, new Label(_("Language:")), 2);
        place(2, row, mLanguageDropDown, 2).setPadding(2);
        ++row;
    }

    place(0, row, themeLabel, 2);
    place(2, row, mThemeDropDown, 2).setPadding(2);
    ++row;

    place(0, row, fontSizeLabel, 2);
    place(2, row, mFontSizeDropDown, 2).setPadding(2);
    ++row;

    place(0, row, space, 1, 1);
    ++row;

    place(0, row, mAlphaSlider, 2);
    place(2, row, alphaLabel, 2);
    ++row;

    place(0, row, mSpeechSlider, 2);
    place(2, row, speechLabel, 2);
    place(4, row, mSpeechLabel, 2).setPadding(2);
}

Setup_Interface::~Setup_Interface() = default;

void Setup_Interface::apply()
{
    auto &theme = gui->getAvailableThemes().at(mThemeDropDown->getSelected());
    auto fontSize = mFontSizeDropDown->getSelected() + 10;
    // Only apply a language change when the user made one, so that a language
    // whose catalog is currently not installed is not silently dropped.
    const bool languageChanged = mLanguageDropDown
            && mLanguageDropDown->getSelected() !=
               LanguageListModel::getLanguageIndex(config.language);
    if (config.theme != theme.getPath() || config.fontSize != fontSize ||
            languageChanged)
    {
        new OkDialog(_("Changing Language, Theme or Font Size"),
                     _("Language, theme and font size changes will apply "
                       "after restart."));
    }
    config.theme = theme.getPath();
    config.fontSize = fontSize;
    if (languageChanged)
        config.language = LanguageListModel::getLanguageCode(
                    mLanguageDropDown->getSelected());

    mShowMonsterDamageEnabled = config.showMonstersTakedDamage;
    mVisibleNamesEnabled = config.visibleNames;
    mNameEnabled = config.showOwnName;
    mNPCLogEnabled = config.logNpcInGui;
    mSpeechMode = config.speech;
    mOpacity = config.guiAlpha;
    mPickupChatEnabled = config.showPickupChat;
    mPickupParticleEnabled = config.showPickupParticle;
}

void Setup_Interface::cancel()
{
    mShowMonsterDamageCheckBox->setSelected(mShowMonsterDamageEnabled);
    mVisibleNamesCheckBox->setSelected(mVisibleNamesEnabled);
    mSpeechSlider->setValue(mSpeechMode);
    mNameCheckBox->setSelected(mNameEnabled);
    mNPCLogCheckBox->setSelected(mNPCLogEnabled);
    if (mLanguageDropDown)
        mLanguageDropDown->setSelected(
                    LanguageListModel::getLanguageIndex(config.language));
    mThemeDropDown->setSelected(ThemesListModel::getThemeIndex(config.theme));
    mFontSizeDropDown->setSelected(config.fontSize - 10);
    mAlphaSlider->setValue(mOpacity);
    //mAlphaSlider->setEnabled(!mSDLTransparencyDisabled);

    config.showMonstersTakedDamage = mShowMonsterDamageEnabled;
    setConfigValue(&Config::visibleNames, mVisibleNamesEnabled);
    config.speech = mSpeechMode;
    setConfigValue(&Config::showOwnName, mNameEnabled);
    config.logNpcInGui = mNPCLogEnabled;
    setConfigValue<float>(&Config::guiAlpha, mOpacity);
    config.showPickupChat = mPickupChatEnabled;
    config.showPickupParticle = mPickupParticleEnabled;
}

void Setup_Interface::action(const gcn::ActionEvent &event)
{
    const std::string &id = event.getId();

    if (id == "guialpha")
    {
        setConfigValue<float>(&Config::guiAlpha, mAlphaSlider->getValue());
    }
    else if (id == "monsterdamage")
    {
        config.showMonstersTakedDamage = mShowMonsterDamageCheckBox->isSelected();
    }
    else if (id == "visiblenames")
    {
        setConfigValue(&Config::visibleNames, mVisibleNamesCheckBox->isSelected());
    }
    else if (id == "pickupchat")
    {
        config.showPickupChat = mPickupChatCheckBox->isSelected();
    }
    else if (id == "pickupparticle")
    {
        config.showPickupParticle = mPickupParticleCheckBox->isSelected();
    }
    else if (id == "speech")
    {
        auto val = (Being::Speech)mSpeechSlider->getValue();
        mSpeechLabel->setCaption(speechModeToString(val));
        mSpeechSlider->setValue(val);
        config.speech = val;
    }
    else if (id == "showownname")
    {
        setConfigValue(&Config::showOwnName, mNameCheckBox->isSelected());
    }
    else if (id == "lognpc")
    {
        config.logNpcInGui = mNPCLogCheckBox->isSelected();
    }
}
