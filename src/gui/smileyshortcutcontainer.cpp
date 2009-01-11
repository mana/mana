/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
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

#include "smileyshortcutcontainer.h"

#include "../graphics.h"
#include "../inventory.h"
#include "../item.h"
#include "../itemshortcut.h"
#include "../smileyshortcut.h"
#include "../keyboardconfig.h"
#include "../localplayer.h"
#include "../log.h"


#include "../resources/image.h"
#include "../resources/resourcemanager.h"

#include "../utils/gettext.h"
#include "../utils/tostring.h"

SmileyShortcutContainer::SmileyShortcutContainer():
    mSmileyClicked(false),
    mSmileyMoved(0)
{
    mGridWidth = 1,
    mGridHeight = 1,
    addMouseListener(this);
    addWidgetListener(this);

    ResourceManager *resman = ResourceManager::getInstance();

    mBackgroundImg = resman->getImage("graphics/gui/item_shortcut_bgr.png");
    mSmileyImg = resman->getImageSet("graphics/sprites/emotions.png", 30, 32);
    if (!mSmileyImg) logger->error(_("Unable to load emotions"));

    mMaxItems = smileyShortcut->getSmileyCount(); 

    mBoxHeight = 42;
    mBoxWidth = 36;
}

SmileyShortcutContainer::~SmileyShortcutContainer()
{
    mBackgroundImg->decRef();
    if (mSmileyImg)
    {
       mSmileyImg->decRef();
       mSmileyImg=NULL;
    }

}

void
SmileyShortcutContainer::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    graphics->setFont(getFont());

    for (int i = 0; i < mMaxItems; i++)
    {
        const int itemX = (i % mGridWidth) * mBoxWidth;
        const int itemY = (i / mGridWidth) * mBoxHeight;

        g->drawImage(mBackgroundImg, itemX, itemY);

        // Draw item keyboard shortcut.
        const char *key = SDL_GetKeyName(
            (SDLKey) keyboard.getKeyValue(keyboard.KEY_SMILEY_1 + i));
        graphics->setColor(0x000000);
        g->drawText(key, itemX + 2, itemY + 2, gcn::Graphics::LEFT);
        if (smileyShortcut->getSmiley(i))
        {
        static_cast<Graphics*>(graphics)->drawImage(
                    mSmileyImg->get(smileyShortcut->getSmiley(i)-1), itemX+2, itemY+10);
        }

    }
    if (mSmileyMoved)
    {
        // Draw the item image being dragged by the cursor.
        Image* image = mSmileyImg->get(mSmileyMoved-1);
        if (image)
        {
            const int tPosX = mCursorPosX - (image->getWidth() / 2);
            const int tPosY = mCursorPosY - (image->getHeight() / 2);

            g->drawImage(image, tPosX, tPosY);
        }
    }
}

void
SmileyShortcutContainer::mouseDragged(gcn::MouseEvent &event)
{
   if (event.getButton() == gcn::MouseEvent::LEFT) {
      if (!mSmileyMoved && mSmileyClicked) {
         const int index = getIndexFromGrid(event.getX(), event.getY());
         if (index == -1) {
            return;
         }
         const int smileyId = smileyShortcut->getSmiley(index);
         if (smileyId)
         {
            mSmileyMoved = smileyId;
            smileyShortcut->removeSmiley(index);
         }
      }
      if (mSmileyMoved) {
         mCursorPosX = event.getX();
         mCursorPosY = event.getY();
      }
   }
}

void
SmileyShortcutContainer::mousePressed(gcn::MouseEvent &event)
{
   const int index = getIndexFromGrid(event.getX(), event.getY());
   if (index == -1) {
      return;
   }
   // Stores the selected item if theirs one.
   if (smileyShortcut->isSmileySelected()) {
      smileyShortcut->setSmiley(index);
      smileyShortcut->setSmileySelected(0);
   }
   else if (smileyShortcut->getSmiley(index)) {
      mSmileyClicked = true;
   }
}

void
SmileyShortcutContainer::mouseReleased(gcn::MouseEvent &event)
{
   if (event.getButton() == gcn::MouseEvent::LEFT)
   {
      if (smileyShortcut->isSmileySelected())
      {
         smileyShortcut->setSmileySelected(0);
      }
      const int index = getIndexFromGrid(event.getX(), event.getY());
      if (index == -1) {
         mSmileyMoved = 0;
         return;
      }
      if (mSmileyMoved) {
         smileyShortcut->setSmileys(index, mSmileyMoved);
         mSmileyMoved = 0;
      }
      else if (smileyShortcut->getSmiley(index) && mSmileyClicked)
      {
         smileyShortcut->useSmiley(index+1);
      }
      if (mSmileyClicked) {
         mSmileyClicked = false;
      }
   }
}

