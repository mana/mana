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

#include "equipment.h"
#include "game.h"
#include "graphics.h"

#include "graphic/spriteset.h"

#include "gui/gui.h"

extern std::vector<Spriteset *> hairset;
extern Spriteset *playerset;
extern std::vector<Spriteset *> weaponset;
extern Spriteset *equipmentset;

signed char hairtable[19][4][2] = {
    // S(x,y)    W(x,y)   N(x,y)   E(x,y)
    { {-5, -4}, {-6, -4}, {-6,  0}, {-4, -4} }, // STAND
    { {-5, -3}, {-6, -4}, {-6,  0}, {-4, -4} }, // WALK 1st frame
    { {-5, -4}, {-6, -3}, {-6, -1}, {-4, -3} }, // WALK 2nd frame
    { {-5, -4}, {-6, -4}, {-6, -1}, {-4, -4} }, // WALK 3rd frame
    { {-5, -3}, {-6, -4}, {-6,  0}, {-4, -4} }, // WALK 4th frame
    { {-5, -4}, {-6, -3}, {-6, -1}, {-4, -3} }, // WALK 5th frame
    { {-5, -4}, {-6, -4}, {-6, -1}, {-4, -4} }, // WALK 6th frame
    { {-5,  8}, {-1,  5}, {-6,  8}, {-9,  5} },  // SIT
    { {16, 21}, {16, 21}, {16, 21}, {16, 21} }, // DEAD
    { {-5, -2}, {-2, -5}, {-6,  0}, {-7, -5} }, // ATTACK 1st frame
    { {-5, -3}, {-2, -6}, {-6,  0}, {-7, -6} }, // ATTACK 2nd frame
    { {-5,  0}, {-6, -3}, {-6,  0}, {-4, -3} }, // ATTACK 3rd frame
    { {-5,  1}, {-7, -2}, {-6,  2}, {-3, -2} }, // ATTACK 4th frame
    { {-5, -3}, {-3, -4}, {-6,  0}, {-7, -4} }, // BOW_ATTACK 1st frame
    { {-5, -3}, {-3, -4}, {-6,  0}, {-7, -4} }, // BOW_ATTACK 2nd frame
    { {-5, -3}, {-3, -4}, {-7,  0}, {-7, -4} }, // BOW_ATTACK 3rd frame
    { {-5, -2}, {-1, -5}, {-7,  1}, {-9, -5} }, // BOW_ATTACK 4th frame
    { {-5, -3}, {-1, -5}, {-7,  0}, {-9, -5} }, // BOW_ATTACK 5th frame
    { { 0,  0}, { 0,  0}, { 0,  0}, { 0,  0} }  // ?? HIT
};

unsigned char hairframe[4][20] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 3, 3, 3, 3, 3, 3, 3, 3, 8, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 },
    { 4, 4, 4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 },
    { 5, 5, 5, 5, 5, 5, 5, 5, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 }
};

Player::Player(Uint32 id, Uint16 job, Map *map):
    Being(id, job, map)
{
}

void Player::logic()
{
    switch (mAction) {
        case WALK:
            mFrame = (get_elapsed_time(mWalkTime) * 6) / mWalkSpeed;
            if (mFrame >= 6) {
                nextStep();
            }
            break;

        case ATTACK:
            int frames = 4;
            if (getWeapon() == 2)
            {
                frames = 5;
            }
            mFrame = (get_elapsed_time(mWalkTime) * frames) / mAttackSpeed;
            if (mFrame >= frames) {
                nextStep();
            }
            break;
    }



    Being::logic();
}

Being::Type Player::getType() const
{
    return PLAYER;
}

void Player::draw(Graphics *graphics, int offsetX, int offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;
    int frame = mAction;

    frame = mAction;

    if (mAction != SIT && mAction != DEAD)
    {
        frame += mFrame;
    }

    if (mAction == ATTACK && getWeapon() > 0)
    {
        if (getWeapon() == 2)
        {
            frame += 4;
        }
    }

    unsigned char dir = 0;
    while (!(mDirection & (1 << dir))) dir++;

    graphics->drawImage(playerset->get(frame + 18 * dir),
            px - 16, py - 32);

    Item *item = mEquipment->getEquipment(3);
    if (item)
    {
        graphics->drawImage(equipmentset->get(frame + 18 * dir),
                px - 16, py - 32);
    }

    if (getWeapon() != 0 && mAction == ATTACK)
    {
        int frames = 4;
        if (getWeapon() == 2)
        {
            frames = 5;
        }
        Image *image = weaponset[getWeapon() - 1]->get(mFrame + frames * dir);
        graphics->drawImage(image, px - 16, py - 32);
    }

    if (getHairColor() <= NR_HAIR_COLORS)
    {
        int hf = 9 * (getHairColor() - 1) + hairframe[dir][frame];

        graphics->drawImage(hairset[getHairStyle() - 1]->get(hf),
                px + 1 + hairtable[frame][dir][0],
                py - 33 + hairtable[frame][dir][1]);
    }
}

void
Player::drawName(Graphics *graphics, Sint32 offsetX, Sint32 offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    graphics->setFont(speechFont);
    graphics->drawText(mName, px + 15, py + 30, gcn::Graphics::CENTER);
}
