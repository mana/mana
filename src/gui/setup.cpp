/*----------------------------------------------------------------
 * setup.c -- setup routines
 *----------------------------------------------------------------
 *  This module takes care of everything relating to the
 *  setup dialog.
 */

#include "setup.h"

extern bool show_setup;

/*
 * Setup action listener
 */
void SetupActionListener::action(const std::string& eventId) {
  if(eventId == "apply") {
    puts("apply");
    show_setup = false;
  } else if (eventId == "cancel") {
    puts("calncel");
    show_setup = false;
  }
}

/* 
 * Display setup dialog
 */
Setup::Setup() {
  visible = false;
  setupDialog = new gcn::Container();
  displayLabel = new gcn::Label("Display");
  applyButton = new gcn::Button("Apply");
  cancelButton = new gcn::Button("Cancel");
  
  /* Set dialog elements */
  setupDialog->setDimension(gcn::Rectangle(300,300,200,200));
  displayLabel->setPosition(4,14);
  applyButton->setPosition(30,162);
  cancelButton->setPosition(146,162);
  
  /* Set events */
  applyButton->setEventId("apply");
  cancelButton->setEventId("cancel");
  
  /* Listen for actions */
  SetupActionListener *setupActionListener = new SetupActionListener();
  applyButton->addActionListener(setupActionListener);
  cancelButton->addActionListener(setupActionListener);
  
  /* Assemble dialog */
  setupDialog->add(displayLabel);
  setupDialog->add(applyButton);
  setupDialog->add(cancelButton);

  setupDialog->setVisible(visible);
  guiTop->add(setupDialog);
}

Setup::~Setup() {  
  delete setupDialog;
  delete displayLabel;
  delete applyButton;
  delete cancelButton;
}

void Setup::toggleVisible(bool toggle) {
  if(visible == false && toggle == true)
    visible == true;
    else
  if(visible == true && toggle == false) visible == false;
  
  setupDialog->setVisible(visible);
}
