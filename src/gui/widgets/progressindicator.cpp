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

#include "progressindicator.h"

#include "graphics.h"
#include "gui/gui.h"
#include "simpleanimation.h"

#include "resources/animation.h"
#include "resources/resourcemanager.h"
#include "resources/theme.h"

#include <guichan/widgets/label.hpp>

ProgressIndicator::ProgressIndicator()
{
    const std::string path = gui->getTheme()->resolvePath("progress-indicator.png");
    mImageSet = ResourceManager::getInstance()->getImageSet(path, 32, 32);

    Animation anim;
    for (size_t i = 0; i < mImageSet->size(); ++i)
        anim.addFrame(mImageSet->get(i), 100, 0, 0);

    mIndicator = std::make_unique<SimpleAnimation>(std::move(anim));

    setSize(32, 32);
}

ProgressIndicator::~ProgressIndicator() = default;

void ProgressIndicator::logic()
{
    mIndicator->update(Time::deltaTimeMs());
}

void ProgressIndicator::draw(gcn::Graphics *graphics)
{
    // Draw the indicator centered on the widget
    const int x = (getWidth() - 32) / 2;
    const int y = (getHeight() - 32) / 2;
    mIndicator->draw(static_cast<Graphics*>(graphics), x, y);
}
