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

#include "guichanfwd.h"

#include "gui/widgets/setuptab.h"
#include "video.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

class ResolutionListModel;
class ScaleListModel;

class Setup_Video : public SetupTab, public gcn::ActionListener,
                    public gcn::KeyListener
{
    public:
        Setup_Video();
        ~Setup_Video() override;

        void apply() override;
        void cancel() override;

        void action(const gcn::ActionEvent &event) override;

    private:
        void refreshScaleList();

        VideoSettings mVideoSettings;
        bool mCustomCursorEnabled;
        bool mParticleEffectsEnabled;
        int mFps;
        bool mSDLTransparencyDisabled;

        std::unique_ptr<gcn::ListModel> mWindowModeListModel;
        std::unique_ptr<ResolutionListModel> mResolutionListModel;
        std::unique_ptr<ScaleListModel> mScaleListModel;

        gcn::DropDown *mWindowModeDropDown;
        gcn::DropDown *mResolutionDropDown;
        gcn::DropDown *mScaleDropDown;
        gcn::CheckBox *mVSyncCheckBox;
        gcn::CheckBox *mOpenGLCheckBox;
        gcn::CheckBox *mCustomCursorCheckBox;
        gcn::CheckBox *mParticleEffectsCheckBox;

        gcn::CheckBox *mFpsCheckBox;
        gcn::Slider *mFpsSlider;
        gcn::Label *mFpsLabel;

        int mOverlayDetail;
        gcn::Slider *mOverlayDetailSlider;
        gcn::Label *mOverlayDetailField;

        int mParticleDetail;
        gcn::Slider *mParticleDetailSlider;
        gcn::Label *mParticleDetailField;

        gcn::CheckBox *mDisableSDLTransparencyCheckBox;
};
