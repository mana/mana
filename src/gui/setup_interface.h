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

#pragma once

#include "being.h"
#include "guichanfwd.h"

#include "gui/widgets/setuptab.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

class FontSizeChoiceListModel;

class Setup_Interface : public SetupTab, public gcn::ActionListener,
                    public gcn::KeyListener
{
    public:
        Setup_Interface();
        ~Setup_Interface() override;

        void apply() override;
        void cancel() override;

        void action(const gcn::ActionEvent &event) override;

    private:
        bool mShowMonsterDamageEnabled;
        bool mVisibleNamesEnabled;
        bool mNameEnabled;
        bool mNPCLogEnabled;
        bool mPickupChatEnabled;
        bool mPickupParticleEnabled;
        double mOpacity;
        Being::Speech mSpeechMode;

        FontSizeChoiceListModel *mFontSizeListModel;

        gcn::Label *speechLabel;
        gcn::Label *alphaLabel;
        gcn::Label *fontSizeLabel;

        gcn::CheckBox *mShowMonsterDamageCheckBox;
        gcn::CheckBox *mVisibleNamesCheckBox;
        gcn::CheckBox *mNameCheckBox;
        gcn::CheckBox *mNPCLogCheckBox;

        gcn::Label *mPickupNotifyLabel;
        gcn::CheckBox *mPickupChatCheckBox;
        gcn::CheckBox *mPickupParticleCheckBox;

        gcn::Slider *mSpeechSlider;
        gcn::Label *mSpeechLabel;
        gcn::Slider *mAlphaSlider;

        int mFontSize;
        gcn::DropDown *mFontSizeDropDown;

};
