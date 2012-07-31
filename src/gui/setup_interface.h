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

#ifndef GUI_SETUP_INTERFACE_H
#define GUI_SETUP_INTERFACE_H

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
        ~Setup_Interface();

        void apply();
        void cancel();

        void action(const gcn::ActionEvent &event);

    private:
        bool mShowMonsterDamageEnabled;
        bool mVisibleNamesEnabled;
        bool mNameEnabled;
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

        gcn::Label *mPickupNotifyLabel;
        gcn::CheckBox *mPickupChatCheckBox;
        gcn::CheckBox *mPickupParticleCheckBox;

        gcn::Slider *mSpeechSlider;
        gcn::Label *mSpeechLabel;
        gcn::Slider *mAlphaSlider;

        int mFontSize;
        gcn::DropDown *mFontSizeDropDown;

};

#endif
