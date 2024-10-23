/*
 *  Gui Skinning
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2009  Aethyra Development Team
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "resources/theme.h"

#include "configuration.h"
#include "log.h"
#include "textrenderer.h"

#include "resources/dye.h"
#include "resources/image.h"
#include "resources/imageset.h"
#include "resources/resourcemanager.h"

#include "utils/dtor.h"
#include "utils/filesystem.h"
#include "utils/stringutils.h"
#include "utils/xml.h"

#include <guichan/font.hpp>

#include <algorithm>
#include <optional>

/**
 * Initializes the directory in which the client looks for GUI themes, which at
 * the same time functions as a fallback directory when looking up files
 * relevant for the GUI theme.
 */
static std::string defaultThemePath;

static void initDefaultThemePath()
{
    defaultThemePath = branding.getStringValue("guiThemePath");

    if (defaultThemePath.empty() || !FS::isDirectory(defaultThemePath))
        defaultThemePath = "graphics/gui/";
}

static std::optional<std::string> findThemePath(const std::string &theme)
{
    if (theme.empty())
        return {};

    std::string themePath = defaultThemePath;
    themePath += theme;

    if (FS::isDirectory(themePath))
        return themePath;

    return {};
}


Skin::Skin(ImageRect skin, Image *close, Image *stickyUp, Image *stickyDown):
    mBorder(skin),
    mCloseImage(close),
    mStickyImageUp(stickyUp),
    mStickyImageDown(stickyDown)
{}

Skin::~Skin()
{
    // Clean up static resources
    for (auto img : mBorder.grid)
        delete img;
}

void Skin::updateAlpha(float alpha)
{
    mBorder.setAlpha(alpha);

    mCloseImage->setAlpha(alpha);
    mStickyImageUp->setAlpha(alpha);
    mStickyImageDown->setAlpha(alpha);
}

int Skin::getMinWidth() const
{
    return mBorder.grid[ImageRect::UPPER_LEFT]->getWidth() +
           mBorder.grid[ImageRect::UPPER_RIGHT]->getWidth();
}

int Skin::getMinHeight() const
{
    return mBorder.grid[ImageRect::UPPER_LEFT]->getHeight() +
           mBorder.grid[ImageRect::LOWER_LEFT]->getHeight();
}


enum {
    BUTTON_MODE_STANDARD,       // 0
    BUTTON_MODE_HIGHLIGHTED,    // 1
    BUTTON_MODE_PRESSED,        // 2
    BUTTON_MODE_DISABLED,       // 3
    BUTTON_MODE_COUNT           // 4 - Must be last.
};

enum {
    TAB_STANDARD,    // 0
    TAB_HIGHLIGHTED, // 1
    TAB_SELECTED,    // 2
    TAB_UNUSED,      // 3
    TAB_COUNT        // 4 - Must be last.
};

