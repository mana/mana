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

#include "gui/truetypefont.h"

#include "graphics.h"

#include "resources/image.h"

#include "utils/stringutils.h"

#include <guichan/color.hpp>
#include <guichan/exception.hpp>

const unsigned int CACHE_SIZE = 256;

class TextChunk
{
    public:
        TextChunk(const std::string &text, const gcn::Color &color) :
            text(text), color(color)
        {
        }

        ~TextChunk()
        {
            delete img;
        }

        bool operator==(const TextChunk &chunk) const
        {
            return (chunk.text == text && chunk.color == color);
        }

        void generate(TTF_Font *font)
        {
            SDL_Color sdlCol;
            sdlCol.r = color.r;
            sdlCol.g = color.g;
            sdlCol.b = color.b;
            sdlCol.a = color.a;

            const char *str = getSafeUtf8String(text);
            SDL_Surface *surface = TTF_RenderUTF8_Blended(
                    font, str, sdlCol);
            delete[] str;

            if (!surface)
            {
                img = nullptr;
                return;
            }

            img = Image::load(surface);

            SDL_FreeSurface(surface);
        }

        Image *img = nullptr;
        std::string text;
        gcn::Color color;
};

using CacheIterator = std::list<TextChunk>::iterator;

static int fontCounter;

TrueTypeFont::TrueTypeFont(const std::string &filename, int size, int style)
{
    if (fontCounter == 0 && TTF_Init() == -1)
    {
        throw GCN_EXCEPTION("Unable to initialize SDL_ttf: " +
            std::string(TTF_GetError()));
    }

    ++fontCounter;
    mFont = TTF_OpenFont(filename.c_str(), size);

    if (!mFont)
    {
        throw GCN_EXCEPTION("SDLTrueTypeFont::SDLTrueTypeFont: " +
            std::string(TTF_GetError()));
    }

    TTF_SetFontStyle(mFont, style);
}

TrueTypeFont::~TrueTypeFont()
{
    TTF_CloseFont(mFont);
    --fontCounter;

    if (fontCounter == 0)
        TTF_Quit();
}

void TrueTypeFont::drawString(gcn::Graphics *graphics,
                              const std::string &text,
                              int x, int y)
{
    if (text.empty())
        return;

    auto *g = dynamic_cast<Graphics *>(graphics);

    if (!g)
        throw "Not a valid graphics object!";

    gcn::Color col = g->getColor();
    const float alpha = col.a / 255.0f;

    /* The alpha value is ignored at string generation so avoid caching the
     * same text with different alpha values.
     */
    col.a = 255;

    TextChunk chunk(text, col);

    bool found = false;

    for (auto i = mCache.begin(); i != mCache.end(); ++i)
    {
        if (chunk == (*i))
        {
            // Raise priority: move it to front
            mCache.splice(mCache.begin(), mCache, i);
            found = true;
            break;
        }
    }

    // Surface not found
    if (!found)
    {
        if (mCache.size() >= CACHE_SIZE)
            mCache.pop_back();
        mCache.push_front(chunk);
        mCache.front().generate(mFont);
    }

    if (mCache.front().img)
    {
        mCache.front().img->setAlpha(alpha);
        g->drawImage(mCache.front().img, x, y);
    }
}

int TrueTypeFont::getWidth(const std::string &text) const
{
    for (auto i = mCache.begin(); i != mCache.end(); i++)
    {
        if (i->text == text)
        {
            // Raise priority: move it to front
            // Assumption is that TTF::draw will be called next
            mCache.splice(mCache.begin(), mCache, i);
            if (i->img)
                return i->img->getWidth();
            else
                return 0;
        }
    }

    int w, h;
    const char *str = getSafeUtf8String(text);
    TTF_SizeUTF8(mFont, str, &w, &h);
    delete[] str;
    return w;
}

int TrueTypeFont::getHeight() const
{
    return TTF_FontHeight(mFont);
}

int TrueTypeFont::getLineHeight() const
{
    return TTF_FontLineSkip(mFont);
}
