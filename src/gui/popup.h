/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#include <guichan/widgets/container.hpp>

#include "../graphics.h"
#include "../guichanfwd.h"

class Skin;
class SkinLoader;
class Window;
class WindowContainer;

/**
 * A rather reduced down version of the Window class that is particularly suited
 * for popup type functionality that doesn't need to be resized or moved around
 * by the mouse once created, but only needs to display some simple content,
 * like a static message. Popups, in general, shouldn't also need to update
 * their content once created, although this is not an explicit requirement to
 * use the popup class.
 *
 * \ingroup GUI
 */
class Popup : public gcn::Container
{
    public:
        /**
         * Constructor. Initializes the title to the given text and hooks
         * itself into the popup container.
         *
         * @param name    A human readable name for the popup. Only useful for
         *                debugging purposes.
         * @param parent  The parent Window. This is the Window standing above
         *                this one in the Window hiearchy. When reordering,
         *                a Popup will never go below its parent Window.
         * @param skin    The location where the Popup's skin XML can be found.
         */
        Popup(const std::string& name = "", Window *parent = NULL,
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

        /**
         * Sets the minimum height of the popup.
         */
        void setMinHeight(int height);

        /**
         * Sets the maximum width of the popup.
         */
        void setMaxWidth(int width);

        /**
         * Sets the minimum height of the popup.
         */
        void setMaxHeight(int height);

        /**
         * Gets the minimum width of the popup.
         */
        int getMinWidth() const { return mMinWidth; }

        /**
         * Gets the minimum height of the popup.
         */
        int getMinHeight() const { return mMinHeight; }

        /**
         * Gets the maximum width of the popup.
         */
        int getMaxWidth() const { return mMaxWidth; }

        /**
         * Gets the minimum height of the popup.
         */
        int getMaxHeight() const { return mMaxHeight; }

        /**
         * Gets the padding of the popup. The padding is the distance between
         * the popup border and the content.
         *
         * @return The padding of the popup.
         * @see setPadding
         */
        int getPadding() const { return mPadding; }

        /**
         * Sets the padding of the popup. The padding is the distance between the
         * popup border and the content.
         *
         * @param padding The padding of the popup.
         * @see getPadding
         */
        void setPadding(int padding) { mPadding = padding; }

        /**
         * Returns the parent Window.
         *
         * @return The parent Window or <code>NULL</code> if there is none.
         */
        Window* getParentWindow() { return mParent; }

        /**
         * Sets the name of the popup. This is only useful for debug purposes.
         */
        void setPopupName(const std::string &name) { mPopupName = name; }

        /**
         * Returns the name of the popup. This is only useful for debug purposes.
         */
        const std::string& getPopupName() { return mPopupName; }

        /**
         * Schedule this popup for deletion. It will be deleted at the start
         * of the next logic update.
         */
        void scheduleDelete();

        // Inherited from BasicContainer

        virtual gcn::Rectangle getChildrenArea();

    private:
        Window *mParent;              /**< The parent Window (if there is one) */
        std::string mPopupName;       /**< Name of the popup */
        std::string mDefaultSkinPath; /**< Default skin path for this popup */
        int mMinWidth;                /**< Minimum popup width */
        int mMinHeight;               /**< Minimum popup height */
        int mMaxWidth;                /**< Maximum popup width */
        int mMaxHeight;               /**< Maximum popup height */
        int mPadding;                 /**< Holds the padding of the popup. */

        static int instances;         /**< Number of popup instances */

        Skin* mSkin;                  /**< Skin in use by this popup */
};

#endif
