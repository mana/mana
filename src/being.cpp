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
#include "net/protocol.h"

Being *player_node = NULL;

std::list<Being*> beings;

PATH_NODE::PATH_NODE(unsigned short x, unsigned short y):
    x(x), y(y)
{
}

void add_node(Being *being) {
    beings.push_back(being);
}

void remove_node(unsigned int id) {
    std::list<Being *>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        if ((*i)->id == id) {
            delete (*i);
            beings.erase(i);
            return;
        }
    }
}

unsigned int find_npc(unsigned short x, unsigned short y) {
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

unsigned int find_monster(unsigned short x, unsigned short y) {
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

Being *find_node(unsigned int id) {
    std::list<Being*>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++) {
        Being *being = (*i);
        if (being->id == id) {
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
    speech_time(0),
    id(0), job(0),
    x(0), y(0), destX(0), destY(0), direction(0),
    type(0), action(0), frame(0),
    speech_color(0),
    walk_time(0),
    speed(150),
    emotion(0), emotion_time(0),
    text_x(0), text_y(0),
    hair_style(1), hair_color(1),
    weapon(0)
{
}

Being::~Being() {
    clearPath();
}

void Being::clearPath() {
    path.clear();
}

void Being::setPath(std::list<PATH_NODE> path)
{
    this->path = path;
    nextStep();
}

void Being::setHairColor(int color)
{
    hair_color = color;
}

void Being::setHairStyle(int style)
{
    hair_style = style;
}

void Being::setSpeech(const std::string &text, int time)
{
    speech = text;
    speech_time = time;
}

void Being::nextStep()
{
    if (!path.empty()) {
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
            else                    direction = EAST;
        }
        else if (newX < oldX) {
            if (newY > oldY)      direction = SW;
            else if (newY < oldY) direction = NW;
            else                    direction = WEST;
        }
        else {
            if (newY > oldY)      direction = SOUTH;
            else if (newY < oldY) direction = NORTH;
        }

        x = newX;
        y = newY;
        action = WALK;
    } else {
        action = STAND;
        if (this == player_node) {
            walk_status = 0;
        }
    }
    frame = 0;
    walk_time = tick_time;
}

void Being::drawSpeech(Graphics *graphics)
{
    // Draw speech above this being
    if (speech_time > 0) {
        //if (being->speech_color == makecol(255, 255, 255)) {
        //    guiGraphics->drawText(being->speech,
        //            being->text_x + 16, being->text_y - 60,
        //            gcn::Graphics::CENTER);
        //}
        //else {
        graphics->drawText(speech,
                text_x + 60, text_y - 60,
                gcn::Graphics::CENTER);
        //}
    }
}

void Being::tick()
{
    if (speech_time > 0) {
        speech_time--;
    }
}
