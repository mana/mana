/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#ifndef WINDOWCONTAINER_H
#define WINDOWCONTAINER_H

#include "gui/widgets/container.h"

/**
 * A window container. This container adds functionality for more convenient
 * widget (windows in particular) destruction.
 *
 * \ingroup GUI
 */
class WindowContainer : public Container
{
    public:
        /**
         * Do GUI logic. This functions adds automatic deletion of objects that
         * volunteered to be deleted.
         */
        void logic();

        /**
         * Schedule a widget for deletion. It will be deleted at the start of
         * the next logic update.
         */
        void scheduleDelete(gcn::Widget *widget);

    private:
        /**
         * List of widgets that are scheduled to be deleted.
         */
        typedef std::list<gcn::Widget*> Widgets;
        typedef Widgets::iterator WidgetIterator;
        Widgets mDeathList;
};

extern WindowContainer *windowContainer;

#endif
