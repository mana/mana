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

#ifndef _TMW_CHATINPUT_H
#define _TMW_CHATINPUT_H

#include "textfield.h"

/**
 * The chat input hides when it loses focus. It is also invisible by default.
 */
class ChatInput : public TextField
{
    public:
        /**
         * Constructor.
         */
        ChatInput();

        /**
         * Called if the chat input loses focus. It will set itself to
         * invisible as result.
         */
        void lostFocus();
};

#endif
