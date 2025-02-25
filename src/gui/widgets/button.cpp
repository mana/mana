/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "gui/widgets/button.h"

#include "configuration.h"
#include "graphics.h"

#include "gui/textpopup.h"

#include "resources/image.h"
#include "resources/theme.h"

#include "utils/dtor.h"

#include <guichan/exception.hpp>
#include <guichan/font.hpp>

int Button::mInstances = 0;
float Button::mAlpha = 1.0;
ImageRect *Button::mButton;
TextPopup *Button::mTextPopup = nullptr;

enum {
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

Button::Button()
{
    init();
    adjustSize();
}

Button::Button(const std::string &caption, const std::string &actionEventId,
    gcn::ActionListener *listener):
    gcn::Button(caption)
{
    init();
    setActionEventId(actionEventId);

    if (listener)
        addActionListener(listener);

    adjustSize();
}

bool Button::setButtonIcon(const std::string &iconFile)
{
    // We clean up possible older references.
    removeButtonIcon();

    // If nothing relevant was set, we can quit now.
    if (iconFile.empty())
        return false;

    // Load the icon frames.
    auto btnIcons = Theme::getImageFromTheme(iconFile);
    if (!btnIcons)
        return false;

    // Compute the sub images size.
    const int frameWidth = btnIcons->getWidth() / 4;
    const int frameHeight = btnIcons->getHeight();

    if (frameWidth > 0 && frameHeight > 0)
    {
        mButtonIcon.resize(BUTTON_COUNT);

        for (int mode = 0; mode < BUTTON_COUNT; ++mode)
        {
            mButtonIcon[mode].reset(
                btnIcons->getSubImage(mode * frameWidth, 0, frameWidth, frameHeight));
        }

        adjustSize();
    }

    return !mButtonIcon.empty();
}

void Button::removeButtonIcon()
{
    if (mButtonIcon.empty())
        return;

    mButtonIcon.clear();
    adjustSize();
}

void Button::init()
{
    setFrameSize(0);

    if (mInstances == 0)
    {
        // Load the skin
        mButton = new ImageRect[BUTTON_COUNT];

        for (int mode = 0; mode < BUTTON_COUNT; ++mode)
        {
            auto modeImage = Theme::getImageFromTheme(data[mode].file);
            int a = 0;
            for (int y = 0; y < 3; y++)
            {
                for (int x = 0; x < 3; x++)
                {
                    mButton[mode].grid[a] = modeImage->getSubImage(
                            data[x].gridX, data[y].gridY,
                            data[x + 1].gridX - data[x].gridX + 1,
                            data[y + 1].gridY - data[y].gridY + 1);
                    a++;
                }
            }
        }
        updateAlpha();

        // Create the tooltip popup. It is shared by all buttons and will get
        // deleted by the WindowContainer.
        if (!mTextPopup)
            mTextPopup = new TextPopup;
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
            std::for_each(mButton[mode].grid, mButton[mode].grid + 9,
                dtor<Image*>());
        }
        delete[] mButton;
    }
}

void Button::updateAlpha()
{
    float alpha = std::max(config.guiAlpha,
                           Theme::instance()->getMinimumOpacity());

    if (mAlpha != alpha)
    {
        mAlpha = alpha;
        for (int mode = 0; mode < BUTTON_COUNT; ++mode)
        {
            mButton[mode].setAlpha(mAlpha);
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

    Image *icon = mButtonIcon.empty() ? nullptr : mButtonIcon[mode].get();
    int textX = 0;
    int textY = getHeight() / 2 - getFont()->getHeight() / 2;
    int btnIconX = 0;
    int btnIconY = getHeight() / 2 - (icon ? icon->getHeight() / 2 : 0);
    int btnIconWidth = icon ? icon->getWidth() : 0;

    switch (getAlignment())
    {
        case gcn::Graphics::LEFT:
            if (btnIconWidth)
            {
                btnIconX = 4;
                textX = btnIconX + icon->getWidth() + 2;
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
                    + icon->getWidth() + 2) / 2;
                textX = getWidth() / 2 + icon->getWidth() / 2 + 2;
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
        static_cast<Graphics *>(graphics)->drawImage(icon, btnIconX, btnIconY);
    graphics->drawText(getCaption(), textX, textY, getAlignment());
}

void Button::adjustSize()
{
    // Size of the image button.
    int iconWidth = 0, iconHeight = 0;
    if (!mButtonIcon.empty())
    {
        for (int mode = 0; mode < BUTTON_COUNT; ++mode)
        {
            const Image *icon = mButtonIcon[mode].get();
            iconWidth = std::max(iconWidth, icon->getWidth() + 2);
            iconHeight = std::max(iconHeight, icon->getHeight());
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

void Button::mouseMoved(gcn::MouseEvent &event)
{
    gcn::Button::mouseMoved(event);

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

    mTextPopup->setVisible(false);
}
