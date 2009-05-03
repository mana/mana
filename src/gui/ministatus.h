/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MINISTATUS_H
#define MINISTATUS_H

#include "gui/widgets/popup.h"

#include <vector>

class AnimatedSprite;
class Graphics;
class ProgressBar;

/**
 * The player mini-status dialog.
 *
 * \ingroup Interface
 */
class MiniStatusWindow : public Popup
{
    public:
        MiniStatusWindow();

        void draw(gcn::Graphics *graphics);

        /**
         * Sets one of the icons.
         */
        void setIcon(int index, AnimatedSprite *sprite);

        void eraseIcon(int index);

        void drawIcons(Graphics *graphics);

    private:
        /**
         * Updates this dialog with values from player_node.
         */
        void update();

        /*
         * Mini Status Bars
         */
        ProgressBar *mHpBar;
#ifdef EATHENA_SUPPORT
        ProgressBar *mMpBar;
        ProgressBar *mXpBar;
#endif

        std::vector<AnimatedSprite *> mIcons;
};

#endif
