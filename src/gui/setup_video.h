/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#ifndef _TMW_GUI_SETUP_VIDEO_H
#define _TMW_GUI_SETUP_VIDEO_H

#include "setuptab.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

#include "../guichanfwd.h"

class Setup_Video : public SetupTab, public gcn::ActionListener,
                    public gcn::KeyListener
{
    public:
        Setup_Video();
        ~Setup_Video();

        void apply();
        void cancel();

        void action(const gcn::ActionEvent &event);

        /** Called when key is pressed */
        void
        keyPressed(gcn::KeyEvent &event);

    private:
        bool mFullScreenEnabled;
        bool mOpenGLEnabled;
        bool mCustomCursorEnabled;
        double mOpacity;
        int mFps;

        class ModeListModel *mModeListModel;

        gcn::ListBox *mModeList;
        gcn::CheckBox *mFsCheckBox;
        gcn::CheckBox *mOpenGLCheckBox;
        gcn::CheckBox *mCustomCursorCheckBox;

        gcn::Slider *mAlphaSlider;
        gcn::CheckBox *mFpsCheckBox;
        gcn::Slider *mFpsSlider;
        gcn::TextField *mFpsField;

        int mOriginalScrollLaziness;
        gcn::Slider *mScrollLazinessSlider;
        gcn::TextField *mScrollLazinessField;

        int mOriginalScrollRadius;
        gcn::Slider *mScrollRadiusSlider;
        gcn::TextField *mScrollRadiusField;

        int mOverlayDetail;
        gcn::Slider *mOverlayDetailSlider;
        gcn::Label *mOverlayDetailField;

        void
        updateSliders(bool originalValues);

        int
        updateSlider(gcn::Slider *slider, gcn::TextField *field,
                     const std::string &configName);
};

#endif
