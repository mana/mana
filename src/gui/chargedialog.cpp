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
 */

 /* The window supported by this class shows player stats and keeps a charging
  * action bar in queue.
  */

#include "chargedialog.h"
#include "listbox.h"
#include "scrollarea.h"
#include "button.h"
#include "../main.h"
#include <time.h>

ChargeDialog::ChargeDialog():
    Window("")
{
    
    
    setSize(180,70);
    progBar = new ProgressBar(0.0f,20,40,140,25,128,128,128);
    add(progBar);
}

ChargeDialog::~ChargeDialog()
{
 
}

void ChargeDialog::action(const std::string& eventId)
{

}

// update the dialog
void ChargeDialog::draw(gcn::Graphics *graphics)
{
    // calculate time since the last attack was made
    char_info->lastAttackTime += .01; // this a hack until someone explains
                                      // to me how to work the timer
    if(char_info->lastAttackTime > 1){char_info->lastAttackTime=1;}
    
    // reset the progress bar to display accurate time since attack
    progBar->setProgress(char_info->lastAttackTime); 
    Window::draw(graphics);
}
