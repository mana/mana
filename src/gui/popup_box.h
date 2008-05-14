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

#ifndef _TMW_POPUP_BOX_H
#define _TMW_POPUP_BOX_H

#include <guichan/widgets/dropdown.hpp>

#include "../guichanfwd.h"
#include "linkhandler.h"

class PoppedSelectionWindow;

/**
 * Popup box widget.  Appears as a box that `pops out' into a separate floating window when selected.
 *
 * API adapted from guichan's listbox API.  Typewise
 */
class PopupBox : public gcn::DropDown,
                 public LinkHandler
{
public:
    /**
     * Constructor.
     *
     * \param listModel the list model to use.
     */
    PopupBox(gcn::ListModel *list_model = NULL);

    /**
     * Destructor.
     */
    virtual ~PopupBox(void);

    // Overriding Widget

    virtual void draw(gcn::Graphics* graphics);


    // Overriding MouseListener

    virtual void mousePressed(gcn::MouseEvent& mouseEvent);

    
    // Implementing Linkhandler
    void handleLink(const std::string& link);

protected:
    PoppedSelectionWindow *mWindow;
};


#endif /* !defined(_TMW_POPUP_BOX_H) */
