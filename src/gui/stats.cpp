#include "stats.h"

/*
 * Constructor
 */
StatsWindow::StatsWindow(gcn::Container *parent)
  : Window(parent, "Stats") 
{ 
  statsStr[0] << "Strenght:    "  << char_info->STR;
  statsStr[1] << "Agility:     "  << char_info->AGI;
  statsStr[2] << "Vitality:    "  << char_info->VIT;
  statsStr[3] << "Inteligence: "  << char_info->INT;
  statsStr[4] << "Dexternity:  "  << char_info->DEX;
  statsStr[5] << "Luck:        "  << char_info->LUK;
  
  /* New labels */
  for(i=0; i<5;i++)
    statsLabel[i] = new gcn::Label(statsStr[i].str());
  
  /* New buttons */
  for(i=0; i<5;i++)
    statsButton[i] = new Button("+");
  
  /* Set position */
  for(i=0;i<5;i++) {
    statsLabel[i]->setPosition(10,(i*22)+10);
    statsButton[i]->setPosition(170,(i*22)+10);
  }
  
  /* Set button events Id */
  statsButton[0]->setEventId("STR");
  statsButton[1]->setEventId("AGI");
  statsButton[2]->setEventId("VIT");
  statsButton[3]->setEventId("INT");
  statsButton[4]->setEventId("DEX");
  statsButton[5]->setEventId("LUK");

  /* Assemble */
  for(i=0; i<5; i++) {
    add(statsLabel[i]);
    add(statsButton[i]);
  }
  
  setSize(200,150);
  setLocationRelativeTo(getParent());
}

/*
 * Destructor
 */
StatsWindow::~StatsWindow() {
  for(int i=0; i<5; i++) {
    delete statsStr[i];
    delete statsLabel[i];
    delete statsButton[i];
  }
}

/*
 * Static method for creating singleton objects
 */
StatsWindow * StatsWindow::ptr = NULL;
StatsWindow * StatsWindow::create_statswindow() {
  if(ptr == NULL)
    ptr = new StatsWindow(guiTop);
  else
    ptr->setVisible(true);
  return ptr;
}

/*
 * Event handling method
 */
void StatsWindow::action(const std::string& eventId) {
  if(eventId == "STR")
    setVisible(false);
  if(eventId == "AGI")
    setVisible(false);
  if(eventId == "VIT")
    setVisible(false);
  if(eventId == "INT")
    setVisible(false);
  if(eventId == "DEX")
    setVisible(false);
  if(eventId == "LUK")
    setVisible(false);
}
