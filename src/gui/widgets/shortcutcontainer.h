/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
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
        ShortcutContainer();

        /**
         * Draws the shortcuts
         */
        void draw(gcn::Graphics *graphics) override = 0;

        /**
         * Invoked when a widget changes its size. This is used to determine
         * the new height of the container.
         */
        void widgetResized(const gcn::Event &event) override;

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

        int mMaxItems = 0;
        int mBoxWidth = 0;
        int mBoxHeight = 0;
        int mCursorPosX = 0;
        int mCursorPosY = 0;
        int mGridWidth = 1;
        int mGridHeight = 1;
};

#endif