Theme::Theme(const std::string &path)
    : Palette(THEME_COLORS_END)
    , mThemePath(path)
    , mProgressColors(THEME_PROG_END)
{
    listen(Event::ConfigChannel);
    loadColors();

    mColors[HIGHLIGHT].ch = 'H';
    mColors[CHAT].ch = 'C';
    mColors[GM].ch = 'G';
    mColors[PLAYER].ch = 'Y';
    mColors[WHISPER].ch = 'W';
    mColors[IS].ch = 'I';
    mColors[PARTY].ch = 'P';
    mColors[GUILD].ch = 'U';
    mColors[SERVER].ch = 'S';
    mColors[LOGGER].ch = 'L';
    mColors[HYPERLINK].ch = '<';

    // Button skin
    struct ButtonData
    {
        char const *file;
        int gridX;
        int gridY;
    };

    constexpr ButtonData data[BUTTON_MODE_COUNT] = {
        { "button.png", 0, 0 },
        { "buttonhi.png", 9, 4 },
        { "buttonpress.png", 16, 19 },
        { "button_disabled.png", 25, 23 }
    };

    mButton = new ImageRect[BUTTON_MODE_COUNT];

    for (int mode = 0; mode < BUTTON_MODE_COUNT; ++mode)
    {
        auto modeImage = getImage(data[mode].file);
        int a = 0;
        for (int y = 0; y < 3; y++)
        {
            for (int x = 0; x < 3; x++)
            {
                mButton[mode].grid[a] = modeImage->getSubImage(
                    data[x].gridX, data[y].gridY,
                    data[x + 1].gridX - data[x].gridX + 1,
                    data[y + 1].gridY - data[y].gridY + 1);
                a++;
            }
        }
    }

    // Tab skin
    struct TabData
    {
        char const *file;
        int gridX[4];
        int gridY[4];
    };

    constexpr TabData tabData[TAB_COUNT] = {
        { "tab.png", {0, 9, 16, 25}, {0, 13, 19, 20} },
        { "tab_hilight.png", {0, 9, 16, 25}, {0, 13, 19, 20} },
        { "tabselected.png", {0, 9, 16, 25}, {0, 4, 12, 20} },
        { "tab.png", {0, 9, 16, 25}, {0, 13, 19, 20} }
    };

    for (int mode = 0; mode < TAB_COUNT; mode++)
    {
        auto tabImage = getImage(tabData[mode].file);
        int a = 0;
        for (int y = 0; y < 3; y++)
        {
            for (int x = 0; x < 3; x++)
            {
                mTabImg[mode].grid[a] = tabImage->getSubImage(
                    tabData[mode].gridX[x], tabData[mode].gridY[y],
                    tabData[mode].gridX[x + 1] - tabData[mode].gridX[x] + 1,
                    tabData[mode].gridY[y + 1] - tabData[mode].gridY[y] + 1);
                a++;
            }
        }
        mTabImg[mode].setAlpha(mAlpha);
    }

    // TextField images
    auto deepBox = getImage("deepbox.png");
    constexpr int gridx[4] = {0, 3, 28, 31};
    constexpr int gridy[4] = {0, 3, 28, 31};
    int a = 0;

    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 3; x++)
        {
            mDeepBoxImageRect.grid[a] = deepBox->getSubImage(gridx[x],
                                                             gridy[y],
                                                             gridx[x + 1] - gridx[x] + 1,
                                                             gridy[y + 1] - gridy[y] + 1);
            a++;
        }
    }

    // CheckBox images
    auto checkBox = getImage("checkbox.png");
    mCheckBoxNormal.reset(checkBox->getSubImage(0, 0, 9, 10));
    mCheckBoxChecked.reset(checkBox->getSubImage(9, 0, 9, 10));
    mCheckBoxDisabled.reset(checkBox->getSubImage(18, 0, 9, 10));
    mCheckBoxDisabledChecked.reset(checkBox->getSubImage(27, 0, 9, 10));
    mCheckBoxNormalHi.reset(checkBox->getSubImage(36, 0, 9, 10));
    mCheckBoxCheckedHi.reset(checkBox->getSubImage(45, 0, 9, 10));

    // RadioButton images
    mRadioNormal = getImage("radioout.png");
    mRadioChecked = getImage("radioin.png");
    mRadioDisabled = getImage("radioout.png");
    mRadioDisabledChecked = getImage("radioin.png");
    mRadioNormalHi = getImage("radioout_highlight.png");
    mRadioCheckedHi = getImage("radioin_highlight.png");

    // Slider images
    int x, y, w, h, o1, o2;
    auto slider = getImage("slider.png");
    auto sliderHi = getImage("slider_hilight.png");

    x = 0; y = 0;
    w = 15; h = 6;
    o1 = 4; o2 = 11;
    hStart.reset(slider->getSubImage(x, y, o1 - x, h));
    hMid.reset(slider->getSubImage(o1, y, o2 - o1, h));
    hEnd.reset(slider->getSubImage(o2, y, w - o2 + x, h));
    hStartHi.reset(sliderHi->getSubImage(x, y, o1 - x, h));
    hMidHi.reset(sliderHi->getSubImage(o1, y, o2 - o1, h));
    hEndHi.reset(sliderHi->getSubImage(o2, y, w - o2 + x, h));

    x = 6; y = 8;
    w = 9; h = 10;
    hGrip.reset(slider->getSubImage(x, y, w, h));
    hGripHi.reset(sliderHi->getSubImage(x, y, w, h));

    x = 0; y = 6;
    w = 6; h = 21;
    o1 = 10; o2 = 18;
    vStart.reset(slider->getSubImage(x, y, w, o1 - y));
    vMid.reset(slider->getSubImage(x, o1, w, o2 - o1));
    vEnd.reset(slider->getSubImage(x, o2, w, h - o2 + y));
    vStartHi.reset(sliderHi->getSubImage(x, y, w, o1 - y));
    vMidHi.reset(sliderHi->getSubImage(x, o1, w, o2 - o1));
    vEndHi.reset(sliderHi->getSubImage(x, o2, w, h - o2 + y));

    x = 6; y = 8;
    w = 9; h = 10;
    vGrip.reset(slider->getSubImage(x, y, w, h));
    vGripHi.reset(sliderHi->getSubImage(x, y, w, h));

    // ProgressBar and ScrollArea images
    auto vscroll = getImage("vscroll_grey.png");
    auto vscrollHi = getImage("vscroll_highlight.png");

    constexpr int vsgridx[4] = {0, 4, 7, 11};
    constexpr int vsgridy[4] = {0, 4, 15, 19};
    a = 0;

    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 3; x++)
        {
            mScrollBarMarker.grid[a] = vscroll->getSubImage(
                vsgridx[x], vsgridy[y],
                vsgridx[x + 1] - vsgridx[x],
                vsgridy[y + 1] - vsgridy[y]);
            mScrollBarMarkerHi.grid[a] = vscrollHi->getSubImage(
                vsgridx[x], vsgridy[y],
                vsgridx[x + 1] - vsgridx[x],
                vsgridy[y + 1] - vsgridy[y]);
            a++;
        }
    }

    mScrollBarMarker.setAlpha(config.guiAlpha);
    mScrollBarMarkerHi.setAlpha(config.guiAlpha);

    // DropDown and ScrollArea buttons
    mArrowButtons[ARROW_UP][0] = getImage("vscroll_up_default.png");
    mArrowButtons[ARROW_DOWN][0] = getImage("vscroll_down_default.png");
    mArrowButtons[ARROW_LEFT][0] = getImage("hscroll_left_default.png");
    mArrowButtons[ARROW_RIGHT][0] = getImage("hscroll_right_default.png");
    mArrowButtons[ARROW_UP][1] = getImage("vscroll_up_pressed.png");
    mArrowButtons[ARROW_DOWN][1] = getImage("vscroll_down_pressed.png");
    mArrowButtons[ARROW_LEFT][1] = getImage("hscroll_left_pressed.png");
    mArrowButtons[ARROW_RIGHT][1] = getImage("hscroll_right_pressed.png");

    mResizeGripImage = getImage("resize.png");
}

