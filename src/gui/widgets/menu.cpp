/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "gui/widgets/menu.h"

#include "graphics.h"

#include "gui/gui.h"

#include "resources/theme.h"

#include <guichan/font.hpp>
#include <guichan/mouseinput.hpp>

#include <algorithm>

Menu::Menu(const std::string &name)
    : Popup(name)
{
    addMouseListener(this);

    // Menus are sized to fit their items
    setMinWidth(0);
    setMinHeight(0);
}

void Menu::addItem(std::string caption, Action action)
{
    Entry &item = mItems.emplace_back();
    item.caption = std::move(caption);
    item.action = std::move(action);
}

void Menu::addSeparator()
{
    // Skip leading and repeated separators. Trailing ones are dropped when
    // the menu is laid out.
    if (mItems.empty() || mItems.back().separator)
        return;

    mItems.emplace_back().separator = true;
}

void Menu::clear()
{
    mItems.clear();
    mHoveredItem = -1;
}

void Menu::showAt(int x, int y)
{
    updateLayout();

    mHoveredItem = -1;

    x = std::clamp(x, 0, std::max(0, graphics->getWidth() - getWidth()));
    y = std::clamp(y, 0, std::max(0, graphics->getHeight() - getHeight()));

    setPosition(x, y);
    setVisible(true);
    requestMoveToTop();

    // The menu may open on top of a being or item, whose hover cursor would
    // otherwise stay around while the mouse is over the menu.
    gui->setCursorType(Cursor::Pointer);
}

void Menu::showBelow(gcn::Widget *widget)
{
    int x;
    int y;
    widget->getAbsolutePosition(x, y);
    showAt(x, y + widget->getHeight());
}

void Menu::close()
{
    setVisible(false);
    clear();
}

void Menu::updateLayout()
{
    while (!mItems.empty() && mItems.back().separator)
        mItems.pop_back();

    const Theme *theme = gui->getTheme();
    const Skin &itemSkin = theme->getSkin(SkinType::MenuItem);
    const Skin &separatorSkin = theme->getSkin(SkinType::MenuSeparator);
    gcn::Font *font = getFont();

    const int itemHeight = std::max(itemSkin.height,
                                    font->getHeight() + itemSkin.padding * 2);
    const int separatorHeight = std::max(separatorSkin.height, 1) +
                                separatorSkin.padding * 2;

    int width = itemSkin.width;
    int height = 0;

    for (Entry &item : mItems)
    {
        item.y = height;
        item.height = item.separator ? separatorHeight : itemHeight;
        height += item.height;

        if (!item.separator)
        {
            width = std::max(width, font->getWidth(item.caption) +
                                    itemSkin.padding * 2);
        }
    }

    setContentSize(width, height);
}

int Menu::getItemAt(int x, int y) const
{
    if (x < getPadding() || x >= getWidth() - getPadding())
        return -1;

    y -= getPadding();

    for (size_t i = 0; i < mItems.size(); ++i)
    {
        const Entry &item = mItems[i];
        if (item.separator)
            continue;
        if (y >= item.y && y < item.y + item.height)
            return static_cast<int>(i);
    }

    return -1;
}

void Menu::draw(gcn::Graphics *graphics)
{
    auto g = static_cast<Graphics *>(graphics);

    Popup::draw(graphics);

    const Theme *theme = gui->getTheme();
    const Skin &itemSkin = theme->getSkin(SkinType::MenuItem);
    const Skin &separatorSkin = theme->getSkin(SkinType::MenuSeparator);
    const Palette &palette = theme->getPalette(getSkin().palette);

    // Used when the MenuItem skin does not specify a text format itself
    TextFormat defaultFormat;
    defaultFormat.color = palette.getColor(Theme::TEXT);
    defaultFormat.outlineColor = palette.getOutlineColor(Theme::TEXT);

    WidgetState state;
    state.x = getPadding();
    state.width = getWidth() - getPadding() * 2;

    for (size_t i = 0; i < mItems.size(); ++i)
    {
        const Entry &item = mItems[i];
        state.y = getPadding() + item.y;
        state.height = item.height;
        state.flags = static_cast<int>(i) == mHoveredItem ? STATE_HOVERED : 0;

        if (item.separator)
        {
            WidgetState lineState = state;
            lineState.y += separatorSkin.padding;
            lineState.height -= separatorSkin.padding * 2;
            separatorSkin.draw(g, lineState);
            continue;
        }

        itemSkin.draw(g, state);

        const SkinState *skinState = itemSkin.getState(state.flags);
        const TextFormat &format = skinState ? skinState->textFormat
                                             : defaultFormat;
        gcn::Font *font = format.bold ? boldFont : getFont();

        g->drawText(item.caption,
                    state.x + itemSkin.padding,
                    state.y + (state.height - font->getHeight()) / 2,
                    Graphics::LEFT,
                    font,
                    format);
    }
}

void Menu::mouseMoved(gcn::MouseEvent &event)
{
    Popup::mouseMoved(event);

    gui->setCursorType(Cursor::Pointer);

    mHoveredItem = getItemAt(event.getX(), event.getY());
}

void Menu::mouseExited(gcn::MouseEvent &event)
{
    Popup::mouseExited(event);

    mHoveredItem = -1;
}

void Menu::mousePressed(gcn::MouseEvent &event)
{
    if (event.getButton() != gcn::MouseEvent::LEFT)
        return;

    const int index = getItemAt(event.getX(), event.getY());
    if (index == -1)
        return;

    // Take a copy, since performing the action may change the menu
    Action action = mItems[index].action;

    setVisible(false);
    mHoveredItem = -1;

    if (action)
        action();
}
