#ifndef _STATS_WINDOW_H
#define _STATS_WINDOW_H

#include "button.h"
#include "window.h"
#include "main.h"

class StatsWindow : public Window, public gcn::ActionListener {
 public:
  /*
   * Create the Stats window
   */
  static StatsWindow *create_statswindow();
  
  /*
   * Metod called when receiving actions from widget.
   */
  void action(const std::string& eventId);
  
 private:
  /* Stats values  */
  std::string statsStr[5];  
  
  /* Stats captions */
  gcn::Label *statsLabel[5];
  
  /* Stats buttons */
  Button *statsButton[5];
  
  /* Stats window ptr */
  static StatsWindow *ptr;
  
  /*
   * Constructor.
   */
  StatsWindow(gcn::Container *parent);
  
  /*
   * Destructor.
   */
  virtual ~StatsWindow();
};

#endif /* _STATS_Window_H */
