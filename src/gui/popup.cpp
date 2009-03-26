/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

int Popup::instances = 0;

Popup::Popup(const std::string& name, Window *parent,
             const std::string& skin):
    mParent(parent),
    mPopupName(name),
    mDefaultSkinPath(skin),
    mMinWidth(100),
    mMinHeight(40),
    mMaxWidth(graphics->getWidth()),
    mMaxHeight(graphics->getHeight())
{
    logger->log("Popup::Popup(\"%s\")", name.c_str());

    if (!windowContainer)
        throw GCN_EXCEPTION("Popup::Popup(): no windowContainer set");

    setPadding(3);

    instances++;

    // Loads the skin
    mSkin = skinLoader->load(skin, mDefaultSkinPath);

    // Add this window to the window container
    windowContainer->add(this);

    // Popups are invisible by default
    setVisible(false);
}

Popup::~Popup()
{
    logger->log("Popup::~Popup(\"%s\")", mPopupName.c_str());

    savePopupConfiguration();

    while (!mWidgets.empty())
    {
        gcn::Widget *w = mWidgets.front();
        remove(w);
        delete(w);
    }

    instances--;

    mSkin->instances--;
}

void Popup::setWindowContainer(WindowContainer *wc)
{
    windowContainer = wc;
}

void Popup::loadPopupConfiguration()
{
    if (mPopupName.empty())
        return;

    const std::string &name = mPopupName;
    const std::string &skinName = config.getValue(name + "Skin",
                                                  mSkin->getFilePath());

    if (skinName.compare(mSkin->getFilePath()) != 0)
    {
        mSkin->instances--;
        mSkin = skinLoader->load(skinName, mDefaultSkinPath);
    }
}

void Popup::savePopupConfiguration()
{
    if (mPopupName.empty())
        return;

    const std::string &name = mPopupName;

    // Saves the skin path in a config file (which allows for skins to be
    // changed from the default path)
    config.setValue(name + "Skin", mSkin->getFilePath());
}

void Popup::draw(gcn::Graphics *graphics)
{
    if (!isVisible())
        return;

    Graphics *g = static_cast<Graphics*>(graphics);

    g->drawImageRect(0, 0, getWidth(), getHeight(), mSkin->getBorder());

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

void Popup::setMinWidth(int width)
{
    mMinWidth = width > mSkin->getMinWidth() ? width : mSkin->getMinWidth();
}

void Popup::setMinHeight(int height)
{
    mMinHeight = height > mSkin->getMinHeight() ? height : mSkin->getMinHeight();
}

void Popup::setMaxWidth(int width)
{
    mMaxWidth = width;
}

void Popup::setMaxHeight(int height)
{
    mMaxHeight = height;
}

void Popup::scheduleDelete()
{
    windowContainer->scheduleDelete(this);
}

