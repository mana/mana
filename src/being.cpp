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

#include "being.h"

#include <sstream>

#include <guichan/imagefont.hpp>

#include "game.h"
#include "graphics.h"
#include "log.h"
#include "map.h"

#include "graphic/spriteset.h"

#include "gui/gui.h"

#include "net/messageout.h"
#include "net/network.h"
#include "net/protocol.h"

#include "resources/resourcemanager.h"

extern Being* autoTarget;
extern std::map<int, Spriteset*> monsterset;

Being *player_node = NULL;

std::list<Being*> beings;

PATH_NODE::PATH_NODE(unsigned short x, unsigned short y):
    x(x), y(y)
{
}

Being* createBeing(unsigned int id, unsigned short job, Map *map)
{
    Being *being = new Being;

    being->setId(id);
    being->job = job;
    being->setMap(map);

    beings.push_back(being);

    // If the being is a player, request the name
    if (being->getType() == Being::PLAYER)
    {
        MessageOut outMsg;
        outMsg.writeShort(0x0094);
        outMsg.writeLong(being->getId());//readLong(2));
        writeSet(6);
    }
    // If the being is a monster then load the monsterset
    else if (being->job >= 1002 &&
            monsterset.find(being->job - 1002) == monsterset.end())
    {
        std::stringstream filename;

        filename << "graphics/sprites/monster" << (being->job - 1002) << ".png";
        logger->log("%s",filename.str().c_str());

        Image *monsterbitmap =
            ResourceManager::getInstance()->getImage(filename.str());

        if (!monsterbitmap) {
            logger->error("Unable to load monster.png");
        } else {
            monsterset[being->job - 1002] = new Spriteset(monsterbitmap, 60, 60);
        }
    }

    return being;
}

void remove_node(Being *being)
{
    delete being;
    beings.remove(being);
}

Being *findNode(unsigned int id)
{
    std::list<Being*>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        Being *being = (*i);
        if (being->getId() == id) {
            return being;
        }
    }
    return NULL;
}

Being *findNode(unsigned short x, unsigned short y)
{
    std::list<Being*>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        Being *being = (*i);
        // Return being if found and it is not a dead monster
        if (being->x == x && being->y == y && being->action != Being::MONSTER_DEAD) {
            return being;
        }
    }
    return NULL;
}

Being* findNode(unsigned short x, unsigned short y, Being::Type type)
{
    std::list<Being *>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        Being *being = (*i);
        // Check if is a NPC (only low job ids)
        if (being->x == x && being->y == y &&
                being->getType() == type && being->action != Being::MONSTER_DEAD)
        {
            return being;
        }
    }
    return NULL;
}

class BeingCompare {
    public:
        bool operator() (const Being *a, const Being *b) const {
            return a->y < b->y;
        }
};

void sort() {
    beings.sort(BeingCompare());
}

Being::Being():
    job(0),
    x(0), y(0), direction(SOUTH),
    action(0), frame(0),
    speech_color(0),
    walk_time(0),
    speed(150),
    emotion(0), emotion_time(0),
    text_x(0), text_y(0),
    aspd(350),
    m_weapon(0),
    m_id(0),
    map(0),
    hairStyle(1), hairColor(1),
    speech_time(0),
    damage_time(0),
    showSpeech(false), showDamage(false)
{
}

Being::~Being()
{
    clearPath();
}

void Being::setDestination(int destX, int destY)
{
    if (!map)
        return;

    setPath(map->findPath(x, y, destX, destY));
}

void Being::clearPath()
{
    path.clear();
}

void Being::setPath(std::list<PATH_NODE> path)
{
    this->path = path;

    if (action != WALK && action != DEAD)
    {
        nextStep();
        walk_time = tick_time;
    }
}

void Being::setHairColor(int color)
{
    hairColor = color;
    if (hairColor < 1 || hairColor > NR_HAIR_COLORS + 1)
    {
        hairColor = 1;
    }
}

void Being::setHairStyle(int style)
{
    hairStyle = style;
    if (hairStyle < 1 || hairStyle > NR_HAIR_STYLES)
    {
        hairStyle = 1;
    }
}

unsigned short Being::getHairColor()
{
    return hairColor;
}

