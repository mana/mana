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
#include "game.h"
#include "engine.h"
#include "net/protocol.h"
#include "net/network.h"
#include "resources/resourcemanager.h"

Being *player_node = NULL;

std::list<Being*> beings;

PATH_NODE::PATH_NODE(unsigned short x, unsigned short y):
    x(x), y(y)
{
}

void add_node(Being *being)
{
    beings.push_back(being);
    // If the being is a player, request the name
    if (being-> job < 10) {
        WFIFOW(0) = net_w_value(0x0094);
        WFIFOL(2) = net_l_value(RFIFOL(2));
        WFIFOSET(6);
    }
    // If the being is a monster then load the monsterset
    else if (being->job >= 1002 && monsterset[being->job - 1002] == NULL) {
        std::stringstream filename;
        filename << "graphics/sprites/monster" << (being->job - 1002) << ".png";
        logger.log("%s",filename.str().c_str());
        ResourceManager *resman = ResourceManager::getInstance();
        Image *monsterbitmap = resman->getImage(filename.str());
        if (!monsterbitmap) {
            logger.error("Unable to load monster.png");
        }
        else {
            monsterset[being->job - 1002] = new Spriteset(monsterbitmap, 60, 60);
        }
    }
}

void remove_node(unsigned int id)
{
    std::list<Being *>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        if ((*i)->id == id) {
            delete (*i);
            beings.erase(i);
            return;
        }
    }
}

unsigned int findNpc(unsigned short x, unsigned short y)
{
    std::list<Being *>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        Being *being = (*i);
        // Check if is a NPC (only low job ids)
        if (being->job >= 46 && being->job <= 125 &&
                being->x == x && being->y == y)
        {
            return being->id;
        }
    }
    return 0;
}

unsigned int findPlayer(unsigned short x, unsigned short y)
{
    std::list<Being *>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        Being *being = (*i);
        // Check if is a player
        if (being->job < 10 && being->x == x && being->y == y) {
            return being->id;
        }
    }
    return 0;
}

unsigned int findMonster(unsigned short x, unsigned short y)
{
    std::list<Being*>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        Being *being = (*i);
        // Check if is a MONSTER
        if (being->job > 200 &&
                being->x == x &&
                being->y == y)
        {
            return being->id;
        }
    }
    return 0;
}

Being *findNode(unsigned int id)
{
    std::list<Being*>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        Being *being = (*i);
        if (being->id == id) {
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
        if (being->x == x && being->y == y) {
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
    id(0), job(0),
    x(0), y(0), destX(0), destY(0), direction(0),
    type(0), action(0), frame(0),
    speech_color(0),
    walk_time(0),
    speed(150),
    emotion(0), emotion_time(0),
    text_x(0), text_y(0),
    weapon(0),
    speech_time(0),
    damage_time(0),
    showSpeech(false), showDamage(false),
    aspd(350),
    hairStyle(1), hairColor(1)
{
    strcpy(name, "");
}

Being::~Being()
{
    clearPath();
}

void Being::clearPath()
{
    path.clear();
}

void Being::setPath(std::list<PATH_NODE> path)
{
    this->path = path;

    if (action != WALK)
    {
        nextStep();
        walk_time = tick_time;
    }
}

void Being::setDestination(int destX, int destY)
{
    this->destX = destX;
    this->destY = destY;
    setPath(tiledMap->findPath(x, y, destX, destY));
}

void Being::setHairColor(int color)
{
    hairColor = color;
    if (hairColor < 1 || hairColor > 11)
    {
        hairColor = 1;
    }
}

void Being::setHairStyle(int style)
{
    hairStyle = style;
    if (hairStyle < 1 || hairStyle > 4)
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

void Being::setDamage(const std::string &text, int time)
{
    damage = text;
    damage_time = tick_time;
    showDamage = true;
}

void Being::setName(char *text)
{
    strcpy(name, text);
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
        direction = 0;

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
        if (this == player_node) {
            walk_status = 1;
        }
    } else {
        action = STAND;
        if (this == player_node) {
            walk_status = 0;
        }
    }
    frame = 0;
}

void Being::drawSpeech(Graphics *graphics)
{
    // Draw speech above this being
    if (showSpeech) {
        graphics->drawText(speech,
                text_x + 20, text_y - 60,
                gcn::Graphics::CENTER);
    }
    if (showDamage) {
        graphics->drawText(damage,
                           text_x + 60,
                           text_y - 60 - get_elapsed_time(damage_time) / 100,
                           gcn::Graphics::CENTER);
    }
}

bool Being::isPlayer()
{
    return job < 10;
}

bool Being::isNpc()
{
    return job > 45 && job < 126;
}

bool Being::isMonster()
{
    return job > 200;
}
