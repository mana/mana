/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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

#ifndef VARIABLEDATA_H
#define VARIABLEDATA_H

#include <string>

class ActorSprite;
class Item;

class VariableData
{
    public:
    enum DataType
    {
        DATA_NONE,
        DATA_INT,
        DATA_STRING,
        DATA_FLOAT,
        DATA_BOOL,
        DATA_ITEM,
        DATA_ACTOR
    };

    virtual ~VariableData() {}

    virtual int getType() const = 0;
};

class IntData : public VariableData
{
public:
    IntData(int value) { mData = value; }

    int getData() const { return mData; }

    int getType() const { return DATA_INT; }

private:
    int mData;
};

class StringData : public VariableData
{
public:
    StringData(const std::string &value) { mData = value; }

    const std::string &getData() const { return mData; }

    int getType() const { return DATA_STRING; }

private:
    std::string mData;
};

class FloatData : public VariableData
{
public:
    FloatData(double value) { mData = value; }

    double getData() const { return mData; }

    int getType() const { return DATA_FLOAT; }

private:
    double mData;
};

class BoolData : public VariableData
{
public:
    BoolData(bool value) { mData = value; }

    bool getData() const { return mData; }

    int getType() const { return DATA_BOOL; }

private:
    bool mData;
};

class ItemData : public VariableData
{
public:
    ItemData(Item *value) { mData = value; }

    Item *getData() const { return mData; }

    int getType() const { return DATA_ITEM; }

private:
    Item *mData;
};

class ActorData : public VariableData
{
public:
    ActorData(ActorSprite *value) { mData = value; }

    ActorSprite *getData() const { return mData; }

    int getType() const { return DATA_ACTOR; }

private:
    ActorSprite *mData;
};

#endif
