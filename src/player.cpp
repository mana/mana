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

#include "player.h"

#include "game.h"
#include "graphics.h"

#include "graphic/spriteset.h"

#include "gui/gui.h"

extern Spriteset *hairset;
extern Spriteset *playerset;
extern Spriteset *weaponset;

signed char hairtable[16][4][2] = {
    // S(x,y)    W(x,y)   N(x,y)   E(x,y)
    { { 0,  0}, {-1, 2}, {-1, 2}, { 0, 2} }, // STAND
    { { 0,  2}, {-2, 3}, {-1, 2}, { 1, 3} }, // WALK 1st frame
    { { 0,  3}, {-2, 4}, {-1, 3}, { 1, 4} }, // WALK 2nd frame
    { { 0,  1}, {-2, 2}, {-1, 2}, { 1, 2} }, // WALK 3rd frame
    { { 0,  2}, {-2, 3}, {-1, 2}, { 1, 3} }, // WALK 4th frame
    { { 0,  1}, { 1, 2}, {-1, 3}, {-2, 2} }, // ATTACK 1st frame
    { { 0,  1}, {-1, 2}, {-1, 3}, { 0, 2} }, // ATTACK 2nd frame
    { { 0,  2}, {-4, 3}, { 0, 4}, { 3, 3} }, // ATTACK 3rd frame
    { { 0,  2}, {-4, 3}, { 0, 4}, { 3, 3} }, // ATTACK 4th frame
    { { 0,  0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 1st frame
    { { 0,  0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 2nd frame
    { { 0,  0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 3rd frame
    { { 0,  0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 4th frame
    { { 0,  4}, {-1, 6}, {-1, 6}, { 0, 6} }, // SIT
    { { 0,  0}, { 0, 0}, { 0, 0}, { 0, 0} }, // ?? HIT
    { { 0, 16}, {-1, 6}, {-1, 6}, { 0, 6} }  // DEAD
};

Player::Player(Uint32 id, Uint16 job, Map *map):
    Being(id, job, map)
{
}

void Player::logic()
{
    switch (action) {
        case WALK:
            mFrame = (get_elapsed_time(walk_time) * 4) / mWalkSpeed;
            break;

        case ATTACK:
            mFrame = (get_elapsed_time(walk_time) * 4) / aspd;
            break;
    }

    if (mFrame >= 4) {
        nextStep();
    }

    Being::logic();
}

Being::Type Player::getType() const
{
    return PLAYER;
}

void Player::draw(Graphics *graphics, int offsetX, int offsetY)
{
    unsigned char dir = direction / 2;
    int px = mPx + offsetX;
    int py = mPy + offsetY;
    int frame = action;

    frame = action;

    if (action != SIT && action != DEAD)
    {
        frame += mFrame;
    }

    if (action == ATTACK && getWeapon() > 0)
    {
        frame += 4 * (getWeapon() - 1);
    }

    graphics->drawImage(playerset->spriteset[frame + 16 * dir],
            px - 16, py - 32);

    if (getWeapon() != 0 && action == ATTACK)
    {
        Image *image = weaponset->spriteset[
            16 * (getWeapon() - 1) + 4 * mFrame + dir];

        graphics->drawImage(image, px - 64, py - 80);
    }

    if (getHairColor() <= NR_HAIR_COLORS)
    {
        int hf = getHairColor() - 1 + 10 * (dir + 4 *
                (getHairStyle() - 1));

        graphics->drawImage(hairset->spriteset[hf],
                px - 2 + 2 * hairtable[frame][dir][0],
                py - 50 + 2 * hairtable[frame][dir][1]);
    }

    // Draw player name
    if (getType() != LOCALPLAYER) {
        graphics->setFont(speechFont);
        graphics->drawText(mName, px + 15, py + 30, gcn::Graphics::CENTER);
    }

    Being::draw(graphics, offsetX, offsetY);
}