unsigned short Being::getHairStyle()
{
    return hairStyle;
}

void Being::setSpeech(const std::string &text, int time)
{
    speech = text;
    speech_time = tick_time;
    showSpeech = true;
}

void Being::setDamage(short amount, int time)
{
    if (!amount) {
        damage = "miss";
    } else {
        std::stringstream damageString;
        damageString << amount;
        damage = damageString.str();
    }
    damage_time = tick_time;
    showDamage = true;
}

void Being::setMap(Map *map)
{
    this->map = map;
}

void Being::nextStep()
{
    if (!path.empty())
    {
        PATH_NODE node = path.front();
        path.pop_front();

        int oldX = x;
        int oldY = y;
        int newX = node.x;
        int newY = node.y;

        if (newX > oldX) {
            if (newY > oldY)      direction = SE;
            else if (newY < oldY) direction = NE;
            else                  direction = EAST;
        }
        else if (newX < oldX) {
            if (newY > oldY)      direction = SW;
            else if (newY < oldY) direction = NW;
            else                  direction = WEST;
        }
        else {
            if (newY > oldY)      direction = SOUTH;
            else if (newY < oldY) direction = NORTH;
        }

        x = newX;
        y = newY;
        action = WALK;
        walk_time += speed / 10;
    } else {
        action = STAND;
    }
    frame = 0;
}

void Being::logic()
{
    if (getType() == PLAYER)
    {
        switch (action) {
            case WALK:
                frame = (get_elapsed_time(walk_time) * 4) / speed;
                if (frame >= 4) {
                    nextStep();
                }
                break;
            case ATTACK:
                frame = (get_elapsed_time(walk_time) * 4) / aspd;
                if (frame >= 4) {
                    nextStep();
                    if (autoTarget && this == player_node) {
                        attack(autoTarget);
                    }
                }
                break;
        }

        if (emotion != 0) {
            emotion_time--;
            if (emotion_time == 0) {
                emotion = 0;
            }
        }
    }

    if (get_elapsed_time(speech_time) > 5000) {
        showSpeech = false;
    }
    if (get_elapsed_time(damage_time) > 3000) {
        showDamage = false;
    }
}

void Being::drawSpeech(Graphics *graphics)
{
    // Draw speech above this being
    if (showSpeech) {
        graphics->setFont(speechFont);
        graphics->drawText(speech,
                text_x + 18, text_y - 60,
                gcn::Graphics::CENTER);

        // Backing to default font
        graphics->setFont(gui->getFont());
    }
    if (showDamage) {
        // Selecting the right color
        if (damage == "miss")
        {
            graphics->setFont(hitYellowFont);
        }
        else if (getType() == MONSTER)
        {
            graphics->setFont(hitBlueFont);
        }
        else
        {
            graphics->setFont(hitRedFont);
        }

        int textX = 0;
        int textY = 0;
        if (getType() == PLAYER) {
            textX = 16;
            textY = 70;
        }
        else {
            textX = 60;
            textY = 0;
        }

        graphics->drawText(damage,
                text_x + textX,
                text_y - textY - get_elapsed_time(damage_time) / 100,
                gcn::Graphics::CENTER);

        // Backing to default font
        graphics->setFont(gui->getFont());
    }
}

Being::Type Being::getType()
{
    if (job < 10) {
        return PLAYER;
    } else if (job >= 100 & job < 200) {
        return NPC;
    } else if (job >= 1000 && job < 1200) {
        return MONSTER;
    } else {
        return UNKNOWN;
    }
}

void Being::setWeapon(unsigned short weapon)
{
    m_weapon = weapon;
}

void Being::setWeaponById(unsigned short weapon)
{
    switch (weapon)
    {
    case 529: // iron arrows
    case 1199: // arrows
        break;

    case 1200: // bow
    case 530: // short bow
    case 545: // forest bow
        setWeapon(2);
        break;

    case 521: // sharp knife
    case 522: // dagger
    case 536: // short sword
    case 1201: // knife
        setWeapon(1);
        break;

    case 0: // unequip
        setWeapon(0);
        break;

    default:
        logger->log("unknown item equiped : %d", weapon);
    }
}

void Being::setId(unsigned int id)
{
    m_id = id;
}
