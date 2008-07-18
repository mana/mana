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
 *  $Id: box.h 2529 2006-08-13 10:20:19Z b_lindeijer $
 */


#ifndef BOX_H
#define BOX_H

#include <guichan/widgets/container.hpp>

#include "../guichanfwd.h"

class Box : public gcn::Container
{
    public:
        /**
         * Returns padding.
         */
        unsigned int getPadding();

        /**
         * Sets padding between widgets.
         */
        void setPadding(unsigned int);

    protected:
        Box();
        virtual ~Box();

        /**
         * Spacing between client widgets.
         */
        unsigned int padding;

        virtual void draw(gcn::Graphics *) = 0;

        typedef std::list<gcn::Widget*> Widgets;
        typedef Widgets::iterator WidgetIterator;
};

#endif
