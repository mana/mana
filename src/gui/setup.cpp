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

/*
 *  This module takes care of everything relating to the
 *  setup dialog.
 */

#include "setup.h"
#include <allegro.h>

/*
 * Metod returns the number of elements in container
 */
int ModesListModel::getNumberOfElements() {
  //TODO after moving to SDL
  return 3;
}

/*
 * Metod returns element from container
 */
std::string ModesListModel::getElementAt(int i) {
  //TODO: change hardcoded modes after moving to SDL
  struct Modes {
    int height, width;
    char *desc;
  };
  static Modes modes[] = {
    { 640,480, "640x480"},
    { 800,600, "800x600" },
    { 1024,768, "1024x768" }
  };
  return(modes[i].desc);
}

/* 
 * Setup dialog constructor
 */
Setup::Setup(gcn::Container *parent)
  : Window(parent, "Setup")
{
  modesListModel = new ModesListModel();
  displayLabel = new gcn::Label("Display settings");
  modesList = new gcn::ListBox(modesListModel);
  scrollArea = new gcn::ScrollArea(modesList);
  fsCheckBox = new CheckBox("Full screen", false);
  soundLabel = new gcn::Label("Sound settings");
  soundCheckBox = new CheckBox("Sound", false);
  applyButton = new Button("Apply");
  cancelButton = new Button("Cancel");
  
  /* Set dimension */
  scrollArea->setDimension(gcn::Rectangle(0,0,120,50));
  displayLabel->setDimension(gcn::Rectangle(0,0,100,16));
  applyButton->setDimension(gcn::Rectangle(0,0,80, 16));
  cancelButton->setDimension(gcn::Rectangle(0,0,80, 16));
  
  /* Set events */
  applyButton->setEventId("apply");
  cancelButton->setEventId("cancel");
  
  /* Set position */
  scrollArea->setPosition(10,40);
  displayLabel->setPosition(10,10);
  soundLabel->setPosition(10,90);
  fsCheckBox->setPosition(110,36);
  soundCheckBox->setPosition(10,116);
  applyButton->setPosition(10,190);
  cancelButton->setPosition(150,190);
  
  /* Listen for actions */
  applyButton->addActionListener(this);
  cancelButton->addActionListener(this);
  
  /* Assemble dialog */
  add(scrollArea);
  add(displayLabel);
  add(fsCheckBox);
  add(soundLabel);
  add(soundCheckBox);
  add(applyButton);
  add(cancelButton);
  
  setSize(240,216);
  setLocationRelativeTo(getParent());

  //TODO: load default settings
  modesList->setSelected(1);
  if(config.getValue("screen",0) == 1) 
    fsCheckBox->setMarked(true);
  soundCheckBox->setMarked(config.getValue("sound",0));
}

/* 
 * Destructor
 */
Setup::~Setup() {
  delete modesListModel;
  delete modesList;
  delete scrollArea;
  delete fsCheckBox;
  delete displayLabel;
  delete applyButton;
  delete cancelButton;
}

/*
 * Event handling method
 */
void Setup::action(const std::string& eventId)
{
  if(eventId == "apply") {
    setVisible(false);
    
    /* Display settings */
    if(fsCheckBox->isMarked() == true && config.getValue("screen",0) == 2) {
	    config.setValue("screen",1);
	    set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,800,600,0,0);
	    
    } else 
    if(fsCheckBox->isMarked() == false && config.getValue("screen",0) == 1) {
	    config.setValue("screen",2);
	    set_gfx_mode(GFX_AUTODETECT_WINDOWED,800,600,0,0);
    }
    
    /* Sound settings */
    if(soundCheckBox->isMarked() == true) {
            config.setValue("sound",1);
    } else {
	    config.setValue("sound",0);
    }
    
  } else if(eventId == "cancel") {
    setVisible(false);
  }
}

/*
 * Static method for creating singleton objects
 */
Setup * Setup::ptr = NULL;
Setup * Setup::create_setup() {
  if(ptr == NULL)
    ptr = new Setup(guiTop);
   else
    ptr->setVisible(true);
    
  return ptr;
}

