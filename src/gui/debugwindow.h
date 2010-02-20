/*
 *  The Mana Client
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include "gui/widgets/window.h"

class Label;

/**
 * The debug window.
 *
 * \ingroup Interface
 */
class DebugWindow : public Window
{
    public:
        /**
         * Constructor.
         */
        DebugWindow();

        /**
         * Logic (updates components' size and infos)
         */
        void logic();

    private:
        Label *mMusicFileLabel, *mMapLabel, *mMinimapLabel;
        Label *mTileMouseLabel, *mFPSLabel;
        Label *mParticleCountLabel, *mParticleDetailLabel;
        Label *mAmbientDetailLabel;

        std::string mFPSText;
};

extern DebugWindow *debugWindow;

#endif
