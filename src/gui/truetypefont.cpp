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

    void render(Graphics *graphics,
                int x, int y,
                TTF_Font *font,
                std::unique_ptr<Image> &img,
                float scale);

    const std::string text;
    std::unique_ptr<Image> regular;
    std::unique_ptr<Image> outlined;
};

void TextChunk::render(Graphics *graphics,
                       int x, int y,
                       TTF_Font *font,
                       std::unique_ptr<Image> &img,
                       float scale)
{
    if (!img)
    {
        // Always render in white, we'll use color modulation when rendering
        constexpr SDL_Color white = { 255, 255, 255, 255 };
        SDL_Surface *surface = TTF_RenderUTF8_Blended(font,
                                                      getSafeUtf8String(text),
                                                      white);

        if (surface)
        {
            img.reset(Image::load(surface));
            SDL_FreeSurface(surface);
        }
    }

    if (img)
    {
        graphics->drawRescaledImageF(img.get(), 0, 0, x, y,
                                     img->getWidth(),
                                     img->getHeight(),
                                     img->getWidth() / scale,
                                     img->getHeight() / scale, true);

    }
}


std::list<TrueTypeFont*> TrueTypeFont::mFonts;
float TrueTypeFont::mScale = 1.0f;

TrueTypeFont::TrueTypeFont(const std::string &filename, int size, int style)
    : mFilename(filename)
    , mPointSize(size)
    , mStyle(style)
{
    if (TTF_Init() == -1)
    {
        throw GCN_EXCEPTION("Unable to initialize SDL_ttf: " +
            std::string(TTF_GetError()));
    }

    mFont = TTF_OpenFont(filename.c_str(), size * mScale);
    mFontOutline = TTF_OpenFont(filename.c_str(), size * mScale);

    if (!mFont || !mFontOutline)
    {
        throw GCN_EXCEPTION("SDLTrueTypeFont::SDLTrueTypeFont: " +
            std::string(TTF_GetError()));
    }

    TTF_SetFontStyle(mFont, style);
    TTF_SetFontStyle(mFontOutline, style);

    TTF_SetFontOutline(mFontOutline, static_cast<int>(mScale));

    mFonts.push_back(this);
}

TrueTypeFont::~TrueTypeFont()
{
    mFonts.remove(this);

    if (mFont)
        TTF_CloseFont(mFont);

    if (mFontOutline)
        TTF_CloseFont(mFontOutline);

    TTF_Quit();
}

void TrueTypeFont::drawString(gcn::Graphics *graphics,
                              const std::string &text,
                              int x, int y)
{
    if (text.empty())
        return;

    auto *g = static_cast<Graphics *>(graphics);
    TextChunk &chunk = getChunk(text);

    chunk.render(g, x, y, mFont, chunk.regular, mScale);
}

void TrueTypeFont::drawString(Graphics *graphics,
                              const std::string &text,
                              int x, int y,
                              const std::optional<gcn::Color> &outlineColor,
                              const std::optional<gcn::Color> &shadowColor)
{
    if (text.empty())
        return;

    auto *g = static_cast<Graphics *>(graphics);
    auto color = graphics->getColor();
    TextChunk &chunk = getChunk(text);

    if (shadowColor)
    {
        g->setColor(*shadowColor);
        if (outlineColor)
            chunk.render(g, x, y, mFontOutline, chunk.outlined, mScale);
        else
            chunk.render(g, x + 1, y + 1, mFont, chunk.regular, mScale);
    }

    if (outlineColor)
    {
        g->setColor(*outlineColor);
        chunk.render(g, x - 1, y - 1, mFontOutline, chunk.outlined, mScale);
    }

    g->setColor(color);
    chunk.render(g, x, y, mFont, chunk.regular, mScale);
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
        TTF_SetFontSize(font->mFontOutline, font->mPointSize * mScale);
        TTF_SetFontOutline(font->mFontOutline, mScale);
#else
        TTF_CloseFont(font->mFont);
        TTF_CloseFont(font->mFontOutline);
        font->mFont = TTF_OpenFont(font->mFilename.c_str(), font->mPointSize * mScale);
        font->mFontOutline = TTF_OpenFont(font->mFilename.c_str(), font->mPointSize * mScale);
        TTF_SetFontStyle(font->mFont, font->mStyle);
        TTF_SetFontStyle(font->mFontOutline, font->mStyle);
        TTF_SetFontOutline(font->mFontOutline, mScale);
#endif

        font->mCache.clear();
    }
}

int TrueTypeFont::getWidth(const std::string &text) const
{
    TextChunk &chunk = getChunk(text);
    if (auto img = chunk.regular.get())
        return std::ceil(img->getWidth() / mScale);

    // If the image wasn't created yet, just calculate the width of the text
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

TextChunk &TrueTypeFont::getChunk(const std::string &text) const
{
    for (auto i = mCache.begin(); i != mCache.end(); i++)
    {
        if (i->text == text)
        {
            // Raise priority: move it to front
            mCache.splice(mCache.begin(), mCache, i);
            return *i;
        }
    }

    if (mCache.size() >= CACHE_SIZE)
        mCache.pop_back();

    return mCache.emplace_front(text);
}