Theme::~Theme()
{
    for (int mode = 0; mode < BUTTON_MODE_COUNT; ++mode)
    {
        std::for_each(mButton[mode].grid, mButton[mode].grid + 9,
                      dtor<Image*>());
    }
    delete[] mButton;

    for (auto &imgRect : mTabImg)
        std::for_each(imgRect.grid, imgRect.grid + 9, dtor<Image*>());

    std::for_each(mDeepBoxImageRect.grid, mDeepBoxImageRect.grid + 9, dtor<Image*>());
    std::for_each(mScrollBarMarker.grid, mScrollBarMarker.grid + 9, dtor<Image*>());
    std::for_each(mScrollBarMarkerHi.grid, mScrollBarMarkerHi.grid + 9, dtor<Image*>());
}

const gcn::Color &Theme::getThemeColor(int type, int alpha)
{
    return gui->getTheme()->getColor(type, alpha);
}

const gcn::Color &Theme::getThemeColor(char c, bool &valid)
{
    return gui->getTheme()->getColor(c, valid);
}

gcn::Color Theme::getProgressColor(int type, float progress)
{
    const auto &dye = gui->getTheme()->mProgressColors[type];

    int color[3] = {0, 0, 0};
    dye->getColor(progress, color);

    return gcn::Color(color[0], color[1], color[2]);
}

Skin *Theme::load(const std::string &filename)
{
    // Check if this skin was already loaded
    auto skinIterator = mSkins.find(filename);
    if (skinIterator != mSkins.end())
    {
        auto &skin = skinIterator->second;
        skin->instances++;
        return skin.get();
    }

    auto skin = readSkin(filename);
    if (!skin)
    {
        logger->error(strprintf("Error: Loading default skin '%s' failed. "
                                "Make sure the skin file is valid.",
                                mThemePath.c_str()));
    }

    // Add the skin to the loaded skins
    return (mSkins[filename] = std::move(skin)).get();
}

