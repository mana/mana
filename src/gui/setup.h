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

#ifndef _TMW_SETUP_H
#define _TMW_SETUP_H

#include <vector>

#include <guichan/actionlistener.hpp>
#include <guichan/listmodel.hpp>

#include "window.h"

#include "../guichanfwd.h"

/**
 * The setup dialog.
 *
 * \ingroup GUI
 */
class Setup : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        Setup();

        /**
         * Destructor.
         */
        ~Setup();

        /**
         * Event handling method.
         */
        void
        action(const std::string& eventId);

    private:
        class ModeListModel *mModeListModel;

        // Dialog widgets
        gcn::ListBox *mModeList;
        gcn::CheckBox *mFsCheckBox;
        gcn::CheckBox *mOpenGLCheckBox;
        gcn::CheckBox *mSoundCheckBox;
        gcn::CheckBox *mCustomCursorCheckBox;
        gcn::Slider *mAlphaSlider;
        gcn::Slider *mSfxSlider, *mMusicSlider;
        gcn::Label *mCalibrateLabel;
        gcn::Button *mCalibrateButton;

        // Variables that keeps old settings until the user "apply" them...
        int mMusicVolume, mSfxVolume;
        double mOpacity;
        bool mFullScreenEnabled;
        bool mOpenGLEnabled;
        bool mCustomCursorEnabled;
        bool mSoundEnabled;
};

#endif
