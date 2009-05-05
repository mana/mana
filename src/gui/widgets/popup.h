/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef POPUP_H
#define POPUP_H

#include "gui/widgets/container.h"

#include "configuration.h"
#include "guichanfwd.h"

class Skin;
class WindowContainer;

/**
 * A light version of the Window class. Particularly suited for popup type
 * functionality that doesn't need to be resized or moved around by the mouse
 * once created, but only needs to display some simple content, like a static
 * message.
 *
 * Popups, in general, shouldn't also need to update their content once
 * created, although this is not an explicit requirement to use the popup
 * class.
 *
 * \ingroup GUI
 */
class Popup : public Container
{
    public:
        /**
         * Constructor. Initializes the title to the given text and hooks
         * itself into the popup container.
         *
         * @param name    A human readable name for the popup. Only useful for
         *                debugging purposes.
         * @param skin    The location where the Popup's skin XML can be found.
         */
        Popup(const std::string &name = "",
              const std::string &skin = "graphics/gui/gui.xml");

        /**
         * Destructor. Deletes all the added widgets.
         */
        ~Popup();

        /**
         * Sets the window container to be used by new popups.
         */
        static void setWindowContainer(WindowContainer *windowContainer);

        /**
         * Changes the popup's skin to use the skin defined in the saved
         * configuration file.
         */
        void loadPopupConfiguration();

        /**
         * Currently only saves the skin used by the popup so that when the
         * client is reloaded, it can use the saved skin.
         */
        void savePopupConfiguration();

        /**
         * Draws the popup.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Sets the size of this popup.
         */
        void setContentSize(int width, int height);

        /**
         * Sets the location relative to the given widget.
         */
        void setLocationRelativeTo(gcn::Widget *widget);

        /**
         * Sets the minimum width of the popup.
         */
        void setMinWidth(int width);

        int getMinWidth() const { return mMinWidth; }

        /**
         * Sets the minimum height of the popup.
         */
        void setMinHeight(int height);

        int getMinHeight() const { return mMinHeight; }

        /**
         * Sets the maximum width of the popup.
         */
        void setMaxWidth(int width);

        int getMaxWidth() const { return mMaxWidth; }

        /**
         * Sets the minimum height of the popup.
         */
        void setMaxHeight(int height);

        int getMaxHeight() const { return mMaxHeight; }

        /**
         * Gets the padding of the popup. The padding is the distance between
         * the popup border and the content.
         *
         * @return The padding of the popup.
         * @see setPadding
         */
        int getPadding() const { return mPadding; }

        void setPadding(int padding) { mPadding = padding; }

        /**
         * Sets the name of the popup. This is only useful for debug purposes.
         */
        void setPopupName(const std::string &name)
        { mPopupName = name; }

        const std::string &getPopupName() const
        { return mPopupName; }

        /**
         * Schedule this popup for deletion. It will be deleted at the start
         * of the next logic update.
         */
        void scheduleDelete();

        // Inherited from BasicContainer

        virtual gcn::Rectangle getChildrenArea();

    private:
        std::string mPopupName;       /**< Name of the popup */
        std::string mDefaultSkinPath; /**< Default skin path for this popup */
        int mMinWidth;                /**< Minimum popup width */
        int mMinHeight;               /**< Minimum popup height */
        int mMaxWidth;                /**< Maximum popup width */
        int mMaxHeight;               /**< Maximum popup height */
        int mPadding;                 /**< Holds the padding of the popup. */

        Skin *mSkin;                  /**< Skin in use by this popup */
};

#endif
