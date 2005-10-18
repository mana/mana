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

#ifndef _TMW_GUI_GCCONTAINER_H
#define _TMW_GUI_GCCONTAINER_H

#include <list>

#include <guichan/widgets/container.hpp>

class GCContainer : public gcn::Container
{
    public:
        virtual ~GCContainer();
        virtual void add(gcn::Widget *w, bool delChild=true);
        virtual void add(gcn::Widget *w, int x, int y, bool delChild=true);
        virtual void _announceDeath(gcn::Widget *w);

    protected:
        std::list<gcn::Widget*> mDeathList;
};

#endif
