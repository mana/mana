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

#ifndef tmw_included_setup_h
#define tmw_included_setup_h

#include "window.h"
#include "../sound.h"

/**
 * The list model for mode list.
 *
 * \ingroup GUI
 */
class ModeListModel : public gcn::ListModel {
    public:
        /**
         * Returns the number of elements in container.
         */
        int getNumberOfElements();

        /**
         * Returns element from container.
         */
        std::string getElementAt(int i);

	/**
	 * Constructor.
	 */
	ModeListModel();

	/**
	 * Destructor.
	 */
   	virtual ~ModeListModel();

    private:
	int nmode;
	char **mode;
};

/**
 * The setup dialog.
 *
 * \ingroup GUI
 */
class Setup : public Window, public gcn::ActionListener {
    private:
        // Dialog parts
        ModeListModel *modeListModel;
        gcn::Label *displayLabel;
        gcn::CheckBox *fsCheckBox;
        gcn::Label *soundLabel;
        gcn::CheckBox *soundCheckBox;
        gcn::RadioButton *disabledRadio;
        gcn::ScrollArea *scrollArea;
        gcn::ListBox *modeList;
        gcn::Button *applyButton;
        gcn::Button *cancelButton;

	// Video selections
	int last_sel, sel;
	
    public:
    
        /**
         * Constructor.
         */
        Setup();

        /**
         * Destructor.
         */
        ~Setup();    
    
        /**
         * Event handling method.
         */
        void action(const std::string& eventId);

};

#endif
