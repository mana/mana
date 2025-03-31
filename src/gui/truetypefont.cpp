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

#include <guichan/color.hpp>
#include <guichan/exception.hpp>

#include <cmath>
#include <memory>

const unsigned int CACHE_SIZE = 256;

static const char *getSafeUtf8String(const std::string &text)
{
    static int UTF8_MAX_SIZE = 10;

    static char buf[4096];
    const int len = std::min(text.size(), sizeof(buf) - UTF8_MAX_SIZE);
    memcpy(buf, text.c_str(), len);
    memset(buf + len, 0, UTF8_MAX_SIZE);
    return buf;
}

bool operator==(SDL_Color lhs, SDL_Color rhs)
{
    return (lhs.r == rhs.r &&
            lhs.g == rhs.g &&
            lhs.b == rhs.b &&
            lhs.a == rhs.a);
}

class TextChunk
{
public:
    TextChunk(const std::string &text)
        : text(text)
    {}

    void generate(TTF_Font *font)
    {
        // Always render in white, we'll use color modulation when rendering
        SDL_Surface *surface = TTF_RenderUTF8_Blended(font,
                                                      getSafeUtf8String(text),
                                                      SDL_Color { 255, 255, 255, 255 });

        if (!surface)
            return;

        img.reset(Image::load(surface));

        SDL_FreeSurface(surface);
    }

    std::unique_ptr<Image> img;
    const std::string text;
};

std::list<TrueTypeFont*> TrueTypeFont::mFonts;
float TrueTypeFont::mScale = 1.0f;

TrueTypeFont::TrueTypeFont(const std::string &filename, int size, int style)
    : mFilename(filename)
    , mPointSize(size)
{
    if (TTF_Init() == -1)
    {
        throw GCN_EXCEPTION("Unable to initialize SDL_ttf: " +
            std::string(TTF_GetError()));
    }

    mFont = TTF_OpenFont(filename.c_str(), size * mScale);

    if (!mFont)
    {
        throw GCN_EXCEPTION("SDLTrueTypeFont::SDLTrueTypeFont: " +
            std::string(TTF_GetError()));
    }

    TTF_SetFontStyle(mFont, style);

    mFonts.push_back(this);
}

TrueTypeFont::~TrueTypeFont()
{
    mFonts.remove(this);

    if (mFont)
        TTF_CloseFont(mFont);

    TTF_Quit();
}

void TrueTypeFont::drawString(gcn::Graphics *graphics,
                              const std::string &text,
                              int x, int y)
{
    if (text.empty())
        return;

    auto *g = static_cast<Graphics *>(graphics);

    bool found = false;

    for (auto i = mCache.begin(); i != mCache.end(); ++i)
    {
        auto &chunk = *i;
        if (chunk.text == text)
        {
            // Raise priority: move it to front
            mCache.splice(mCache.begin(), mCache, i);
            found = true;
            break;
        }
    }

    if (!found)
    {
        if (mCache.size() >= CACHE_SIZE)
            mCache.pop_back();
        mCache.emplace_front(text);
        mCache.front().generate(mFont);
    }

    if (auto img = mCache.front().img.get())
    {
        g->drawRescaledImageF(img, 0, 0, x, y,
                              img->getWidth(),
                              img->getHeight(),
                              img->getWidth() / mScale,
                              img->getHeight() / mScale, true);
    }
}

void TrueTypeFont::updateFontScale(float scale)
{
    if (mScale == scale)
        return;

    mScale = scale;

    for (auto font : mFonts)
    {
#if SDL_TTF_VERSION_ATLEAST(2, 0, 18)
        TTF_SetFontSize(font->mFont, font->mPointSize * mScale);
#else
        TTF_CloseFont(font->mFont);
        font->mFont = TTF_OpenFont(font->mFilename.c_str(), font->mPointSize * mScale);
#endif

        font->mCache.clear();
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
                return std::ceil(i->img->getWidth() / mScale);
            return 0;
        }
    }

    int w, h;
    TTF_SizeUTF8(mFont, getSafeUtf8String(text), &w, &h);
    return std::ceil(w / mScale);
}

int TrueTypeFont::getHeight() const
{
    return std::ceil(TTF_FontHeight(mFont) / mScale);
}

int TrueTypeFont::getLineHeight() const
{
    return std::ceil(TTF_FontLineSkip(mFont) / mScale);
}
