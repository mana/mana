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

#ifndef _STATS_WINDOW_H
#define _STATS_WINDOW_H

#include "button.h"
#include "window.h"
#include "../main.h"
#include <string>
#include <sstream>

class StatsWindow : public Window, public gcn::ActionListener {
 public:
    /**
     * Create the Stats window
     */
    static StatsWindow *create_statswindow();
    
    /**
     * Metod called when receiving actions from widget.
     */
    void action(const std::string& eventId);
    
    /**
     * Metod updates stats in window.
     */
    void update();
    
 private:
    int i;

    /** 
     * Stats captions 
     */
    gcn::Label *statsLabel[6];
    
    /** 
     * Stats buttons 
     */
    Button *statsButton[6];
    
    /**
     * Stats window ptr 
     */
    static StatsWindow *ptr;
    
    /**
     * Constructor.
     */
    StatsWindow(gcn::Container *parent);
    
    /**
     * Destructor.
     */
    virtual ~StatsWindow();
};

#endif /* _STATS_Window_H */
