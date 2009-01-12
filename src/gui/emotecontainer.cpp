/*
 *  Aethyra
 *  Copyright 2009 Aethyra Development Team
 *
 *  This file is part of Aethyra.
 *
 *  Aethyra is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  Aethyra is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Aethyra; if not, write to the Free Software Foundation, 
 *  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <guichan/mouseinput.hpp>
#include <guichan/selectionlistener.hpp>

#include "emotecontainer.h"
#include "emoteshortcut.h"

#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"

#include "../resources/image.h"
#include "../resources/iteminfo.h"
#include "../resources/resourcemanager.h"

#include "../utils/gettext.h"
#include "../utils/tostring.h"

const int EmoteContainer::gridWidth = 34;  // emote icon width + 4
const int EmoteContainer::gridHeight = 36; // emote icon height + 4

static const int NO_EMOTE = -1;

EmoteContainer::EmoteContainer():
    mSelectedEmoteIndex(NO_EMOTE)
{
    ResourceManager *resman = ResourceManager::getInstance();

    mEmoteImg = resman->getImageSet("graphics/sprites/emotions.png", 30, 32);
    if (!mEmoteImg) logger->error(_("Unable to load emotions"));

    mSelImg = resman->getImage("graphics/gui/selection.png");
    if (!mSelImg) logger->error(_("Unable to load selection.png"));

    mSelImg->setAlpha(config.getValue("guialpha", 0.8));

    mMaxEmote = mEmoteImg->size(); 

    addMouseListener(this);
    addWidgetListener(this);
}

EmoteContainer::~EmoteContainer()
{
    if (mEmoteImg)
    {
       mEmoteImg->decRef();
       mEmoteImg = NULL;
    }
    if (!mSelImg)
    {
       mSelImg->decRef();
       mSelImg = NULL;
    }
}

void EmoteContainer::draw(gcn::Graphics *graphics)
{
    int columns = getWidth() / gridWidth;

    // Have at least 1 column
    if (columns < 1)
    {
        columns = 1;
    }

    for (int i = 0; i < mMaxEmote ; i++)
    {
        int emoteX = ((i) % columns) * gridWidth;
        int emoteY = ((i) / columns) * gridHeight;
           
        // Draw emote icon
        static_cast<Graphics*>(graphics)->drawImage(
                    mEmoteImg->get(i), emoteX, emoteY);

        // Draw selection image below selected item
        if (mSelectedEmoteIndex == i)
        {
            static_cast<Graphics*>(graphics)->drawImage(
                    mSelImg, emoteX, emoteY);
        }
    }
}

void EmoteContainer::widgetResized(const gcn::Event &event)
{
    recalculateHeight();
}

void EmoteContainer::recalculateHeight()
{
    int cols = getWidth() / gridWidth;

    if (cols < 1)
        cols = 1;

    const int rows = (mMaxEmote / cols) + (mMaxEmote % cols > 0 ? 1 : 0);
    const int height = rows * gridHeight + 8;
    if (height != getHeight())
        setHeight(height);
}

int EmoteContainer::getSelectedEmote() 
{
    if (mSelectedEmoteIndex == NO_EMOTE)
        return 0;

    return 1 + mSelectedEmoteIndex;
}

void EmoteContainer::selectNone()
{
    setSelectedEmoteIndex(NO_EMOTE);
}

void EmoteContainer::setSelectedEmoteIndex(int index)
{
    if (index < 0 || index >= mMaxEmote )
        mSelectedEmoteIndex = NO_EMOTE;
    else
        mSelectedEmoteIndex = index;
}

void EmoteContainer::distributeValueChangedEvent()
{
    gcn::SelectionEvent event(this);
    std::list<gcn::SelectionListener*>::iterator i_end = mListeners.end();
    std::list<gcn::SelectionListener*>::iterator i;

    for (i = mListeners.begin(); i != i_end; ++i)
    {
        (*i)->valueChanged(event);
    }
}

void EmoteContainer::mousePressed(gcn::MouseEvent &event)
{
    int button = event.getButton();
    if (button == gcn::MouseEvent::LEFT || button == gcn::MouseEvent::RIGHT)
    {
        int columns = getWidth() / gridWidth;
        int mx = event.getX();
        int my = event.getY();
        int index = mx / gridWidth + ((my / gridHeight) * columns);
        if (index < mMaxEmote)
        {
           setSelectedEmoteIndex(index);
           emoteShortcut->setEmoteSelected(index + 1);
        }
    }
}
