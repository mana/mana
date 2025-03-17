/*
 *  The Mana Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
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

#include "gui/widgets/dropdown.h"

#include "graphics.h"

#include "gui/gui.h"
#include "gui/sdlinput.h"

#include "gui/widgets/listbox.h"
#include "gui/widgets/scrollarea.h"

#include "resources/theme.h"

DropDown::DropDown(gcn::ListModel *listModel):
    gcn::DropDown::DropDown(listModel,
                            new ScrollArea,
                            new ListBox(listModel))
{
    setFrameSize(2);
}

DropDown::~DropDown()
{
    delete mScrollArea;
}

void DropDown::draw(gcn::Graphics* graphics)
{
    const int h = mDroppedDown ? mFoldedUpHeight : getHeight();

    const int alpha = gui->getTheme()->getGuiAlpha();
    gcn::Color faceColor = getBaseColor();
    faceColor.a = alpha;
    const gcn::Color *highlightColor = &Theme::getThemeColor(Theme::HIGHLIGHT, alpha);
    gcn::Color shadowColor = faceColor - 0x303030;
    shadowColor.a = alpha;

    if (mListBox->getListModel() && mListBox->getSelected() >= 0)
    {
        graphics->setFont(getFont());
        graphics->setColor(Theme::getThemeColor(Theme::TEXT));
        graphics->drawText(mListBox->getListModel()->getElementAt(mListBox->getSelected()), 1, 0);
    }

    if (isFocused())
    {
        graphics->setColor(*highlightColor);
        graphics->drawRectangle(gcn::Rectangle(0, 0, getWidth() - h, h));
    }

    drawButton(graphics);

    if (mDroppedDown)
    {
        drawChildren(graphics);

        // Draw two lines separating the ListBox with selected
        // element view.
        graphics->setColor(*highlightColor);
        graphics->drawLine(0, h, getWidth(), h);
        graphics->setColor(shadowColor);
        graphics->drawLine(0, h + 1, getWidth(), h + 1);
    }
}

void DropDown::drawFrame(gcn::Graphics *graphics)
{
    const int bs = getFrameSize();

    WidgetState state(this);
    state.width += bs * 2;
    state.height += bs * 2;

    gui->getTheme()->drawSkin(static_cast<Graphics *>(graphics), SkinType::DropDownFrame, state);
}

void DropDown::drawButton(gcn::Graphics *graphics)
{
    WidgetState state(this);
    if (mDroppedDown)
    {
        state.height = mFoldedUpHeight;
        state.flags |= STATE_SELECTED;
    }
    if (mPushed)
        state.flags |= STATE_HOVERED;

    const auto theme = gui->getTheme();
    const int buttonWidth = theme->getMinWidth(SkinType::DropDownButton);

    // FIXME: Needs support for setting alignment in the theme.
    state.x = state.width - buttonWidth;

    theme->drawSkin(static_cast<Graphics *>(graphics), SkinType::DropDownButton, state);
}

// -- KeyListener notifications
void DropDown::keyPressed(gcn::KeyEvent& keyEvent)
{
    if (keyEvent.isConsumed())
        return;

    const gcn::Key key = keyEvent.getKey();
    const int selectedIndex = getSelected();

    if (key.getValue() == Key::ENTER || key.getValue() == Key::SPACE)
        dropDown();
    else if (key.getValue() == Key::UP)
        setSelected(getSelected() - 1);
    else if (key.getValue() == Key::DOWN)
        setSelected(getSelected() + 1);
    else if (key.getValue() == Key::HOME)
        setSelected(0);
    else if (key.getValue() == Key::END)
        setSelected(mListBox->getListModel()->getNumberOfElements() - 1);
    else
        return;

    keyEvent.consume();

    if (getSelected() != selectedIndex)
        distributeActionEvent();
}

void DropDown::focusLost(const gcn::Event& event)
{
    gcn::DropDown::focusLost(event);
    releaseModalMouseInputFocus();
}

void DropDown::mousePressed(gcn::MouseEvent& mouseEvent)
{
    gcn::DropDown::mousePressed(mouseEvent);

    if (0 <= mouseEvent.getY() && mouseEvent.getY() < getHeight() &&
        mouseEvent.getX() >= 0 && mouseEvent.getX() < getWidth() &&
        mouseEvent.getButton() == gcn::MouseEvent::LEFT && mDroppedDown &&
        mouseEvent.getSource() == mListBox)
    {
        mPushed = false;
        foldUp();
        releaseModalMouseInputFocus();
        distributeActionEvent();
    }
}

void DropDown::mouseWheelMovedUp(gcn::MouseEvent& mouseEvent)
{
    setSelected(getSelected() - 1);
    mouseEvent.consume();
    distributeActionEvent();
}

void DropDown::mouseWheelMovedDown(gcn::MouseEvent& mouseEvent)
{
    setSelected(getSelected() + 1);
    mouseEvent.consume();
    distributeActionEvent();
}
