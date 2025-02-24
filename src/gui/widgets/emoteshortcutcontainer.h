/*
 *  Extended support for activating emotes
 *  Copyright (C) 2009  Aethyra Development Team
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

#include "gui/widgets/shortcutcontainer.h"

/**
 * An emote shortcut container. Used to quickly use emoticons.
 *
 * \ingroup GUI
 */
class EmoteShortcutContainer : public ShortcutContainer
{
    public:
        EmoteShortcutContainer();

        /**
         * Draws the items.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Handles mouse when dragged.
         */
        void mouseDragged(gcn::MouseEvent &event) override;

        /**
         * Handles mouse when pressed.
         */
        void mousePressed(gcn::MouseEvent &event) override;

        /**
         * Handles mouse release.
         */
        void mouseReleased(gcn::MouseEvent &event) override;

    private:
        bool mEmoteClicked = false;
        int mEmoteMoved = -1;
};
