/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *  Copyright (C) 2009  Aethyra Development Team
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

#include "guichanfwd.h"

#include "gui/widgets/container.h"
#include "resources/theme.h"

#include <guichan/mouselistener.hpp>

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
class Popup : public Container, public gcn::MouseListener
{
    public:
        /**
         * Constructor. Initializes the title to the given text and hooks
         * itself into the popup container.
         *
         * @param name    A human readable name for the popup. Only useful for
         *                debugging purposes.
         * @param skinType The skin type used when drawing the popup.
         */
        explicit Popup(const std::string &name = std::string(),
                       SkinType skinType = SkinType::Popup);

        /**
         * Destructor. Deletes all the added widgets.
         */
        ~Popup() override;

        /**
         * Sets the window container to be used by new popups.
         */
        static void setWindowContainer(WindowContainer *windowContainer);

        /**
         * Draws the popup.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Sets the size of this popup.
         */
        void setContentSize(int width, int height);

        /**
         * Sets the location relative to the given widget.
         */
        void setLocationRelativeTo(gcn::Widget *widget);

        void mouseMoved(gcn::MouseEvent &event) override;

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

        gcn::Rectangle getChildrenArea() override;

        /**
         * Sets the location to display the popup. Tries to horizontally center
         * the popup and provide a vertical buffer between the given point and
         * the popup. Prevents the popup from extending off-screen, if
         * possible.
         */
        void position(int x, int y);

    private:
        std::string mPopupName;     /**< Name of the popup */
        int mMinWidth = 100;        /**< Minimum popup width */
        int mMinHeight = 40;        /**< Minimum popup height */
        int mMaxWidth;              /**< Maximum popup width */
        int mMaxHeight;             /**< Maximum popup height */
        int mPadding;               /**< Holds the padding of the popup. */

        SkinType mSkinType;         /**< The skin type used when drawing the popup widget. */
};
