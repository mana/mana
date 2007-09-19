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
 *
 *  $Id$
 */

#ifndef _TMW_WINDOW_H__
#define _TMW_WINDOW_H__

#include <guichan/widgets/window.hpp>

#include "../guichanfwd.h"

#include "windowlistener.h"

class ConfigListener;
class GCContainer;
class ImageRect;
class ResizeGrip;
class WindowContainer;
class Image;

/**
 * A window. This window can be dragged around and has a title bar. Windows are
 * invisible by default.
 *
 * \ingroup GUI
 */
class Window : public gcn::Window
{
    public:
        friend class WindowConfigListener;

        /**
         * Constructor. Initializes the title to the given text and hooks
         * itself into the window container.
         *
         * @param caption The initial window title, "Window" by default.
         * @param modal   Block input to other windows.
         * @param parent  The parent window. This is the window standing above
         *                this one in the window hiearchy. When reordering,
         *                a window will never go below its parent window.
         */
        Window(const std::string &caption = "Window", bool modal = false,
                Window *parent = NULL);

        /**
         * Destructor.
         */
        ~Window();

        /**
         * Sets the window container to be used by new windows.
         */
        static void setWindowContainer(WindowContainer *windowContainer);

        /**
         * Draws the window.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Adds a widget to the window.
         */
        void add(gcn::Widget *wi, bool delChild = true);

        /**
         * Adds a widget to the window and also specifices its position.
         */
        void add(gcn::Widget *w, int x, int y, bool delChild = true);

        /**
         * Sets the width of the window contents.
         */
        void setContentWidth(int width);

        /**
         * Sets the height of the window contents.
         */
        void setContentHeight(int height);

        /**
         * Sets the size of this window.
         */
        void setContentSize(int width, int height);

        /**
         * Called when either the user or resetToDefaultSize resizes the
         * windows, so that the windows can manage its widgets.
         */
        virtual void updateContentSize() {}

        /**
         * Sets the width of this window.
         */
        void setWidth(int width);

        /**
         * Sets the height of this window.
         */
        void setHeight(int height);

        /**
         * Sets the position and size of this window.
         */
        void setDimension(const gcn::Rectangle &dimension);

        /**
         * Sets the position of this window.
         */
        void setPosition(int x, int y);

        /**
         * Sets the window x coordinate.
         */
        void setX(int x);

        /**
         * Sets the window y coordinate.
         */
        void setY(int y);

        /**
         * Sets the location relative to the given widget.
         */
        void setLocationRelativeTo(gcn::Widget *widget);

        /**
         * Sets whether or not the window can be resized.
         */
        void setResizable(bool resize);

        /**
         * Sets whether or not the window has a close button.
         */
        void setCloseButton(bool flag);

        /**
         * Returns whether the window can be resized.
         */
        bool isResizable();

        /**
         * Sets the minimum width of the window.
         */
        void setMinWidth(unsigned int width);

        /**
         * Sets the minimum height of the window.
         */
        void setMinHeight(unsigned int height);

        /**
         * Sets the maximum width of the window.
         */
        void setMaxWidth(unsigned int width);

        /**
         * Sets the minimum height of the window.
         */
        void setMaxHeight(unsigned int height);

        /**
         * Sets flag to show a title or not.
         */
        void setShowTitle(bool flag)
        { mShowTitle = flag; }

        /**
          * Sets whether the window is sticky. A sticky window will not have
          * its visibility set to false on a general setVisible(false) call.
          */
        void setSticky(bool sticky);

        /**
         * Returns whether the window is sticky.
         */
        bool isSticky();

        /**
         * Overloads window setVisible by Guichan to allow sticky window
         * handling.
         */
        void setVisible(bool visible);

        /**
         * Returns the parent window.
         *
         * @return The parent window or <code>NULL</code> if there is none.
         */
        Window*
        getParentWindow() { return mParent; }

        /**
         * Schedule this window for deletion. It will be deleted at the start
         * of the next logic update.
         */
        void scheduleDelete();

