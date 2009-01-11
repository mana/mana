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
 */

#include "smileycontainer.h"

#include <guichan/mouseinput.hpp>
#include <guichan/selectionlistener.hpp>

#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"
#include "../smileyshortcut.h"

#include "../resources/image.h"
#include "../resources/iteminfo.h"
#include "../resources/resourcemanager.h"

#include "../utils/gettext.h"
#include "../utils/tostring.h"

const int SmileyContainer::gridWidth = 34;  // emote icon width + 4
const int SmileyContainer::gridHeight = 36; // emote icon height + 4

static const int NO_EMOTE = -1;

SmileyContainer::SmileyContainer():
    mSelectedEmoteIndex(NO_EMOTE)
{
    ResourceManager *resman = ResourceManager::getInstance();

    mSmileyImg = resman->getImageSet("graphics/sprites/emotions.png", 30, 32);
    if (!mSmileyImg) logger->error(_("Unable to load emotions"));

    mSelImg = resman->getImage("graphics/gui/selection.png");
    if (!mSelImg) logger->error(_("Unable to load selection.png"));

    mSelImg->setAlpha(config.getValue("guialpha", 0.8));

    mMaxSmiley = mSmileyImg->size(); 

    addMouseListener(this);
    addWidgetListener(this);
}

SmileyContainer::~SmileyContainer()
{
    if (mSmileyImg)
    {
       mSmileyImg->decRef();
       mSmileyImg=NULL;
    }
    if (!mSelImg)
    {
       mSelImg->decRef();
       mSelImg=NULL;
    }
}

void SmileyContainer::draw(gcn::Graphics *graphics)
{
    int columns = getWidth() / gridWidth;

    // Have at least 1 column
    if (columns < 1)
    {
        columns = 1;
    }

    for (int i = 0; i < mMaxSmiley ; i++)
    {
        int itemX = ((i) % columns) * gridWidth;
        int itemY = ((i) / columns) * gridHeight;
           

        // Draw item icon
        static_cast<Graphics*>(graphics)->drawImage(
                    mSmileyImg->get(i), itemX, itemY);

        // Draw selection image below selected item
        if (mSelectedEmoteIndex == i)
        {
            static_cast<Graphics*>(graphics)->drawImage(
                    mSelImg, itemX, itemY);
        }
    }
}

void SmileyContainer::widgetResized(const gcn::Event &event)
{
    recalculateHeight();
}

void SmileyContainer::recalculateHeight()
{
    int cols = getWidth() / gridWidth;

    if (cols < 1)
        cols = 1;

    const int rows = (mMaxSmiley / cols) + (mMaxSmiley % cols > 0 ? 1 : 0);
    const int height = rows * gridHeight + 8;
    if (height != getHeight())
        setHeight(height);
}

int SmileyContainer::getSelectedSmiley() 
{
    if (mSelectedEmoteIndex == NO_EMOTE)
        return 0;

    return 1+mSelectedEmoteIndex;
}

void SmileyContainer::selectNone()
{
    setSelectedEmoteIndex(NO_EMOTE);
}

void SmileyContainer::setSelectedEmoteIndex(int index)
{
    if (index < 0 || index >= mMaxSmiley )
        mSelectedEmoteIndex = NO_EMOTE;
    else
        mSelectedEmoteIndex = index;
}

void SmileyContainer::distributeValueChangedEvent()
{
    gcn::SelectionEvent event(this);
    std::list<gcn::SelectionListener*>::iterator i_end = mListeners.end();
    std::list<gcn::SelectionListener*>::iterator i;

    for (i = mListeners.begin(); i != i_end; ++i)
    {
        (*i)->valueChanged(event);
    }
}

void SmileyContainer::mousePressed(gcn::MouseEvent &event)
{
    int button = event.getButton();
    if (button == gcn::MouseEvent::LEFT || button == gcn::MouseEvent::RIGHT)
    {
        int columns = getWidth() / gridWidth;
        int mx = event.getX();
        int my = event.getY();
        int index = mx / gridWidth + ((my / gridHeight) * columns);
        if (index <mMaxSmiley)
        {
           setSelectedEmoteIndex(index);
           smileyShortcut->setSmileySelected(index+1);
        }
    }
}
