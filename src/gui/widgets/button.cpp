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

#include "gui/widgets/button.h"

#include "configuration.h"
#include "graphics.h"

#include "gui/palette.h"
#include "gui/textpopup.h"

#include "resources/image.h"
#include "resources/theme.h"

#include "utils/dtor.h"

#include <guichan/exception.hpp>
#include <guichan/font.hpp>

int Button::mInstances = 0;
float Button::mAlpha = 1.0;
ImageRect* Button::mButton;
TextPopup* Button::mTextPopup = 0;

enum{
    BUTTON_STANDARD,    // 0
    BUTTON_HIGHLIGHTED, // 1
    BUTTON_PRESSED,     // 2
    BUTTON_DISABLED,    // 3
    BUTTON_COUNT        // 4 - Must be last.
};

struct ButtonData
{
    char const *file;
    int gridX;
    int gridY;
};

static ButtonData const data[BUTTON_COUNT] = {
    { "button.png", 0, 0 },
    { "buttonhi.png", 9, 4 },
    { "buttonpress.png", 16, 19 },
    { "button_disabled.png", 25, 23 }
};

Button::Button():
    mButtonIcon(0)
{
    init();
    adjustSize();
}

Button::Button(const std::string &caption, const std::string &actionEventId,
    gcn::ActionListener *listener):
    gcn::Button(caption),
    mButtonIcon(0)
{
    init();
    setActionEventId(actionEventId);

    if (listener)
        addActionListener(listener);

    adjustSize();
}

void Button::setButtonIcon(const std::string& iconFile, int frameHeight,
                           int frameWidth)
{
    // We clean up possible older references.
    if (mButtonIcon)
        removeButtonIcon();

    // If nothing relevant was set, we can quit now.
    if (iconFile.empty() || !frameWidth || !frameHeight)
        return;

    // Load the icon frames.
    Image *btnIcons = Theme::getImageFromTheme(iconFile);
    if (!btnIcons)
        return;

    if (btnIcons->getWidth() > 0 && btnIcons->getHeight() > 0)
    {
        mButtonIcon = new Image*[BUTTON_COUNT];
        for (int mode = 0; mode < BUTTON_COUNT; ++mode)
        {
            mButtonIcon[mode] = btnIcons->getSubImage(mode * frameWidth, 0,
                                                      frameWidth, frameHeight);
        }

        adjustSize();
    }

    btnIcons->decRef();
}

void Button::removeButtonIcon()
{
    if (!mButtonIcon)
        return;

    // Delete potential button icons
    for (int mode = 0; mode < BUTTON_COUNT; ++mode)
    {
        delete mButtonIcon[mode];
        mButtonIcon[mode] = 0;
    }
    delete[] mButtonIcon;
    mButtonIcon = 0;

    adjustSize();
}

void Button::init()
{
    setFrameSize(0);

    if (mInstances == 0)
    {
        // Load the skin
        Image *btn[BUTTON_COUNT];
        mButton = new ImageRect[BUTTON_COUNT];

        int a, x, y, mode;

        for (mode = 0; mode < BUTTON_COUNT; ++mode)
        {
            btn[mode] = Theme::getImageFromTheme(data[mode].file);
            a = 0;
            for (y = 0; y < 3; y++)
            {
                for (x = 0; x < 3; x++)
                {
                    mButton[mode].grid[a] = btn[mode]->getSubImage(
                            data[x].gridX, data[y].gridY,
                            data[x + 1].gridX - data[x].gridX + 1,
                            data[y + 1].gridY - data[y].gridY + 1);
                    a++;
                }
            }
            btn[mode]->decRef();
        }
        updateAlpha();

        // Load the popup
        if (!mTextPopup)
            mTextPopup = new TextPopup();
    }
    mInstances++;
}

Button::~Button()
{
    mInstances--;

    if (mInstances == 0)
    {
        for (int mode = 0; mode < BUTTON_COUNT; ++mode)
        {
            for_each(mButton[mode].grid, mButton[mode].grid + 9,
                dtor<Image*>());
        }
        delete[] mButton;

        // Remove the popup
        delete mTextPopup;
    }
    removeButtonIcon();
}

void Button::updateAlpha()
{
    float alpha = std::max(config.getFloatValue("guialpha"),
                           Theme::instance()->getMinimumOpacity());

    if (mAlpha != alpha)
    {
        mAlpha = alpha;
        for (int mode = 0; mode < BUTTON_COUNT; ++mode)
        {
            for (int a = 0; a < 9; ++a)
                mButton[mode].grid[a]->setAlpha(mAlpha);
        }
    }
}

