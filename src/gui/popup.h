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

#include <guichan/widgetlistener.hpp>

#include <guichan/widgets/container.hpp>

#include "../graphics.h"
#include "../guichanfwd.h"

class ConfigListener;
class Skin;
class SkinLoader;
class Window;
class WindowContainer;

/**
 * A rather reduced down version of the Window class that is particularly suited
 * for 
 *
 * \ingroup GUI
 */
class Popup : public gcn::Container, gcn::WidgetListener
{
    public:
        friend class PopupConfigListener;

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
        void setMinWidth(unsigned int width);

        /**
         * Sets the minimum height of the popup.
         */
        void setMinHeight(unsigned int height);

        /**
         * Sets the maximum width of the popup.
         */
        void setMaxWidth(unsigned int width);

        /**
         * Sets the minimum height of the popup.
         */
        void setMaxHeight(unsigned int height);

        /**
         * Gets the minimum width of the popup.
         */
        int getMinWidth() { return mMinWidth; }

        /**
         * Gets the minimum height of the popup.
         */
        int getMinHeight() { return mMinHeight; }

        /**
         * Gets the maximum width of the popup.
         */
        int getMaxWidth() { return mMaxWidth; }

        /**
         * Gets the minimum height of the popup.
         */
        int getMaxHeight() { return mMaxHeight; }

        /**
         * Gets the padding of the popup. The padding is the distance between
         * the popup border and the content.
         *
         * @return The padding of the popup.
         * @see setPadding
         */
        unsigned int getPadding() const { return mPadding; }

        /**
         * Sets the padding of the popup. The padding is the distance between the
         * popup border and the content.
         *
         * @param padding The padding of the popup.
         * @see getPadding
         */
        void setPadding(unsigned int padding) { mPadding = padding; }

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
        void setGuiAlpha();

        Window *mParent;           /**< The parent Window (if there is one) */
        std::string mPopupName;    /**< Name of the Popup */
        static bool mAlphaChanged; /**< Whether the alpha percent was changed */
        int mMinWidth;             /**< Minimum Popup width */
        int mMinHeight;            /**< Minimum Popup height */
        int mMaxWidth;             /**< Maximum Popup width */
        int mMaxHeight;            /**< Maximum Popup height */
        unsigned int mPadding;     /**< Holds the padding of the window. */ 

        /**
         * The config listener that listens to changes relevant to all Popups.
         */
        static ConfigListener *popupConfigListener;

        static int instances;      /**< Number of Popup instances */

        Skin* mSkin;               /**< Skin in use by this Popup */
};

#endif
