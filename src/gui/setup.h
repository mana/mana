/*----------------------------------------------------------------
 * setup.h -- setup dialog module
 *----------------------------------------------------------------
 */

#ifndef tmw_included_setup_h
#define tmw_included_setup_h

#include "gui.h"
#include "button.h"
#include "checkbox.h"
#include "../graphic/graphic.h"
#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif


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
  gcn::ScrollArea *scrollArea;
  gcn::ListBox *modesList;
  CheckBox *fsCheckBox;
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
