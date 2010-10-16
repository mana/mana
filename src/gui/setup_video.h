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

#ifndef GUI_SETUP_VIDEO_H
#define GUI_SETUP_VIDEO_H

#include "being.h"
#include "guichanfwd.h"

#include "gui/widgets/setuptab.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

class ModeListModel;
class FontSizeChoiceListModel;

class Setup_Video : public SetupTab, public gcn::ActionListener,
                    public gcn::KeyListener
{
    public:
        Setup_Video();
        ~Setup_Video();

        void apply();
        void cancel();

        void action(const gcn::ActionEvent &event);

        static const char *overlayDetailToString(int detail = -1);

        static const char *particleDetailToString(int detail = -1);

    private:
        bool mFullScreenEnabled;
        bool mOpenGLEnabled;
        bool mCustomCursorEnabled;
        bool mShowMonsterDamageEnabled;
        bool mVisibleNamesEnabled;
        bool mParticleEffectsEnabled;
        bool mNameEnabled;
        bool mNPCLogEnabled;
        bool mPickupChatEnabled;
        bool mPickupParticleEnabled;
        double mOpacity;
        int mFps;
        bool mLowCPUEnabled;
        Being::Speech mSpeechMode;

        ModeListModel *mModeListModel;
        FontSizeChoiceListModel *mFontSizeListModel;

        gcn::Label *speechLabel;
        gcn::Label *alphaLabel;
        gcn::Label *scrollRadiusLabel;
        gcn::Label *scrollLazinessLabel;
        gcn::Label *overlayDetailLabel;
        gcn::Label *particleDetailLabel;
        gcn::Label *fontSizeLabel;

        gcn::ListBox *mModeList;
        gcn::CheckBox *mFsCheckBox;
        gcn::CheckBox *mOpenGLCheckBox;
        gcn::CheckBox *mCustomCursorCheckBox;
        gcn::CheckBox *mShowMonsterDamageCheckBox;
        gcn::CheckBox *mVisibleNamesCheckBox;
        gcn::CheckBox *mParticleEffectsCheckBox;
        gcn::CheckBox *mNameCheckBox;
        gcn::CheckBox *mNPCLogCheckBox;

        gcn::Label *mPickupNotifyLabel;
        gcn::CheckBox *mPickupChatCheckBox;
        gcn::CheckBox *mPickupParticleCheckBox;

        gcn::Slider *mSpeechSlider;
        gcn::Label *mSpeechLabel;
        gcn::Slider *mAlphaSlider;
        gcn::CheckBox *mFpsCheckBox;
        gcn::Slider *mFpsSlider;
        gcn::Label *mFpsLabel;

        int mOverlayDetail;
        gcn::Slider *mOverlayDetailSlider;
        gcn::Label *mOverlayDetailField;

        int mParticleDetail;
        gcn::Slider *mParticleDetailSlider;
        gcn::Label *mParticleDetailField;

        int mFontSize;
        gcn::DropDown *mFontSizeDropDown;

        gcn::CheckBox *mLowCPUCheckBox;
};

#endif
