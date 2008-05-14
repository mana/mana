/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "popup_box.h"
#include "window.h"
#include "browserbox.h"

#include <iostream>
#include <sstream>

class PoppedSelectionWindow : public Window
{
public:
    PoppedSelectionWindow(PopupBox *owner):
        mOwner(owner)
    {
        setResizable(false);
        setTitleBarHeight(0);
        mShowTitle = false;

        mBrowserBox = new BrowserBox();
        mBrowserBox->setPosition(4, 4);
        mBrowserBox->setHighlightMode(BrowserBox::BACKGROUND);
        mBrowserBox->setOpaque(false);
        add(mBrowserBox);
        mBrowserBox->setLinkHandler(owner);

        initModel();
    }

    void initModel(void)
    {
        mBrowserBox->clearRows();
        gcn::ListModel *model = mOwner->getListModel();
        for (int i = 0; i < model->getNumberOfElements(); i++) {
            std::stringstream s;
            s << "@@" << i << "|" << model->getElementAt(i) << "@@";
            mBrowserBox->addRow(s.str());
        }

        setContentSize(mBrowserBox->getWidth() + 8, mBrowserBox->getHeight() + 8);
    }

protected:
    BrowserBox *mBrowserBox;
    PopupBox *mOwner;
};

PopupBox::PopupBox(gcn::ListModel *list_model) :
    DropDown(list_model),
    mWindow(NULL)
{
}

PopupBox::~PopupBox(void)
{
    if (mWindow)
        delete mWindow;
}

////////////////////////////////////////
// Widget ops

void
PopupBox::draw(gcn::Graphics *graphics)
{
    // fill background
    graphics->setColor(gcn::Color(0xd0, 0xd0, 0xd0));
    graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));

    // draw frame-ish object
    graphics->setColor(gcn::Color(0xc0, 0xc0, 0xc0));
    graphics->drawRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
    graphics->setColor(gcn::Color(0xe0, 0xe0, 0xe0));
    graphics->drawLine(0, getHeight(), getWidth(), getHeight());
    graphics->drawLine(getWidth(), 0, getWidth(), getHeight());

    graphics->drawText(getListModel()->getElementAt(getSelected()), 0, 0);
}

////////////////////////////////////////
// MouseListener ops

void
PopupBox::mousePressed(gcn::MouseEvent& mouseEvent)
{
    if (0 <= mouseEvent.getY()
        && mouseEvent.getY() < getHeight()
        && mouseEvent.getX() >= 0
        && mouseEvent.getX() < getWidth()
        && mouseEvent.getButton() == gcn::MouseEvent::LEFT
        && mouseEvent.getSource() == this) {
        if (mWindow == NULL) {
            mWindow = new PoppedSelectionWindow(this);
            mWindow->resizeToContent();
        }

        int x, y;
        getAbsolutePosition(x, y);
        mWindow->setPosition(mouseEvent.getX() + x,
                             mouseEvent.getY() + y);
        mWindow->setVisible(true);
        mWindow->requestMoveToTop();
    }
}

////////////////////////////////////////
// LinkHandler ops

void
PopupBox::handleLink(const std::string &link)
{
    if (mWindow)
        mWindow->setVisible(false);

    std::stringstream s;
    s << link;
    int index;
    s >> index;
    setSelected(index);
}