void Theme::drawButton(Graphics *graphics, const WidgetState &state) const
{
    int mode;

    if (!state.enabled)
        mode = BUTTON_MODE_DISABLED;
    else if (state.selected)
        mode = BUTTON_MODE_PRESSED;
    else if (state.hovered || state.focused)
        mode = BUTTON_MODE_HIGHLIGHTED;
    else
        mode = BUTTON_MODE_STANDARD;

    graphics->drawImageRect(0, 0, state.width, state.height, mButton[mode]);
}

void Theme::drawTextFieldFrame(Graphics *graphics, const WidgetState &state) const
{
    graphics->drawImageRect(0, 0, state.width, state.height, mDeepBoxImageRect);
}

void Theme::drawTab(Graphics *graphics, const WidgetState &state) const
{
    int mode = TAB_STANDARD;

    if (state.selected)
        mode = TAB_SELECTED;
    else if (state.hovered)
        mode = TAB_HIGHLIGHTED;

    graphics->drawImageRect(0, 0, state.width, state.height, mTabImg[mode]);
}

void Theme::drawCheckBox(gcn::Graphics *graphics, const WidgetState &state) const
{
    Image *box;

    if (state.enabled)
    {
        if (state.selected)
        {
            if (state.hovered)
                box = mCheckBoxCheckedHi.get();
            else
                box = mCheckBoxChecked.get();
        }
        else
        {
            if (state.hovered)
                box = mCheckBoxNormalHi.get();
            else
                box = mCheckBoxNormal.get();
        }
    }
    else
    {
        if (state.selected)
            box = mCheckBoxDisabledChecked.get();
        else
            box = mCheckBoxDisabled.get();
    }

    static_cast<Graphics*>(graphics)->drawImage(box, 2, 2);
}

void Theme::drawRadioButton(gcn::Graphics *graphics, const WidgetState &state) const
{
    Image *box = nullptr;

    if (state.enabled)
    {
        if (state.selected)
        {
            if (state.hovered)
                box = mRadioCheckedHi;
            else
                box = mRadioChecked;
        }
        else
        {
            if (state.hovered)
                box = mRadioNormalHi;
            else
                box = mRadioNormal;
        }
    }
    else
    {
        if (state.selected)
            box = mRadioDisabledChecked;
        else
            box = mRadioDisabled;
    }

    static_cast<Graphics*>(graphics)->drawImage(box, 2, 2);
}

void Theme::drawSlider(Graphics *graphics, const WidgetState &state, int markerPosition) const
{
    auto start = state.hovered ? hStartHi.get() : hStart.get();
    auto mid = state.hovered ? hMidHi.get() : hMid.get();
    auto end = state.hovered ? hEndHi.get() : hEnd.get();
    auto grip = state.hovered ? hGripHi.get() : hGrip.get();

    int w = state.width;
    int h = state.height;
    int x = 0;
    int y = (h - start->getHeight()) / 2;

    graphics->drawImage(start, x, y);

    w -= start->getWidth() + end->getWidth();
    x += start->getWidth();

    graphics->drawImagePattern(mid, x, y, w, mid->getHeight());

    x += w;
    graphics->drawImage(end, x, y);

    graphics->drawImage(grip, markerPosition, (state.height - grip->getHeight()) / 2);
}

void Theme::drawDropDownFrame(Graphics *graphics, const WidgetState &state) const
{
    graphics->drawImageRect(0, 0, state.width, state.height, mDeepBoxImageRect);
}

void Theme::drawDropDownButton(Graphics *graphics, const WidgetState &state) const
{
    const auto buttonDir = state.selected ? ARROW_UP : ARROW_DOWN;
    const Image *img = mArrowButtons[buttonDir][state.hovered];
    graphics->drawImage(img, state.width - img->getHeight() - 2, 2);
}

void Theme::drawProgressBar(Graphics *graphics, const gcn::Rectangle &area,
                            const gcn::Color &color, float progress,
                            const std::string &text) const
{
    gcn::Font *oldFont = graphics->getFont();
    gcn::Color oldColor = graphics->getColor();

    graphics->drawImageRect(area, mScrollBarMarker);

    // The bar
    if (progress > 0)
    {
        graphics->setColor(color);
        graphics->fillRectangle(gcn::Rectangle(area.x + 4,
                                               area.y + 4,
                                               (int) (progress * (area.width - 8)),
                                               area.height - 8));
    }

    // The label
    if (!text.empty())
    {
        const int textX = area.x + area.width / 2;
        const int textY = area.y + (area.height - boldFont->getHeight()) / 2;

        TextRenderer::renderText(graphics,
                                 text,
                                 textX,
                                 textY,
                                 gcn::Graphics::CENTER,
                                 Theme::getThemeColor(Theme::PROGRESS_BAR),
                                 gui->getFont(),
                                 true,
                                 false);
    }

    graphics->setFont(oldFont);
    graphics->setColor(oldColor);
}

