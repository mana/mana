/*----------------------------------------------------------------
 * setup.h -- setup dialog module
 *----------------------------------------------------------------
 */

#ifndef tmw_included_setup_h
#define tmw_included_setup_h

#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif
#include "gui.h"
#include "../graphic/graphic.h"

class Setup {
 public:
  Setup();
  ~Setup();
  void toggleVisible(bool toggle);
  
 private:
  bool visible;
  /* Dialog parts */
  gcn::Container *setupDialog;
  gcn::Label *displayLabel;
  gcn::Button *applyButton;
  gcn::Button *cancelButton;
};

/* The action listener for setup dialog */
class SetupActionListener : public gcn::ActionListener
{
 public:
  void action(const std::string& eventId);
};

#endif
