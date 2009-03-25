/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <algorithm>
#include <cassert>
#include <climits>

#include <guichan/exception.hpp>

#include "gui.h"
#include "skin.h"
#include "popup.h"
#include "window.h"
#include "windowcontainer.h"

#include "../configlistener.h"
#include "../configuration.h"
#include "../log.h"

#include "../resources/image.h"

ConfigListener *Popup::popupConfigListener = 0;
int Popup::instances = 0;
bool Popup::mAlphaChanged = false;

class PopupConfigListener : public ConfigListener
{
    void optionChanged(const std::string &)
    {
        Popup::mAlphaChanged = true;
    }
};

Popup::Popup(const std::string& name, Window *parent,
             const std::string& skin):
    mParent(parent),
    mPopupName(name),
    mMinWidth(100),
    mMinHeight(40),
    mMaxWidth(INT_MAX),
    mMaxHeight(INT_MAX)
{
    logger->log("Popup::Popup(\"%s\")", name.c_str());

    if (!windowContainer)
        throw GCN_EXCEPTION("Popup::Popup(): no windowContainer set");

    if (instances == 0)
    {
        popupConfigListener = new PopupConfigListener();
        // Send GUI alpha changed for initialization
        popupConfigListener->optionChanged("guialpha");
        config.addListener("guialpha", popupConfigListener);
    }

    setPadding(3);

    instances++;

    // Loads the skin
    mSkin = skinLoader->load(skin);

    setGuiAlpha();

    // Add this window to the window container
    windowContainer->add(this);

    // Popups are invisible by default
    setVisible(false);
}

Popup::~Popup()
{
    logger->log("Popup::~Popup(\"%s\")", mPopupName.c_str());

    while (!mWidgets.empty())
    {
        gcn::Widget *w = mWidgets.front();
        remove(w);
        delete(w);
    }

    instances--;

    mSkin->instances--;

    if (instances == 0)
    {
        config.removeListener("guialpha", popupConfigListener);
        delete popupConfigListener;
        popupConfigListener = NULL;
    }
}

void Popup::setWindowContainer(WindowContainer *wc)
{
    windowContainer = wc;
}

void Popup::draw(gcn::Graphics *graphics)
{
    if (!isVisible())
        return;

    Graphics *g = static_cast<Graphics*>(graphics);

    g->drawImageRect(0, 0, getWidth(), getHeight(), mSkin->getBorder());

    // Update Popup alpha values
    if (mAlphaChanged)
    {
        for_each(mSkin->getBorder().grid, mSkin->getBorder().grid + 9,
                 std::bind2nd(std::mem_fun(&Image::setAlpha),
                 config.getValue("guialpha", 0.8)));
    }
    drawChildren(graphics);
}

gcn::Rectangle Popup::getChildrenArea()
{
    return gcn::Rectangle(getPadding(), 0, getWidth() - getPadding() * 2,
                          getHeight() - getPadding() * 2);
}

void Popup::setContentSize(int width, int height)
{
    width += 2 * getPadding();
    height += 2 * getPadding();

    if (getMinWidth() > width)
        width = getMinWidth();
    else if (getMaxWidth() < width)
        width = getMaxWidth();
    if (getMinHeight() > height)
        height = getMinHeight();
    else if (getMaxHeight() < height)
        height = getMaxHeight();

    setSize(width, height);
}

void Popup::setLocationRelativeTo(gcn::Widget *widget)
{
    int wx, wy;
    int x, y;

    widget->getAbsolutePosition(wx, wy);
    getAbsolutePosition(x, y);

    setPosition(getX() + (wx + (widget->getWidth() - getWidth()) / 2 - x),
                getY() + (wy + (widget->getHeight() - getHeight()) / 2 - y));
}

void Popup::setMinWidth(unsigned int width)
{
    mMinWidth = width > mSkin->getMinWidth() ? width : mSkin->getMinWidth();
}

void Popup::setMinHeight(unsigned int height)
{
    mMinHeight = height > mSkin->getMinHeight() ? height : mSkin->getMinHeight();
}

void Popup::setMaxWidth(unsigned int width)
{
    mMaxWidth = width;
}

void Popup::setMaxHeight(unsigned int height)
{
    mMaxHeight = height;
}

void Popup::scheduleDelete()
{
    windowContainer->scheduleDelete(this);
}

void Popup::setGuiAlpha()
{
    //logger->log("Popup::setGuiAlpha: Alpha Value %f", config.getValue("guialpha", 0.8));
    for (int i = 0; i < 9; i++)
    {
        //logger->log("Popup::setGuiAlpha: Border Image (%i)", i);
        mSkin->getBorder().grid[i]->setAlpha(config.getValue("guialpha", 0.8));
    }

    mAlphaChanged = false;
}

