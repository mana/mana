/*----------------------------------------------------------------
 * setup.h -- setup dialog module
 *----------------------------------------------------------------
 */

#ifndef tmw_included_setup_h
#define tmw_included_setup_h

#include "gui.h"
#include "../graphic/graphic.h"
#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif

class Setup : public Window, public gcn::ActionListener {
 private:
  /* Dialog parts */
  gcn::Label *displayLabel;
  gcn::Button *applyButton;
  gcn::Button *cancelButton;
  
 public:
  Setup(gcn::Container *parent);
  ~Setup();
  
  void action(const std::string& eventId);
  
};

void create_setup();

#endif
