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

#ifndef __TMW_SCROLLAREA_H__
#define __TMW_SCROLLAREA_H__

#include <guichan.hpp>
#include "../graphics.h"
#include "../configuration.h"

/**
 * A scroll area.
 *
 * \ingroup GUI
 */
class ScrollArea : public gcn::ScrollArea {
    public:
        /**
         * Constructor.
         */
        ScrollArea();

        /**
         * Constructor.
         */
        ScrollArea(gcn::Widget *content);

        /**
         * Draws the scroll area.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Draws the background and border of the scroll area.
         */
        void drawBorder(gcn::Graphics *graphics);

    protected:
        /**
         * Initializes the scroll area.
         */
        void init();

        void drawUpButton(gcn::Graphics *graphics);
        void drawDownButton(gcn::Graphics *graphics);
        void drawLeftButton(gcn::Graphics *graphics);
        void drawRightButton(gcn::Graphics *graphics);
        void drawVBar(gcn::Graphics *graphics);
        void drawHBar(gcn::Graphics *graphics);
        void drawVMarker(gcn::Graphics *graphics);
        void drawHMarker(gcn::Graphics *graphics);

        ImageRect background;
        ImageRect vMarker;
	float guiAlpha;
};

#endif
