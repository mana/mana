/*----------------------------------------------------------------
 * setup.c -- setup routines
 *----------------------------------------------------------------
 *  This module takes care of everything relating to the
 *  setup dialog.
 */

#include "setup.h"

/* 
 * Display setup dialog
 */
Setup::Setup(gcn::Container *parent)
  : Window(parent, "Setup")
{
  displayLabel = new gcn::Label("Display");
  applyButton = new gcn::Button("Apply");
  cancelButton = new gcn::Button("Cancel");
  
  /* Set dimension */
  displayLabel->setDimension(gcn::Rectangle(0,0,128, 16));
  applyButton->setDimension(gcn::Rectangle(0,0,128, 16));
  cancelButton->setDimension(gcn::Rectangle(0,0,128, 16));
  
  /* Set events */
  applyButton->setEventId("apply");
  cancelButton->setEventId("cancel");
  
  /* Set position */
  displayLabel->setPosition(10,10);
  applyButton->setPosition(10,100);
  cancelButton->setPosition(100,100);
  
  /* Listen for actions */
  applyButton->addActionListener(this);
  cancelButton->addActionListener(this);
  
  /* Assemble dialog */
  add(displayLabel);
  add(applyButton);
  add(cancelButton);
  
  setSize(240,216);
  setLocationRelativeTo(getParent());
}

Setup::~Setup() {  
  delete displayLabel;
  delete applyButton;
  delete cancelButton;
}

void Setup::action(const std::string& eventId)
{

  if(eventId == "apply") {
    puts("apply");
  }
  else if(eventId == "cancel") {
    puts("cancel");
  }
}

void create_setup() {
  Setup *setup;
  setup = new Setup(guiTop);
  
  while(!key[KEY_ESC] && !key[KEY_ENTER]) {
    gui_update(NULL);
  }
  
  delete setup;
}