void Theme::drawScrollAreaFrame(Graphics *graphics, const WidgetState &state) const
{
    graphics->drawImageRect(0, 0, state.width, state.height, mDeepBoxImageRect);
}

void Theme::drawScrollAreaButton(Graphics *graphics,
                                 ArrowButtonDirection dir,
                                 bool pressed,
                                 const gcn::Rectangle &dim) const
{
    const int state = pressed ? 1 : 0;
    graphics->drawImage(mArrowButtons[dir][state], dim.x, dim.y);
}

void Theme::drawScrollAreaMarker(Graphics *graphics, bool hovered, const gcn::Rectangle &dim) const
{
    const auto &imageRect = hovered ? mScrollBarMarkerHi : mScrollBarMarker;
    graphics->drawImageRect(dim.x, dim.y, dim.width, dim.height, imageRect);
}

int Theme::getSliderMarkerLength() const
{
    return hGrip->getWidth();
}

void Theme::setMinimumOpacity(float minimumOpacity)
{
    if (minimumOpacity > 1.0f)
        return;

    mMinimumOpacity = minimumOpacity;
    updateAlpha();
}

void Theme::updateAlpha()
{
    const float alpha = std::max(config.guiAlpha, mMinimumOpacity);
    if (mAlpha == alpha)
        return;

    mAlpha = alpha;

    for (auto &skin : mSkins)
        skin.second->updateAlpha(mAlpha);

    for (int mode = 0; mode < BUTTON_MODE_COUNT; ++mode)
        mButton[mode].setAlpha(mAlpha);

    for (auto &t : mTabImg)
        t.setAlpha(mAlpha);

    mDeepBoxImageRect.setAlpha(mAlpha);

    mCheckBoxNormal->setAlpha(mAlpha);
    mCheckBoxChecked->setAlpha(mAlpha);
    mCheckBoxDisabled->setAlpha(mAlpha);
    mCheckBoxDisabledChecked->setAlpha(mAlpha);
    mCheckBoxNormalHi->setAlpha(mAlpha);
    mCheckBoxCheckedHi->setAlpha(mAlpha);

    mRadioNormal->setAlpha(mAlpha);
    mRadioChecked->setAlpha(mAlpha);
    mRadioDisabled->setAlpha(mAlpha);
    mRadioDisabledChecked->setAlpha(mAlpha);
    mRadioNormalHi->setAlpha(mAlpha);
    mRadioCheckedHi->setAlpha(mAlpha);

    hStart->setAlpha(mAlpha);
    hMid->setAlpha(mAlpha);
    hEnd->setAlpha(mAlpha);
    hGrip->setAlpha(mAlpha);
    hStartHi->setAlpha(mAlpha);
    hMidHi->setAlpha(mAlpha);
    hEndHi->setAlpha(mAlpha);
    hGripHi->setAlpha(mAlpha);

    vStart->setAlpha(mAlpha);
    vMid->setAlpha(mAlpha);
    vEnd->setAlpha(mAlpha);
    vGrip->setAlpha(mAlpha);
    vStartHi->setAlpha(mAlpha);
    vMidHi->setAlpha(mAlpha);
    vEndHi->setAlpha(mAlpha);
    vGripHi->setAlpha(mAlpha);

    mScrollBarMarker.setAlpha(mAlpha);
    mScrollBarMarkerHi.setAlpha(mAlpha);

    mArrowButtons[ARROW_UP][0]->setAlpha(mAlpha);
    mArrowButtons[ARROW_DOWN][0]->setAlpha(mAlpha);
    mArrowButtons[ARROW_LEFT][0]->setAlpha(mAlpha);
    mArrowButtons[ARROW_RIGHT][0]->setAlpha(mAlpha);
    mArrowButtons[ARROW_UP][1]->setAlpha(mAlpha);
    mArrowButtons[ARROW_DOWN][1]->setAlpha(mAlpha);
    mArrowButtons[ARROW_LEFT][1]->setAlpha(mAlpha);
    mArrowButtons[ARROW_RIGHT][1]->setAlpha(mAlpha);

    mResizeGripImage->setAlpha(mAlpha);
}

