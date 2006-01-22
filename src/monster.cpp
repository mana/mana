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

#include "monster.h"

#include <sstream>

#include "game.h"
#include "graphics.h"
#include "log.h"

#include "graphic/spriteset.h"

#include "resources/resourcemanager.h"

extern std::map<int, Spriteset*> monsterset;

Monster::Monster(Uint32 id, Uint16 job, Map *map):
    Being(id, job, map)
{
    // Load monster spriteset, if necessary
    if (monsterset.find(job - 1002) == monsterset.end())
    {
        std::stringstream filename;

        filename << "graphics/sprites/monster" << (job - 1002) << ".png";
        logger->log("%s",filename.str().c_str());

        Spriteset *tmp = ResourceManager::getInstance()->createSpriteset(
                filename.str(), 60, 60);
        if (!tmp) {
            logger->error("Unable to load monster spriteset!");
        } else {
            monsterset[job - 1002] = tmp;
        }
    }
}

void Monster::logic()
{
    if (action != STAND)
    {
        mFrame = (get_elapsed_time(walk_time) * 4) / mWalkSpeed;

        if (mFrame >= 4 && action != MONSTER_DEAD)
        {
            nextStep();
        }
    }

    Being::logic();
}

Being::Type Monster::getType() const
{
    return MONSTER;
}

void Monster::draw(Graphics *graphics, int offsetX, int offsetY)
{
    unsigned char dir = direction / 2;
    int px = mPx + offsetX;
    int py = mPy + offsetY;
    int frame;

    if (mFrame >= 4)
    {
        mFrame = 3;
    }

    frame = action;
    if (action != MONSTER_DEAD) {
        frame += mFrame;
    }

    graphics->drawImage(
            monsterset[job-1002]->spriteset[dir + 4 * frame],
            px - 12, py - 25);

    Being::draw(graphics, offsetX, offsetY);
}
