/*
 *  The Mana World
 *  Copyright (C) 2007  The Mana World Development Team
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

#ifndef SHORTCUTCONTAINER_H
#define SHORTCUTCONTAINER_H

#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>

class Image;

/**
 * A generic shortcut container.
 *
 * \ingroup GUI
 */
class ShortcutContainer : public gcn::Widget,
                          public gcn::WidgetListener,
                          public gcn::MouseListener
{
    public:
        /**
         * Constructor. Initializes the shortcut container.
         */
        ShortcutContainer();

        /**
         * Destructor.
         */
        ~ShortcutContainer() {}

        /**
         * Draws the shortcuts
         */
        virtual void draw(gcn::Graphics *graphics) = 0;

        /**
         * Invoked when a widget changes its size. This is used to determine
         * the new height of the container.
         */
        virtual void widgetResized(const gcn::Event &event);

        /**
         * Handles mouse when dragged.
         */
        virtual void mouseDragged(gcn::MouseEvent &event) = 0;

        /**
         * Handles mouse when pressed.
         */
        virtual void mousePressed(gcn::MouseEvent &event) = 0;

        /**
         * Handles mouse release.
         */
        virtual void mouseReleased(gcn::MouseEvent &event) = 0;

        int getMaxItems() const
        { return mMaxItems; }

        int getBoxWidth() const
        { return mBoxWidth; }

        int getBoxHeight() const
        { return mBoxHeight; }

    protected:
        /**
         * Gets the index from the grid provided the point is in an item box.
         *
         * @param pointX X coordinate of the point.
         * @param pointY Y coordinate of the point.
         * @return index on success, -1 on failure.
         */
        int getIndexFromGrid(int pointX, int pointY) const;

        Image *mBackgroundImg;

        static float mAlpha;

        int mMaxItems;
        int mBoxWidth;
        int mBoxHeight;
        int mCursorPosX, mCursorPosY;
        int mGridWidth, mGridHeight;
};

#endif
