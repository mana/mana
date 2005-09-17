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

#ifndef _TMW_STATUS_H
#define _TMW_STATUS_H

#include <iosfwd>

#include <guichan/actionlistener.hpp>

#include "window.h"

#include "../guichanfwd.h"

class ProgressBar;


/**
 * The player status dialog.
 *
 * \ingroup Interface
 */
class StatusWindow : public Window, public gcn::ActionListener {
    public:
        /**
         * Constructor.
         */
        StatusWindow();

        /**
         * Destructor.
         */
        ~StatusWindow();

         /**
         * Called when receiving actions from widget.
         */
        void action(const std::string& eventId);

        /**
         * Draw this window
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Updates this dialog with values from PLAYER_INFO *char_info
         */
        void update();

    private:

        /**
         * Status Part
         */
        gcn::Label *lvlLabel, *gpLabel, *hpLabel, *hpValueLabel, *mpLabel, *mpValueLabel;
        gcn::Label *xpLabel, *xpValueLabel, *jobXpLabel, *jobValueLabel;
        ProgressBar *hpBar, *mpBar;
        ProgressBar *xpBar, *jobXpBar;

        /**
         * Caracteristics Part
         */
        gcn::Label *statsTitleLabel, *statsTotalLabel, *statsCostLabel;

        /**
         * Derived Statistics captions
         */
        gcn::Label *statsAttackLabel, *statsDefenseLabel, *statsMagicAttackLabel;
        gcn::Label *statsMagicDefenseLabel, *statsAccuracyLabel, *statsEvadeLabel;
        gcn::Label *statsReflexLabel;

        gcn::Label *statsAttackPoints, *statsDefensePoints, *statsMagicAttackPoints;
        gcn::Label *statsMagicDefensePoints, *statsAccuracyPoints, *statsEvadePoints;
        gcn::Label *statsReflexPoints;

        /** 
         * Stats captions.
         */
        gcn::Label *statsLabel[6];
        gcn::Label *pointsLabel[6];
        gcn::Label *statsDisplayLabel[6];
        gcn::Label *remainingStatsPointsLabel;

        /** 
         * Stats buttons.
         */
        gcn::Button *statsButton[6];
};

extern StatusWindow *statusWindow;

#endif
