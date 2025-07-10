/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *  Copyright (C) 2009  Aethyra Development Team
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

#include <guichan/color.hpp>
#include <guichan/font.hpp>

#include <SDL_ttf.h>

#include <list>
#include <optional>
#include <string>

class Graphics;
class TextChunk;

/**
 * A wrapper around SDL_ttf for allowing the use of TrueType fonts.
 *
 * <b>NOTE:</b> This class initializes SDL_ttf as necessary.
 */
class TrueTypeFont : public gcn::Font
{
    public:
        /**
         * Constructor.
         *
         * @param filename  Font filename.
         * @param size      Font size.
         */
        TrueTypeFont(const std::string &filename, int size, int style = 0);
        ~TrueTypeFont() override;

        const std::string &filename() const { return mFilename; }
        int pointSize() const { return mPointSize; }
        int style() const { return mStyle; }

        int getWidth(const std::string &text) const override;
        int getHeight() const override;

        /**
         * Returns the height of a line of text. This is not the visual height
         * as returned by getHeight() but the recommended spacing between lines
         * of text.
         */
        int getLineHeight() const;

        /**
         * @see Font::drawString
         */
        void drawString(gcn::Graphics *graphics,
                        const std::string &text,
                        int x, int y) override;

        /**
         * Extended version of drawString that allows for rendering text with
         * outline and/or shadow.
         */
        void drawString(Graphics *graphics,
                        const std::string &text,
                        int x, int y,
                        const std::optional<gcn::Color> &outlineColor,
                        const std::optional<gcn::Color> &shadowColor);

        static void updateFontScale(float scale);

    private:
        TextChunk &getChunk(const std::string &text) const;

        const std::string mFilename;
        TTF_Font *mFont = nullptr;
        TTF_Font *mFontOutline = nullptr;

        const int mPointSize;
        const int mStyle;

        // Word surfaces cache
        mutable std::list<TextChunk> mCache;

        static std::list<TrueTypeFont*> mFonts;
        static float mScale;
};