void Button::draw(gcn::Graphics *graphics)
{
    int mode;

    if (!isEnabled())
        mode = BUTTON_DISABLED;
    else if (isPressed())
        mode = BUTTON_PRESSED;
    else if (mHasMouse || isFocused())
        mode = BUTTON_HIGHLIGHTED;
    else
        mode = BUTTON_STANDARD;

    updateAlpha();

    static_cast<Graphics*>(graphics)->
        drawImageRect(0, 0, getWidth(), getHeight(), mButton[mode]);

    if (mode == BUTTON_DISABLED)
        graphics->setColor(Theme::getThemeColor(Theme::BUTTON_DISABLED));
    else
        graphics->setColor(Theme::getThemeColor(Theme::BUTTON));

    int textX = 0;
    int textY = getHeight() / 2 - getFont()->getHeight() / 2;
    int btnIconX = 0;
    int btnIconY = getHeight() / 2
                   - ((mButtonIcon && mButtonIcon[mode]) ?
                      mButtonIcon[mode]->getHeight() / 2 : 0);

    int btnIconWidth = (mButtonIcon && mButtonIcon[mode]) ?
                           mButtonIcon[mode]->getWidth() : 0;

    switch (getAlignment())
    {
        case gcn::Graphics::LEFT:
            if (btnIconWidth)
            {
                btnIconX = 4;
                textX = btnIconX + mButtonIcon[mode]->getWidth() + 2;
            }
            else
            {
                textX = 4;
            }
            break;
        case gcn::Graphics::CENTER:
            if (btnIconWidth)
            {
                btnIconX = getWidth() / 2 - (getFont()->getWidth(mCaption)
                    + mButtonIcon[mode]->getWidth() + 2) / 2;
                textX = getWidth() / 2 + mButtonIcon[mode]->getWidth() / 2 + 2;
            }
            else
            {
                textX = getWidth() / 2;
            }
            break;
        case gcn::Graphics::RIGHT:
            if (btnIconWidth)
                btnIconX = getWidth() - 4 - getFont()->getWidth(mCaption) - 2;
            textX = getWidth() - 4;
            break;
        default:
            throw GCN_EXCEPTION("Button::draw(). Unknown alignment.");
    }

    graphics->setFont(getFont());

    if (isPressed())
    {
        textX++; textY++;
        btnIconX++; btnIconY++;
    }

    if (btnIconWidth)
        static_cast<Graphics*>(graphics)->drawImage(mButtonIcon[mode],
                                                    btnIconX, btnIconY);
    graphics->drawText(getCaption(), textX, textY, getAlignment());
}

void Button::adjustSize()
{
    // Size of the image button.
    int iconWidth = 0, iconHeight = 0;
    if (mButtonIcon)
    {
        for (int mode = 0; mode < BUTTON_COUNT; ++mode)
        {
            iconWidth = std::max(iconWidth, mButtonIcon[mode] ?
                            mButtonIcon[mode]->getWidth() + 2 : 0);
            iconHeight = std::max(iconHeight, mButtonIcon[mode] ?
                             mButtonIcon[mode]->getHeight() : 0);
        }
    }

    setWidth(std::max(getFont()->getWidth(mCaption) + iconWidth + 2, iconWidth)
             + 2 * mSpacing);
    setHeight(std::max(getFont()->getHeight(), iconHeight) + 2 * mSpacing);
}

void Button::setCaption(const std::string& caption)
{
    mCaption = caption;
    adjustSize();
}

void Button::logic()
{
    gcn::Button::logic();
    mTextPopup->logic();
}

void Button::mouseMoved(gcn::MouseEvent &event)
{
    gcn::Button::mouseMoved(event);
    mTextPopup->mouseMoved(event);

    int x = event.getX();
    int y = event.getY();

    if (event.getSource() == this && !mPopupText.empty())
    {
        if (mParent)
        {
            x += mParent->getX();
            y += mParent->getY();
        }

        mTextPopup->show(x + getX(), y + getY(), mPopupText);
    }
    else
    {
        mTextPopup->setVisible(false);
    }
}

void Button::mouseExited(gcn::MouseEvent &event)
{
    gcn::Button::mouseExited(event);
    mTextPopup->mouseExited(event);

    mTextPopup->setVisible(false);
}

void Button::setButtonPopupText(const std::string& text)
{
    mPopupText = text;
    if (!mPopupText.empty())
        mTextPopup->show(getX(), getY(), mPopupText);
    else
        mTextPopup->setVisible(false);
}
