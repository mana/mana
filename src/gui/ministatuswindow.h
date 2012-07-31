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

#ifndef MINISTATUS_H
#define MINISTATUS_H

#include "eventlistener.h"

#include "gui/widgets/container.h"

#include <guichan/mouselistener.hpp>

#include <vector>

class AnimatedSprite;
class Graphics;
class ProgressBar;
class TextPopup;

/**
 * The player mini-status dialog.
 *
 * \ingroup Interface
 */
class MiniStatusWindow : public Container,
                         public EventListener,
                         public gcn::MouseListener
{
    public:
        MiniStatusWindow();

        void drawIcons(Graphics *graphics);

        void event(Event::Channel channel, const Event &event);

        void logic(); // Updates icons

        void draw(gcn::Graphics *graphics);

        void mouseMoved(gcn::MouseEvent &mouseEvent);
        void mouseExited(gcn::MouseEvent &event);

    private:
        bool isInBar(ProgressBar *bar, int x, int y) const;

        /**
         * Sets one of the icons.
         */
        void setIcon(int index, AnimatedSprite *sprite);

        void eraseIcon(int index);

        /*
         * Mini Status Bars
         */
        ProgressBar *mHpBar;
        ProgressBar *mMpBar;
        ProgressBar *mXpBar;
        TextPopup *mTextPopup;

        std::vector<int> mStatusEffectIcons;
        std::vector<AnimatedSprite *> mIcons;
};

#endif
