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
 *
 *  $Id$
 */

#include <iostream>
#include "windowcontainer.h"
#include "window.h"

WindowContainer::WindowContainer():
    mouseDown(false),
    modalWindow(NULL)
{
}

void WindowContainer::logic()
{
    std::list<gcn::Widget*>::iterator i = deathList.begin();
    while (i != deathList.end()) {
        delete (*i);
        i = deathList.erase(i);
    }

    gcn::Container::logic();
}

void WindowContainer::_mouseInputMessage(const gcn::MouseInput &mouseInput)
{
    if (mouseInput.getType() == gcn::MouseInput::PRESS)
    {
        mouseDown = true;
    }
    else if (mouseInput.getType() == gcn::MouseInput::RELEASE)
    {
        mouseDown = false;
    }

    // Make drag events not change widget with mouse. The Window instances
    // need this behaviour to be able to handle window dragging.
    if (!(mouseInput.getType() == gcn::MouseInput::MOTION && mouseDown))
    {
        Widget* tempWidgetWithMouse = NULL;

        WidgetIterator iter;
        for (iter = mWidgets.begin(); iter != mWidgets.end(); iter++)
        {
            if ((*iter)->getDimension().isPointInRect(
                        mouseInput.x, mouseInput.y) &&
                    (*iter)->isVisible())
            {
                tempWidgetWithMouse = (*iter);
            }
        }

        if (tempWidgetWithMouse != mWidgetWithMouse)
        {
            if (mWidgetWithMouse) {
                mWidgetWithMouse->_mouseOutMessage();
            }

            if (tempWidgetWithMouse) {
                tempWidgetWithMouse->_mouseInMessage();
            }

            mWidgetWithMouse = tempWidgetWithMouse;
        }
    }

    if (mWidgetWithMouse && !mWidgetWithMouse->hasFocus() &&
            (!modalWindow || (gcn::Widget*)modalWindow == mWidgetWithMouse)) {
        gcn::MouseInput mi = mouseInput;
        mi.x -= mWidgetWithMouse->getX();
        mi.y -= mWidgetWithMouse->getY();      
        mWidgetWithMouse->_mouseInputMessage(mi);
    }

    if (mWidgetWithMouse == NULL) {
        gcn::Widget::_mouseInputMessage(mouseInput);
    }
}

void WindowContainer::add(gcn::Widget *widget)
{
    gcn::Container::add(widget);
}

void WindowContainer::add(Window *window, bool modal)
{
    gcn::Container::add(window);
    if (modal) {
        setModalWindow(window);
    }
}

void WindowContainer::remove(gcn::Widget *widget)
{
    if (modalWindow == widget) {
        setModalWindow(NULL);
    }
    gcn::Container::remove(widget);
}

void WindowContainer::_announceDeath(gcn::Widget *widget)
{
    if (modalWindow == widget) {
        setModalWindow(NULL);
    }
    gcn::Container::_announceDeath(widget);
}

void WindowContainer::clear()
{
    modalWindow = NULL;
    gcn::Container::clear();
}

void WindowContainer::setModalWindow(Window *window)
{
    if (modalWindow != window) {
        if (!window) {
            // Removing modal window, at this point there must be a modal
            // window set.
            Window *modalParent = modalWindow->getParentWindow();
            if (modalParent && modalParent->isModal()) {
                // Return modality to parent.
                modalWindow = modalParent;
                return;
            }
        }
        else if (modalWindow && window->getParentWindow() != modalWindow) {
            // Adding a modal window, but failed sanity check.
            std::cerr << "Error: existing modal window not parent.\n";
            return;
        }
        modalWindow = window;
    }
}

gcn::Widget *WindowContainer::getModalWindow()
{
    return modalWindow;
}

void WindowContainer::scheduleDelete(gcn::Widget *widget)
{
    if (widget == (gcn::Widget*)modalWindow) {
        setModalWindow(NULL);
    }
    deathList.push_back(widget);
}
