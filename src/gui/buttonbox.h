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
 *  $Id: buttonbox.h
 */

#ifndef _TMW_BUTTONBOX_H
#define _TMW_BUTTONBOX_H

#include <string>

#include <guichan/actionlistener.hpp>

#include "../guichanfwd.h"

#include "window.h"

class ButtonBoxListener
{
    public:

	/*
	 * function that ButtonBox calls when the button has been pressed
	 */
	virtual void buttonBoxRespond() = 0;
};

class ButtonBox : public Window, public gcn::ActionListener
{
    public:

	/*
	 * Constructor
	 *
	 * @param title is the text that appears at the top of the box
	 * @param buttonTxt is the text that appears on the button
	 * @param listener points to the class that should respond to the
	 *                 button press
	 */
	ButtonBox(const std::string &title, const std::string &buttonTxt,
		  ButtonBoxListener *listener);

	/*
	 * called when the button is pressed
	 *
	 * @param event is the event that is generated
	 */
	void
	action(const gcn::ActionEvent &event);

    private:

	ButtonBoxListener *mListener;
};
#endif
