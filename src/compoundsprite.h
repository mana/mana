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

#ifndef COMPOUNDSPRITE_H
#define COMPOUNDSPRITE_H

#include "sprite.h"

#include <vector>

class Image;

class CompoundSprite : public Sprite
{
public:
    CompoundSprite();

    ~CompoundSprite();

    virtual bool reset();

    virtual bool play(std::string action);

    virtual bool update(int time);

    virtual bool draw(Graphics *graphics, int posX, int posY) const;

    /**
     * Gets the width in pixels of the first sprite in the list.
     */
    virtual int getWidth() const
    { return mWidth; }

    /**
     * Gets the height in pixels of the first sprite in the list.
     */
    virtual int getHeight() const
    { return mHeight; }

    int getOffsetX() const
    { return mOffsetX; }

    int getOffsetY() const
    { return mOffsetY; }

    virtual const Image *getImage() const;

    virtual bool setDirection(SpriteDirection direction);

    int getNumberOfLayers() const;

    virtual bool drawnWhenBehind() const;

    int getDuration() const;

    size_t size() const
    { return mSprites.size(); }

    /**
     * Add a sprite on the highest layer id available.
     * @deprecated Should not be used we can't be sure of the layer id used.
     * @param sprite The sprite filename to load and add.
     * @param variant Offset in the same image. This is permitting to reuse
     * the same image for multiple sprites.
     */
    void addSprite(const std::string &spriteFile, int variant = 0);

    /**
     * Add an image (non-animated) sprite, with no directions handling,
     * and no alternate.
     */
    void addImageSprite(Image *image);

    /**
     * Add a sprite on the given layer id.
     * @param layer The layer id.
     * @param sprite The sprite filename to load and add.
     * @param variant Offset in the same image. This is permitting to reuse
     * the same image for multiple sprites.
     */
    void setSprite(int layer, const std::string &spriteFile, int variant = 0);

    /** Set the alternative sprite used with the sprite directions given. */
    void setAlternateSprite(int layer,
                            const std::string& spriteFile,
                            int variant = 0,
                            const std::list<SpriteDirection> &directions =
                                std::list<SpriteDirection>());

    Sprite *getSprite(int layer) const
    { return mSprites.at(layer); }

    /**
     * Removes a sprite, but also its potential alternate if present.
     */
    void removeSprite(int layer);

    /**
     * Removes an alternate display sprite.
     */
    void removeAlternateSprite(int layer,
                               const std::list<SpriteDirection> &directions);

    // Deals with deleting the Sprites* pointers
    void clear();

    void doRedraw()
    { mNeedsRedraw = true; }

    /** Removes every alternate sprite reference. Used when updating them. */
    void resetAlternateSprites();

private:

    typedef std::map<int, Sprite*>::iterator SpriteIterator;
    typedef std::map<int, Sprite*>::const_iterator SpriteConstIterator;

    /**
     * Add a sprite on the highest layer id available.
     * @deprecated Should not be used we can't be sure of the layer id used.
     * @param sprite The sprite object to add.
     */
    void addSprite(Sprite* sprite);

    /**
     * Add a sprite on the given layer id.
     * @param layer The layer id.
     * @param sprite The sprite object to add.
     */
    void setSprite(int layer, Sprite* sprite);

    /** Register the sprite for display according to the directions given. */
    void registerSprite(int layer, Sprite *sprite, bool alternate,
                        SpriteDirection direction = DIRECTION_DEFAULT);

    /** Unregister the sprite to not display it anymore. */
    void unregisterSprite(int layer, Sprite *sprite, bool alternate);

    void redraw() const;

    mutable Image *mImage;
    mutable Image *mAlphaImage;

    mutable int mWidth, mHeight;
    mutable int mOffsetX, mOffsetY;

    mutable bool mNeedsRedraw;

    /** The sprites direction */
    SpriteDirection mDirection;

    // The sprite ordered list to compound the multi-layer sprite.
    std::map<int, Sprite*> mSprites;
    // Alternate sprites used to display nicely with certain equipments.
    std::map<int, Sprite*> mAlternateSprites[DIRECTION_INVALID];
    /**
     * A list keeping the actual sprite used to be display depending on the
     * direction of the being.
     * The map will be organized this way:
     * @param int The compound sprite layer.
     * @param std::map<int, Sprite*> the sprite pointer used for the direction
     * (int) given.
     * Note that the sprite pointer is either one of mSprites
     * or mAlternateSprites depending on the direction and equipment used.
     * So the Sprite* pointer life cycle management is done with those.
     * Note also that removal of the main sprite implies the removal
     * of its alternate.
     */
     // <layer, <direction, Sprite*>>
    std::map<int, std::map<SpriteDirection, Sprite*> > mDisplayedSprites;
};

#endif // COMPOUNDSPRITE_H
