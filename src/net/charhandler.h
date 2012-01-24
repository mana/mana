/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CHARHANDLER_H
#define CHARHANDLER_H

#include "localplayer.h"
#include "playerinfo.h"

#include <iosfwd>
#include <vector>

class CharCreateDialog;
class CharSelectDialog;

namespace Net {

/**
 * A structure to hold information about a character.
 */
struct Character
{
    Character() :
        slot(0),
        dummy(0)
    {
    }

    ~Character()
    {
        delete dummy;
    }

    int slot;            /**< The index in the list of characters */
    LocalPlayer *dummy;  /**< A dummy representing this character */
    PlayerInfoBackend data;
};

typedef std::list<Character*> Characters;

class CharHandler
{
    public:
        virtual ~CharHandler() {}

        virtual void setCharSelectDialog(CharSelectDialog *window) = 0;

        virtual void setCharCreateDialog(CharCreateDialog *window) = 0;

        virtual void requestCharacters() = 0;

        virtual void chooseCharacter(Net::Character *character) = 0;

        virtual void newCharacter(const std::string &name, int slot,
                                  bool gender, int hairstyle, int hairColor,
                                  const std::vector<int> &stats) = 0;

        virtual void deleteCharacter(Net::Character *character) = 0;

        virtual void switchCharacter() = 0;

        virtual unsigned int baseSprite() const = 0;

        virtual unsigned int hairSprite() const = 0;

        virtual unsigned int maxSprite() const = 0;

    protected:
        CharHandler():
            mSelectedCharacter(0),
            mCharSelectDialog(0),
            mCharCreateDialog(0)
        {}

        void updateCharSelectDialog();
        void unlockCharSelectDialog();

        /** The list of available characters. */
        Net::Characters mCharacters;

        /** The selected character. */
        Net::Character *mSelectedCharacter;

        CharSelectDialog *mCharSelectDialog;
        CharCreateDialog *mCharCreateDialog;
};

} // namespace Net

#endif // CHARHANDLER_H
