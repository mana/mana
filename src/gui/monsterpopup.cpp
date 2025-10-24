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

#include "gui/monsterpopup.h"

#include "configuration.h"
#include "graphics.h"
#include "sprite.h"

#include "gui/gui.h"
#include "gui/widgets/label.h"

#include "resources/beinginfo.h"
#include "resources/spritedef.h"

#include <guichan/font.hpp>

#include <algorithm>

MonsterPopup::MonsterPopup()
    : Popup("MonsterPopup")
{
    setMinHeight(boldFont->getHeight());

    mMonsterName = new Label;
    mMonsterName->setFont(boldFont);
    mMonsterName->setPosition(0, 0);
    add(mMonsterName);

    addMouseListener(this);
}

MonsterPopup::~MonsterPopup() = default;

/**
 * Sets the monster info to display.
 */
void MonsterPopup::setMonster(const BeingInfo &monster)
{
    mMonsterName->setCaption(monster.name);
    mMonsterName->adjustSize();

    loadSpriteDisplay(monster.display);
    relayout();
}

/**
 * Load sprites according to the provided display.
 * Ensures first frame of the default "stand" action is available.
 */
void MonsterPopup::loadSpriteDisplay(const SpriteDisplay &display)
{
    mSprite.clear();
    int minOffsetY = 0;

    // Build the compound sprite from all layers and compute top overshoot
    for (const auto &ref : display.sprites)
    {
        const std::string file = paths.getStringValue("sprites") + ref.sprite;
        if (Sprite *s = Sprite::load(file, ref.variant))
        {
            // Ensure we are using idle-looking-down to query offsets
            s->setDirection(DIRECTION_DOWN);
            s->play(SpriteAction::STAND);

            minOffsetY = std::min(minOffsetY, s->getOffsetY());

            mSprite.add(s);
        }
    }

    // Extra space above sprite to avoid clipping when offsetY is negative
    mTopOvershoot = minOffsetY < 0 ? -minOffsetY : 0;

    // Ensure compound uses the same action/direction
    mSprite.setDirection(DIRECTION_DOWN);
    mSprite.play(SpriteAction::STAND);

    // Cache metrics
    mSpriteWidth = mSprite.getWidth();
    mSpriteHeight = mSprite.getHeight();
}

/**
 * Recalculate positions and overall popup size based on current content.
 */
void MonsterPopup::relayout()
{
    const int nameWidth = mMonsterName->getWidth();
    const int nameHeight = mMonsterName->getHeight();

    const int contentWidth = std::max(mSpriteWidth, nameWidth);
    const int contentHeight = mTopOvershoot + mSpriteHeight + nameHeight;

    // Center the name below the sprite (account for top overshoot)
    mMonsterName->setPosition((contentWidth - nameWidth) / 2, mTopOvershoot + mSpriteHeight);

    setContentSize(contentWidth, contentHeight);

    // Center the name below the sprite
    const auto childrenArea = getChildrenArea();
    mMonsterName->setPosition((childrenArea.width - nameWidth) / 2, childrenArea.height - nameHeight);
}

void MonsterPopup::draw(gcn::Graphics *g)
{
    // Draw frame and children (labels, etc.)
    Popup::draw(g);

    if (mSprite.size() == 0)
        return;

    auto *graphics = static_cast<Graphics*>(g);

    // Draw the sprite centered horizontally
    const int spriteX = getPadding() + getChildrenArea().width / 2;
    const int spriteY = getPadding() + mTopOvershoot + mSpriteHeight;

    mSprite.draw(graphics, spriteX, spriteY);
}

void MonsterPopup::mouseMoved(gcn::MouseEvent &event)
{
    Popup::mouseMoved(event);

    // Hide popup when hovered (same UX as ItemPopup)
    setVisible(false);
}
