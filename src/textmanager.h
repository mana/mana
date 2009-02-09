/*
 *  Support for non-overlapping floating text
 *  Copyright (C) 2008  Douglas Boffey <DougABoffey@netscape.net>
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

#ifndef TEXTMANAGER_H
#define TEXTMANAGER_H

#include <list>

#include "guichanfwd.h"

class Text;

class TextManager
{
    public:
        /**
         * Constructor
         */
        TextManager();

        /**
         * Add text to the manager
         */
        void addText(Text *text);

        /**
         * Move the text around the screen
         */
        void moveText(Text *text, int x, int y);

        /**
         * Remove the text from the manager
         */
        void removeText(const Text *text);

        /**
         * Destroy the manager
         */
        ~TextManager();

        /**
         * Draw the text
         */
        void draw(gcn::Graphics *graphics, int xOff, int yOff);

    private:
        /**
         * Position the text so as to avoid conflict
         */
        void place(const Text *textObj, const Text *omit,
        int &x, int &y, int h);

        typedef std::list<Text *> TextList; /**< The container type */
        TextList mTextList; /**< The container */
};

extern TextManager *textManager;

#endif // TEXTMANAGER_H