        /**
         * Starts window resizing when appropriate.
         */
        void mousePressed(gcn::MouseEvent &event);

        /**
         * Implements window resizing and makes sure the window is not
         * dragged/resized outside of the screen.
         */
        void mouseDragged(gcn::MouseEvent &event);

        /**
         * Implements custom cursor image changing context, based on mouse
         * relative position.
         */
        void mouseMoved(gcn::MouseEvent &event);

        /**
         * When the mouse button has been let go, this ensures that the mouse
         * custom cursor is restored back to it's standard image.
         */
        void mouseReleased(gcn::MouseEvent &event);

        /**
         * When the mouse leaves the window this ensures that the custom cursor
         * is restored back to it's standard image.
         */
        void mouseExited(gcn::MouseEvent &event);

        /**
         * Read the x, y, and width and height for resizables in the config
         * based on the given string.
         * That function let the values set with set{X, Y, Height, width}()
         * if no config value is found.
         * Don't forget to set these default values and resizable before
         * calling this function.
         */
        void loadWindowState(std::string const &);

        /**
         * Set the default win pos and size.
         * (which can be different of the actual ones.)
         */
        void setDefaultSize(int defaultX, int defaultY,
                            int defaultWidth, int defaultHeight);

        /**
         * Reset the win pos and size to default. Don't forget to set defaults
         * first.
         */
        void resetToDefaultSize();

        /**
         * Adds a listener to the list that's notified when the window is
         * moved or resized.
         */
        void addWindowListener(WindowListener *listener)
        { mListeners.push_back(listener); }

        /**
         * Removes a listener from the list that's notified when the window is
         * moved or resized.
         */
        void removeWindowListener(WindowListener *listener)
        { mListeners.remove(listener); }

        enum ResizeHandles
        {
            TOP    = 0x01,
            RIGHT  = 0x02,
            BOTTOM = 0x04,
            LEFT   = 0x08
        };

        /** The window container windows add themselves to. */
        static WindowContainer *windowContainer;

    private:
        /**
         * Determines if the mouse is in a resize area and returns appropriate
         * resize handles. Also initializes drag offset in case the resize
         * grip is used.
         *
         * @see ResizeHandles
         */
        int getResizeHandles(gcn::MouseEvent &event);

        GCContainer *mChrome;      /**< Contained container */
        ResizeGrip *mGrip;         /**< Resize grip */
        Window *mParent;           /**< The parent window */
        std::string mConfigName;   /**< Name used for saving window-related data */
        bool mShowTitle;           /**< Window has a title bar */
        bool mModal;               /**< Window is modal */
        bool mResizable;           /**< Window can be resized */
        bool mCloseButton;         /**< Window has a close button */
        bool mSticky;              /**< Window resists minimization */
        int mMinWinWidth;          /**< Minimum window width */
        int mMinWinHeight;         /**< Minimum window height */
        int mMaxWinWidth;          /**< Maximum window width */
        int mMaxWinHeight;         /**< Maximum window height */
        int mDefaultX;             /**< Default window X position */
        int mDefaultY;             /**< Default window Y position */
        int mDefaultWidth;         /**< Default window width */
        int mDefaultHeight;        /**< Default window height */

        /**
         * The config listener that listens to changes relevant to all windows.
         */
        static ConfigListener *windowConfigListener;

        static int mouseResize;    /**< Active resize handles */
        static int instances;      /**< Number of Window instances */
        static ImageRect border;   /**< The window border and background */
        static Image *closeImage;  /**< Close Button Image */

        /**
         * The width of the resize border. Is independent of the actual window
         * border width, and determines mostly the size of the corner area
         * where two borders are moved at the same time.
         */
        static const int resizeBorderWidth = 10;

    private:
        /**
         * Sends out a window event to the list of selection listeners.
         */
        void fireWindowEvent(const WindowEvent &event);

        WindowListeners mListeners;
};

#endif
