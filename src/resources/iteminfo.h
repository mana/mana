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

#ifndef _TMW_ITEMINFO_H_
#define _TMW_ITEMINFO_H_

#include <map>
#include <string>
#include <vector>

#include "spritedef.h"

enum EquipmentSoundEvent
{
    EQUIP_EVENT_STRIKE,
    EQUIP_EVENT_HIT
};

/**
 * Defines a class for storing item infos. This includes information used when
 * the item is equipped.
 */
class ItemInfo
{
    public:
        /**
         * Constructor.
         */
        ItemInfo():
            mType(0),
            mWeight(0),
            mView(0),
            mAttackType(ACTION_DEFAULT)
        {
        }

        void setName(const std::string &name)
        { mName = name; }

        const std::string& getName() const
        { return mName; }

        void setImageName(const std::string &imageName)
        { mImageName = imageName; }

        const std::string& getImageName() const
        { return mImageName; }

        void setDescription(const std::string &description)
        { mDescription = description; }

        const std::string& getDescription() const
        { return mDescription; }

        void setEffect(const std::string &effect)
        { mEffect = effect; }

        const std::string&
        getEffect() const { return mEffect; }

        void setType(short type)
        { mType = type; }

        short getType() const
        { return mType; }

        void setWeight(short weight)
        { mWeight = weight; }

        short getWeight() const
        { return mWeight; }

        void setView(int view)
        { mView = view; }

        void setSprite(const std::string &animationFile, int gender)
        { mAnimationFiles[gender] = animationFile; }

        const std::string& getSprite(int gender) const;

        void setWeaponType(int);

        SpriteAction getAttackType() const
        { return mAttackType; }

        void addSound(EquipmentSoundEvent event, const std::string &filename);

        const std::string& getSound(EquipmentSoundEvent event) const;

    protected:
        std::string mImageName;        /**< The filename of the icon image. */
        std::string mName;
        std::string mDescription;      /**< Short description. */
        std::string mEffect;           /**< Description of effects. */
        char mType;                    /**< Item type. */
        short mWeight;                 /**< Weight in grams. */
        int mView;                     /**< Item ID of how this item looks. */

        // Equipment related members
        SpriteAction mAttackType;      /**< Attack type, in case of weapon. */

        /** Maps gender to sprite filenames. */
        std::map<int, std::string> mAnimationFiles;

        /** Stores the names of sounds to be played at certain event. */
        std::map< EquipmentSoundEvent, std::vector<std::string> > mSounds;
};

#endif
