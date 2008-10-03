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
 *  $Id: checkbox.h 4045 2008-04-07 15:23:07Z b_lindeijer $
 */

#ifndef _TMW_CHECKBOX_H
#define _TMW_CHECKBOX_H

#include <iosfwd>

#include <guichan/widgets/checkbox.hpp>

#include "../guichanfwd.h"

class Image;

/**
 * Check box widget. Same as the Guichan check box but with custom look.
 *
 * \ingroup GUI
 */
class CheckBox : public gcn::CheckBox {
    public:
        /**
         * Constructor.
         */
        CheckBox(const std::string& caption, bool selected = false);

        /**
         * Destructor.
         */
        ~CheckBox();

        /**
         * Draws the check box, not the caption.
         */
        void drawBox(gcn::Graphics* graphics);

    private:
        static int instances;
        static Image *checkBoxNormal;
        static Image *checkBoxChecked;
        static Image *checkBoxDisabled;
        static Image *checkBoxDisabledChecked;
};

#endif
