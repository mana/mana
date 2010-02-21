/*
 *  Support for non-overlapping floating text
 *  Copyright (C) 2008  Douglas Boffey <DougABoffey@netscape.net>
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef TEXT_H
#define TEXT_H

#include "graphics.h"
#include "guichanfwd.h"

#include <guichan/color.hpp>

class TextManager;

class Text
{
    friend class TextManager;

    public:
        /**
         * Constructor creates a text object to display on the screen.
         */
        Text(const std::string &text, int x, int y,
             gcn::Graphics::Alignment alignment,
             const gcn::Color *color, bool isSpeech = false);

        /**
         * Destructor. The text is removed from the screen.
         */
        virtual ~Text();

        void setColor(const gcn::Color *color);

        int getWidth() const { return mWidth; }
        int getHeight() const { return mHeight; }

        /**
         * Allows the originator of the text to specify the ideal coordinates.
         */
        void adviseXY(int x, int y);

        /**
         * Draws the text.
         */
        virtual void draw(gcn::Graphics *graphics, int xOff, int yOff);

    private:
        int mX;                /**< Actual x-value of left of text written. */
        int mY;                /**< Actual y-value of top of text written. */
        int mWidth;            /**< The width of the text. */
        int mHeight;           /**< The height of the text. */
        int mXOffset;          /**< The offset of mX from the desired x. */
        static int mInstances; /**< Instances of text. */
        std::string mText;     /**< The text to display. */
        const gcn::Color *mColor;     /**< The color of the text. */
        gcn::Font *mFont;      /**< The font of the text */
        bool mIsSpeech;        /**< Is this text a speech bubble? */

    protected:
        static ImageRect mBubble;   /**< Speech bubble graphic */
        static Image *mBubbleArrow; /**< Speech bubble arrow graphic */
};

class FlashText : public Text
{
    public:
        FlashText(const std::string &text, int x, int y,
                  gcn::Graphics::Alignment alignment,
                  const gcn::Color* color);

        /**
         * Remove the text from the screen
         */
        virtual ~FlashText() {}

        /**
         * Flash the text for so many refreshes.
         */
        void flash(int time) {mTime = time; }

        /**
         * Draws the text.
         */
        virtual void draw(gcn::Graphics *graphics, int xOff, int yOff);

    private:
        int mTime;             /**< Time left for flashing */
};

#endif // TEXT_H
