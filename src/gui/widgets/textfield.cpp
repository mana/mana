/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "gui/widgets/textfield.h"

#include "configuration.h"
#include "graphics.h"

#include "gui/palette.h"
#include "gui/sdlinput.h"
#include "gui/theme.h"

#include "resources/image.h"

#include "utils/copynpaste.h"
#include "utils/dtor.h"

#include <guichan/font.hpp>

#undef DELETE //Win32 compatibility hack

int TextField::instances = 0;
float TextField::mAlpha = 1.0;
ImageRect TextField::skin;

TextField::TextField(const std::string &text, bool loseFocusOnTab):
    gcn::TextField(text),
    mNumeric(false)
{
    setFrameSize(2);

    mLoseFocusOnTab = loseFocusOnTab;

    if (instances == 0)
    {
        // Load the skin
        Image *textbox = Theme::getImageFromTheme("deepbox.png");
        int gridx[4] = {0, 3, 28, 31};
        int gridy[4] = {0, 3, 28, 31};
        int a = 0, x, y;

        for (y = 0; y < 3; y++)
        {
            for (x = 0; x < 3; x++)
            {
                skin.grid[a] = textbox->getSubImage(
                        gridx[x], gridy[y],
                        gridx[x + 1] - gridx[x] + 1,
                        gridy[y + 1] - gridy[y] + 1);
                skin.grid[a]->setAlpha(config.getFloatValue("guialpha"));
                a++;
            }
        }

        textbox->decRef();
    }

    instances++;
}

TextField::~TextField()
{
    instances--;

    if (instances == 0)
        for_each(skin.grid, skin.grid + 9, dtor<Image*>());
}

void TextField::updateAlpha()
{
    float alpha = std::max(config.getFloatValue("guialpha"),
                           Theme::instance()->getMinimumOpacity());

    if (alpha != mAlpha)
    {
        mAlpha = alpha;
        for (int a = 0; a < 9; a++)
            skin.grid[a]->setAlpha(mAlpha);
    }
}

void TextField::draw(gcn::Graphics *graphics)
{
    updateAlpha();

    if (isFocused())
    {
        drawCaret(graphics,
                  getFont()->getWidth(mText.substr(0, mCaretPosition)) -
                  mXScroll);
    }

    graphics->setColor(Theme::getThemeColor(Theme::TEXT));
    graphics->setFont(getFont());
    graphics->drawText(mText, 1 - mXScroll, 1);
}

void TextField::drawFrame(gcn::Graphics *graphics)
{
    //updateAlpha(); -> Not useful...

    int w, h, bs;
    bs = getFrameSize();
    w = getWidth() + bs * 2;
    h = getHeight() + bs * 2;

    static_cast<Graphics*>(graphics)->drawImageRect(0, 0, w, h, skin);
}

void TextField::setNumeric(bool numeric)
{
    mNumeric = numeric;
    if (!numeric)
        return;

    const char *text = mText.c_str();
    for (const char *textPtr = text; *textPtr; ++textPtr)
    {
        if (*textPtr < '0' || *textPtr > '9')
        {
            setText(mText.substr(0, textPtr - text));
            return;
        }
    }
}

int TextField::getValue() const
{
    if (!mNumeric)
        return 0;

    int value = atoi(mText.c_str());
    if (value < mMinimum)
        return mMinimum;

    if (value > mMaximum)
        return mMaximum;

    return value;
}

void TextField::keyPressed(gcn::KeyEvent &keyEvent)
{
    int val = keyEvent.getKey().getValue();

    if (val >= 32)
    {
        int l;
        if (val < 128) l = 1;            // 0xxxxxxx
        else if (val < 0x800) l = 2;     // 110xxxxx 10xxxxxx
        else if (val < 0x10000) l = 3;   // 1110xxxx 10xxxxxx 10xxxxxx
        else l = 4;                      // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

        char buf[4];
        for (int i = 0; i < l; ++i)
        {
            buf[i] = val >> (6 * (l - i - 1));
            if (i > 0) buf[i] = (buf[i] & 63) | 128;
        }

        if (l > 1) buf[0] |= 255 << (8 - l);

        mText.insert(mCaretPosition, std::string(buf, buf + l));
        mCaretPosition += l;
    }

    /* In UTF-8, 10xxxxxx is only used for inner parts of characters. So skip
       them when processing key presses. */

    switch (val)
    {
        case Key::LEFT:
        {
            while (mCaretPosition > 0)
            {
                --mCaretPosition;
                if ((mText[mCaretPosition] & 192) != 128)
                    break;
            }
        } break;

        case Key::RIGHT:
        {
            unsigned sz = mText.size();
            while (mCaretPosition < sz)
            {
                ++mCaretPosition;
                if (mCaretPosition == sz ||
                    (mText[mCaretPosition] & 192) != 128)
                    break;
            }
        } break;

        case Key::DELETE:
        {
            unsigned sz = mText.size();
            while (mCaretPosition < sz)
            {
                --sz;
                mText.erase(mCaretPosition, 1);
                if (mCaretPosition == sz ||
                    (mText[mCaretPosition] & 192) != 128)
                    break;
            }
        } break;

        case Key::BACKSPACE:
        {
            while (mCaretPosition > 0)
            {
                --mCaretPosition;
                int v = mText[mCaretPosition];
                mText.erase(mCaretPosition, 1);
                if ((v & 192) != 128) break;
            }
        } break;

        case Key::ENTER:
            distributeActionEvent();
            break;

        case Key::HOME:
            mCaretPosition = 0;
            break;

        case Key::END:
            mCaretPosition = mText.size();
            break;

        case Key::TAB:
            if (mLoseFocusOnTab)
                return;
            break;

        case 22: // Control code 22, SYNCHRONOUS IDLE, sent on Ctrl+v
            handlePaste();
            break;
    }

    keyEvent.consume();
    fixScroll();
}

void TextField::handlePaste()
{
    std::string text = getText();
    std::string::size_type caretPos = getCaretPosition();

    if (RetrieveBuffer(text, caretPos)) {
        setText(text);
        setCaretPosition(caretPos);
    }
}
