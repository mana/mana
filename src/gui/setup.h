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

#ifndef _TMW_SETUP_H
#define _TMW_SETUP_H

#include "window.h"

/**
 * The list model for mode list.
 *
 * \ingroup Interface
 */
class ModeListModel : public gcn::ListModel {
    public:
        /**
         * Constructor.
         */
        ModeListModel();

        /**
         * Destructor.
         */
        virtual ~ModeListModel();

        /**
         * Returns the number of elements in container.
         */
        int getNumberOfElements();

        /**
         * Returns element from container.
         */
        std::string getElementAt(int i);

    private:
        std::vector<std::string> videoModes;
};

/**
 * The setup dialog.
 *
 * \ingroup GUI
 */
class Setup : public Window, public gcn::ActionListener {
    private:
        // Dialog widgets
        gcn::Label *videoLabel, *audioLabel;
        gcn::ListBox *modeList;
        ModeListModel *modeListModel;
        gcn::ScrollArea *scrollArea;
        gcn::CheckBox *fsCheckBox;
        gcn::CheckBox *openGlCheckBox;
        gcn::Slider *alphaSlider;
        gcn::Label *alphaLabel;
        gcn::CheckBox *soundCheckBox;
        gcn::Slider *sfxSlider, *musicSlider;
        gcn::Label *sfxLabel, *musicLabel;
        gcn::Button *applyButton;
        gcn::Button *cancelButton;

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

extern Setup *setupWindow;

#endif