void Theme::event(Event::Channel channel, const Event &event)
{
    if (channel == Event::ConfigChannel &&
        event.getType() == Event::ConfigOptionChanged &&
        event.hasValue(&Config::guiAlpha))
    {
        updateAlpha();
    }
}

std::unique_ptr<Skin> Theme::readSkin(const std::string &filename) const
{
    if (filename.empty())
        return nullptr;

    logger->log("Loading skin '%s'.", filename.c_str());

    XML::Document doc(resolvePath(filename));
    XML::Node rootNode = doc.rootNode();

    if (!rootNode || rootNode.name() != "skinset")
        return nullptr;

    const std::string skinSetImage = rootNode.getProperty("image", "");

    if (skinSetImage.empty())
    {
        logger->log("Theme::readSkin(): Skinset does not define an image!");
        return nullptr;
    }

    logger->log("Theme::load(): <skinset> defines '%s' as a skin image.",
                skinSetImage.c_str());

    auto dBorders = getImage(skinSetImage);
    ImageRect border;
    memset(&border, 0, sizeof(ImageRect));

    // iterate <widget>'s
    for (auto widgetNode : rootNode.children())
    {
        if (widgetNode.name() != "widget")
            continue;

        const std::string widgetType =
                widgetNode.getProperty("type", "unknown");
        if (widgetType == "Window")
        {
            // Iterate through <part>'s
            // LEEOR / TODO:
            // We need to make provisions to load in a CloseButton image. For
            // now it can just be hard-coded.
            for (auto partNode : widgetNode.children())
            {
                if (partNode.name() != "part")
                    continue;

                const std::string partType =
                        partNode.getProperty("type", "unknown");
                // TOP ROW
                const int xPos = partNode.getProperty("xpos", 0);
                const int yPos = partNode.getProperty("ypos", 0);
                const int width = partNode.getProperty("width", 1);
                const int height = partNode.getProperty("height", 1);

                if (partType == "top-left-corner")
                    border.grid[0] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "top-edge")
                    border.grid[1] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "top-right-corner")
                    border.grid[2] = dBorders->getSubImage(xPos, yPos, width, height);

                // MIDDLE ROW
                else if (partType == "left-edge")
                    border.grid[3] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "bg-quad")
                    border.grid[4] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "right-edge")
                    border.grid[5] = dBorders->getSubImage(xPos, yPos, width, height);

                // BOTTOM ROW
                else if (partType == "bottom-left-corner")
                    border.grid[6] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "bottom-edge")
                    border.grid[7] = dBorders->getSubImage(xPos, yPos, width, height);
                else if (partType == "bottom-right-corner")
                    border.grid[8] = dBorders->getSubImage(xPos, yPos, width, height);

                else
                    logger->log("Theme::readSkin(): Unknown part type '%s'",
                                partType.c_str());
            }
        }
        else
        {
            logger->log("Theme::readSkin(): Unknown widget type '%s'",
                        widgetType.c_str());
        }
    }

    logger->log("Finished loading skin.");

    // Hard-coded for now until we update the above code to look for window buttons
    auto closeImage = getImage("close_button.png");
    auto sticky = getImage("sticky_button.png");
    Image *stickyImageUp = sticky->getSubImage(0, 0, 15, 15);
    Image *stickyImageDown = sticky->getSubImage(15, 0, 15, 15);

    auto skin = std::make_unique<Skin>(border, closeImage, stickyImageUp, stickyImageDown);
    skin->updateAlpha(mAlpha);
    return skin;
}

std::string Theme::prepareThemePath()
{
    initDefaultThemePath();

    // Try theme from settings
    auto themePath = findThemePath(config.theme);

    // Try theme from branding
    if (!themePath)
        themePath = findThemePath(branding.getStringValue("theme"));

    return themePath.value_or(defaultThemePath);
}

std::string Theme::resolvePath(const std::string &path) const
{
    // Need to strip off any dye info for the existence tests
    int pos = path.find('|');
    std::string file;
    if (pos > 0)
        file = path.substr(0, pos);
    else
        file = path;

    // Try the theme
    file = mThemePath + "/" + file;
    if (FS::exists(file))
        return mThemePath + "/" + path;

    // Backup
    return defaultThemePath + "/" + path;
}

