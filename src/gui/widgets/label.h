/*
 *  The Mana World
 *  Copyright (c) 2009  Aethyra Development Team 
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

#ifndef LABEL_H
#define LABEL_H

#include <guichan/widgets/label.hpp>

/**
 * Label widget. Same as the Guichan label but modified to use the palette
 * system.
 *
 * \ingroup GUI
 */
class Label : public gcn::Label
{
    public:
        /**
         * Constructor.
         */
        Label();

        /**
         * Constructor. This version of the constructor sets the label with an
         * inintialization string.
         */
        Label(const std::string &caption);

        /**
         * Draws the label.
         */
        void draw(gcn::Graphics *graphics);
};

#endif
