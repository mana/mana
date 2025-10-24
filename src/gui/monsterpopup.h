/*
 *  The Mana Client
 *  Copyright (C) 2025  The Mana Developers
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

#include "gui/widgets/popup.h"

#include "compoundsprite.h"

class Label;
class Graphics;
class BeingInfo;
struct SpriteDisplay;

/**
 * A popup that displays a monster sprite (first frame of idle looking-down)
 * along with its name.
 */
class MonsterPopup : public Popup
{
public:
    MonsterPopup();
    ~MonsterPopup() override;

    void setMonster(const BeingInfo &monster);

    void draw(gcn::Graphics *graphics) override;

    void mouseMoved(gcn::MouseEvent &event) override;

private:
    void loadSpriteDisplay(const SpriteDisplay &display);
    void relayout();

    Label *mMonsterName = nullptr;

    // The monster sprite assembled from one or multiple Sprite layers.
    CompoundSprite mSprite;

    // Extra top margin to accommodate negative frame offsets
    int mTopOvershoot = 0;

    // Cached metrics for positioning the sprite and text.
    int mSpriteWidth = 0;
    int mSpriteHeight = 0;
};