ResourceRef<Image> Theme::getImage(const std::string &path) const
{
    return ResourceManager::getInstance()->getImage(resolvePath(path));
}

ResourceRef<Image> Theme::getImageFromTheme(const std::string &path)
{
    return gui->getTheme()->getImage(path);
}

static int readColorType(const std::string &type)
{
    static const char *colors[] = {
        "TEXT",
        "SHADOW",
        "OUTLINE",
        "PROGRESS_BAR",
        "BUTTON",
        "BUTTON_DISABLED",
        "TAB",
        "PARTY_CHAT_TAB",
        "PARTY_SOCIAL_TAB",
        "BACKGROUND",
        "HIGHLIGHT",
        "TAB_FLASH",
        "SHOP_WARNING",
        "ITEM_EQUIPPED",
        "CHAT",
        "GM",
        "PLAYER",
        "WHISPER",
        "IS",
        "PARTY",
        "GUILD",
        "SERVER",
        "LOGGER",
        "HYPERLINK",
        "UNKNOWN_ITEM",
        "GENERIC",
        "HEAD",
        "USABLE",
        "TORSO",
        "ONEHAND",
        "LEGS",
        "FEET",
        "TWOHAND",
        "SHIELD",
        "RING",
        "NECKLACE",
        "ARMS",
        "AMMO",
        "SERVER_VERSION_NOT_SUPPORTED"
    };

    if (type.empty())
        return -1;

    for (int i = 0; i < Theme::THEME_COLORS_END; i++)
        if (type == colors[i])
            return i;

    return -1;
}

static gcn::Color readColor(const std::string &description)
{
    int size = description.length();
    if (size < 7 || description[0] != '#')
    {
        error:
        logger->log("Error, invalid theme color palette: %s",
                    description.c_str());
        return Palette::BLACK;
    }

    int v = 0;
    for (int i = 1; i < 7; ++i)
    {
        char c = description[i];
        int n;

        if ('0' <= c && c <= '9')
            n = c - '0';
        else if ('A' <= c && c <= 'F')
            n = c - 'A' + 10;
        else if ('a' <= c && c <= 'f')
            n = c - 'a' + 10;
        else
            goto error;

        v = (v << 4) | n;
    }

    return gcn::Color(v);
}

static Palette::GradientType readColorGradient(const std::string &grad)
{
    static const char *grads[] = {
        "STATIC",
        "PULSE",
        "SPECTRUM",
        "RAINBOW"
    };

    if (grad.empty())
        return Palette::STATIC;

    for (int i = 0; i < 4; i++)
        if (grad == grads[i])
            return static_cast<Palette::GradientType>(i);

    return Palette::STATIC;
}

static int readProgressType(const std::string &type)
{
    static const char *colors[] = {
        "DEFAULT",
        "HP",
        "MP",
        "NO_MP",
        "EXP",
        "INVY_SLOTS",
        "WEIGHT",
        "JOB"
    };

    if (type.empty())
        return -1;

    for (int i = 0; i < Theme::THEME_PROG_END; i++)
        if (type == colors[i])
            return i;

    return -1;
}

void Theme::loadColors()
{
    std::string file = resolvePath("colors.xml");

    XML::Document doc(file);
    XML::Node root = doc.rootNode();

    if (!root || root.name() != "colors")
    {
        logger->log("Error loading colors file: %s", file.c_str());
        return;
    }

    for (auto node : root.children())
    {
        if (node.name() == "color")
        {
            const int type = readColorType(node.getProperty("id", std::string()));
            if (type < 0) // invalid or no type given
                continue;

            const std::string temp = node.getProperty("color", std::string());
            if (temp.empty()) // no color set, so move on
                continue;

            const gcn::Color color = readColor(temp);
            const GradientType grad = readColorGradient(node.getProperty("effect", std::string()));

            mColors[type].set(type, color, grad, 10);
        }
        else if (node.name() == "progressbar")
        {
            const int type = readProgressType(node.getProperty("id", std::string()));
            if (type < 0) // invalid or no type given
                continue;

            mProgressColors[type] = std::make_unique<DyePalette>(node.getProperty("color", std::string()));
        }
    }
}
