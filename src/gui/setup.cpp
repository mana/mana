/*----------------------------------------------------------------
 * setup.cpp -- setup routines
 *----------------------------------------------------------------
 *  This module takes care of everything relating to the
 *  setup dialog.
 */

#include "setup.h"
/*
 * Metod returns the number of elements in container
 */
int ModesListModel::getNumberOfElements() {
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
  displayLabel = new gcn::Label("Display");
  modesList = new gcn::ListBox(modesListModel);
  scrollArea = new gcn::ScrollArea(modesList);
  applyButton = new Button("Apply");
  cancelButton = new Button("Cancel");
  
  /* Set dimension */
  scrollArea->setDimension(gcn::Rectangle(0,0,120,50));
  displayLabel->setDimension(gcn::Rectangle(0,0,80, 16));
  applyButton->setDimension(gcn::Rectangle(0,0,80, 16));
  cancelButton->setDimension(gcn::Rectangle(0,0,80, 16));
  
  /* Set events */
  applyButton->setEventId("apply");
  cancelButton->setEventId("cancel");
  
  /* Set position */
  scrollArea->setPosition(10,40);
  displayLabel->setPosition(10,10);
  applyButton->setPosition(10,190);
  cancelButton->setPosition(150,190);
  
  /* Listen for actions */
  applyButton->addActionListener(this);
  cancelButton->addActionListener(this);
  
  /* Assemble dialog */
  add(scrollArea);
  add(displayLabel);
  add(applyButton);
  add(cancelButton);
  
  setSize(240,216);
  setLocationRelativeTo(getParent());

  //TODO: load default settings
  modesList->setSelected(1);
}

/* 
 * Destructor
 */
Setup::~Setup() {
  delete modesListModel;
  delete modesList;
  delete scrollArea;
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
    //TODO: Save&apply setup changes
  }
  else if(eventId == "cancel") {
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

