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
  applyButton = new Button("Apply");
  cancelButton = new Button("Cancel");
  
  /* Set dimension */
  displayLabel->setDimension(gcn::Rectangle(0,0,80, 16));
  applyButton->setDimension(gcn::Rectangle(0,0,80, 16));
  cancelButton->setDimension(gcn::Rectangle(0,0,80, 16));
  
  /* Set events */
  applyButton->setEventId("apply");
  cancelButton->setEventId("cancel");
  
  /* Set position */
  displayLabel->setPosition(10,10);
  applyButton->setPosition(10,190);
  cancelButton->setPosition(150,190);
  
  /* Listen for actions */
  applyButton->addActionListener(this);
  cancelButton->addActionListener(this);
  
  /* Assemble dialog */
  add(displayLabel);
  add(applyButton);
  add(cancelButton);
  
  setSize(240,216);
  setLocationRelativeTo(getParent());
  
  /* Is hidden */
  //setVisible(false);
}

Setup::~Setup() {  
  delete displayLabel;
  delete applyButton;
  delete cancelButton;
}

void Setup::action(const std::string& eventId)
{
  if(eventId == "apply") {
    setVisible(false);
  }
  else if(eventId == "cancel") {
    setVisible(false);
  }
}

Setup * Setup::ptr = NULL;
Setup * Setup::create_setup() {
  if(ptr == NULL)
    ptr = new Setup(guiTop);
   else
    ptr->setVisible(true);
    
  return ptr;
}
