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

#include "gui.h"
#include "button.h"
#include "checkbox.h"
#include "scrollarea.h"
#include "listbox.h"
#include "radiobutton.h"
#include "../graphic/graphic.h"
#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif

#include "../sound/sound.h"

/*
 * The list model for modes list
 */
class ModesListModel : public gcn::ListModel {
 public:
  int getNumberOfElements();
  std::string getElementAt(int i);
};

/* 
 * Setup dialog window
 */
class Setup : public Window, public gcn::ActionListener {
 private:
  /* Dialog parts */
  ModesListModel *modesListModel;
  gcn::Label *displayLabel;
  CheckBox *fsCheckBox;
  gcn::Label *soundLabel;
  CheckBox *soundCheckBox;
  RadioButton *disabledRadio;
  ScrollArea *scrollArea;
  ListBox *modesList;
  Button *applyButton;
  Button *cancelButton;
  
  /* Setup dialog */
  static Setup *ptr;
  
  /* Methods */
  Setup(gcn::Container *parent);
  virtual ~Setup();
  
 public:
  void action(const std::string& eventId);
  static Setup * create_setup();
};

#endif
