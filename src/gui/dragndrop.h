/*
 *  The Mana Client
 *  Copyright (C) 2009-2026  The Mana Developers
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

#pragma once

#include "item.h"

#include <cstdint>
#include <memory>

class Graphics;

struct Drag;

enum class DragResult : std::uint8_t
{
    Accepted,
    Canceled,
    Ignored
};

class DragSource
{
    public:
        virtual ~DragSource() = default;
        virtual void dragFinished(const Drag &drag, DragResult result) = 0;
};

class DragTarget
{
    public:
        virtual ~DragTarget() = default;
        virtual bool handleDrop(const Drag &drag, int absX, int absY) = 0;
};

struct Drag
{
    enum class SourceType : std::uint8_t
    {
        Inventory,
        Storage,
        Equipment,
        Outfit,
        ItemShortcut
    };

    Drag() = default;

    Drag(SourceType sourceType, Item *item, DragSource *source,
         int sourceIndex = -1)
        : sourceType(sourceType)
        , item(item ? std::make_unique<Item>(*item) : nullptr)
        , source(source)
        , sourceIndex(sourceIndex)
    {}

    static Drag fromItemShortcut(Item *item, DragSource *source,
                                 int sourceIndex)
    {
        return { SourceType::ItemShortcut, item, source, sourceIndex };
    }

    static Drag fromEquipment(Item *item, DragSource *source, int sourceIndex)
    {
        return { SourceType::Equipment, item, source, sourceIndex };
    }

    static Drag fromOutfit(Item *item, DragSource *source, int sourceIndex)
    {
        return { SourceType::Outfit, item, source, sourceIndex };
    }

    void draw(Graphics *graphics, int mouseX, int mouseY) const;

    SourceType sourceType = SourceType::Inventory;
    std::unique_ptr<Item> item;
    DragSource *source = nullptr;
    int sourceIndex = -1;
};
