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

#ifndef __TMW_PLAYERBOX_H__
#define __TMW_PLAYERBOX_H__

#include <guichan/widgets/scrollarea.hpp>

class ImageRect;

/**
 * A box showing a player. Draws the various hair styles a player can have
 * currently.
 *
 * \ingroup GUI
 */
class PlayerBox : public gcn::ScrollArea
{
    public:
        /**
         * Constructor.
         */
        PlayerBox(unsigned char sex);

        /**
         * Destructor.
         */
        ~PlayerBox();

        /**
         * Draws the scroll area.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Draws the background and border of the scroll area.
         */
        void drawBorder(gcn::Graphics *graphics);

        int mHairColor;         /**< The hair color index */
        int mHairStyle;         /**< The hair style index */
        unsigned char mSex;     /**< Sex */
        bool mShowPlayer;       /**< Wether to show the player or not */

    private:
        static int instances;
        static ImageRect background;
};

#endif
