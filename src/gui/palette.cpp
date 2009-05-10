/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "palette.h"

#include "configuration.h"
#include "game.h"

#include "gui/gui.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <math.h>

const gcn::Color Palette::BLACK = gcn::Color(0, 0, 0);

const gcn::Color Palette::RAINBOW_COLORS[7] = {
    gcn::Color(255, 0, 0),
    gcn::Color(255, 153, 0),
    gcn::Color(255, 255, 0),
    gcn::Color(0, 153, 0),
    gcn::Color(0, 204, 204),
    gcn::Color(51, 0, 153),
    gcn::Color(153, 0, 153)
};
/** Number of Elemets of RAINBOW_COLORS */
const int Palette::RAINBOW_COLOR_COUNT = 7;

std::string Palette::getConfigName(const std::string &typeName)
{
    std::string res = "Color" + typeName;

    int pos = 5;
    for (size_t i = 0; i < typeName.length(); i++)
    {
        if (i == 0 || typeName[i] == '_')
        {
            if (i > 0)
                i++;

            res[pos] = typeName[i];
        }
        else
        {
           res[pos] = tolower(typeName[i]);
        }
        pos++;
    }
    res.erase(pos, res.length() - pos);

    return res;
}

DEFENUMNAMES(ColorType, COLOR_TYPE);

Palette::Palette() :
    mRainbowTime(tick_time),
    mColVector(ColVector(TYPE_COUNT))
{
    std::string indent = "  ";
    addColor(TEXT, 0x000000, STATIC, _("Text"));
    addColor(SHADOW, 0x000000, STATIC, indent + _("Text Shadow"));
    addColor(OUTLINE, 0x000000, STATIC, indent + _("Text Outline"));
    addColor(PROGRESS_BAR, 0xffffff, STATIC, indent + _("Progress Bar Labels"));

    addColor(BACKGROUND, 0xffffff, STATIC, _("Background"));

    addColor(HIGHLIGHT, 0xebc873, STATIC, _("Highlight"), 'H');
    addColor(TAB_HIGHLIGHT, 0xff0000, PULSE, indent + _("Tab Highlight"));
    addColor(SHOP_WARNING, 0x910000, STATIC, indent + _("Item too expensive"));
    addColor(ITEM_EQUIPPED, 0x000091, STATIC, indent + _("Item is equipped"));

    addColor(CHAT, 0x000000, STATIC, _("Chat"), 'C');
    addColor(GM, 0xff0000, STATIC, indent + _("GM"), 'G');
    addColor(PLAYER, 0x1fa052, STATIC, indent + _("Player"), 'Y');
    addColor(WHISPER, 0x0000ff, STATIC, indent + _("Whisper"), 'W');
    addColor(IS, 0xa08527, STATIC, indent + _("Is"), 'I');
    addColor(PARTY, 0xf48055, STATIC, indent + _("Party"), 'P');
    addColor(SERVER, 0x8415e2, STATIC, indent + _("Server"), 'S');
    addColor(LOGGER, 0x919191, STATIC, indent + _("Logger"), 'L');
    addColor(HYPERLINK, 0xe50d0d, STATIC, indent + _("Hyperlink"), '<');

    addColor(BEING, 0xffffff, STATIC, _("Being"));
    addColor(PC, 0xffffff, STATIC, indent + _("Other Players' Names"));
    addColor(SELF, 0xff8040, STATIC, indent + _("Own Name"));
    addColor(GM_NAME, 0x00ff00, STATIC, indent + _("GM Names"));
    addColor(NPC, 0xc8c8ff, STATIC, indent + _("NPCs"));
    addColor(MONSTER, 0xff4040, STATIC, indent + _("Monsters"));

    addColor(UNKNOWN_ITEM, 0x000000, STATIC, _("Unknown Item Type"));
    addColor(GENERIC, 0x21a5b1, STATIC, indent + _("Generics"));
    addColor(HEAD, 0x527fa4, STATIC, indent + _("Hats"));
    addColor(USABLE, 0x268d24, STATIC, indent + _("Usables"));
    addColor(TORSO, 0xd12aa4, STATIC, indent + _("Shirts"));
    addColor(ONEHAND, 0xf42a2a, STATIC, indent + _("1 Handed Weapons"));
    addColor(LEGS, 0x699900, STATIC, indent + _("Pants"));
    addColor(FEET, 0xaa1d48, STATIC, indent + _("Shoes"));
    addColor(TWOHAND, 0xf46d0e, STATIC, indent + _("2 Handed Weapons"));
    addColor(SHIELD, 0x9c2424, STATIC, indent + _("Shields"));
    addColor(RING, 0x0000ff, STATIC, indent + _("Rings"));
    addColor(NECKLACE, 0xff00ff, STATIC, indent + _("Necklaces"));
    addColor(ARMS, 0x9c24e8, STATIC, indent + _("Arms"));
    addColor(AMMO, 0x8b6311, STATIC, indent + _("Ammo"));

    addColor(PARTICLE, 0xffffff, STATIC, _("Particle Effects"));
    addColor(PICKUP_INFO, 0x28dc28, STATIC, indent + _("Pickup Notification"));
    addColor(EXP_INFO, 0xffff00, STATIC, indent + _("Exp Notification"));
    addColor(HIT_PLAYER_MONSTER, 0x0064ff, STATIC,
            indent + _("Player hits Monster"));
    addColor(HIT_MONSTER_PLAYER, 0xff3232, STATIC,
            indent + _("Monster hits Player"));
    addColor(HIT_CRITICAL, 0xff0000, RAINBOW, indent + _("Critical Hit"));
    addColor(MISS, 0xffff00, STATIC, indent + _("Misses"));

    addColor(HPBAR_FULL, 0x0f6a20, STATIC, _("HP Bar"));
    addColor(HPBAR_THREE_QUARTERS, 0xc38948, STATIC, indent + _("3/4 HP Bar"));
    addColor(HPBAR_ONE_HALF, 0xe28000, STATIC, indent + _("1/2 HP Bar"));
    addColor(HPBAR_ONE_QUARTER, 0xff0000, PULSE, indent + _("1/4 HP Bar"));
    commit(true);
}

Palette::~Palette()
{
    const std::string *configName;
    for (ColVector::iterator col = mColVector.begin(),
         colEnd = mColVector.end(); col != colEnd; ++col)
    {
        configName = &ColorTypeNames[col->type];
        config.setValue(*configName + "Gradient", col->committedGrad);

        if (col->grad != STATIC)
            config.setValue(*configName + "Delay", col->delay);

        if (col->grad == STATIC || col->grad == PULSE)
        {
            char buffer[20];
            sprintf(buffer, "0x%06x", col->getRGB());
            config.setValue(*configName, std::string(buffer));
        }
    }
}

const gcn::Color& Palette::getColor(char c, bool &valid)
 {
    for (ColVector::const_iterator col = mColVector.begin(),
         colEnd = mColVector.end(); col != colEnd; ++col)
    {
        if (col->ch == c)
        {
            valid = true;
            return col->color;
        }
    }
    valid = false;
    return BLACK;
}

void Palette::setColor(ColorType type, int r, int g, int b)
{
    mColVector[type].color.r = r;
    mColVector[type].color.g = g;
    mColVector[type].color.b = b;
}

void Palette::setGradient(ColorType type, GradientType grad)
{
    ColorElem *elem = &mColVector[type];
    if (elem->grad != STATIC && grad == STATIC)
    {
        for (size_t i = 0; i < mGradVector.size(); i++)
        {
            if (mGradVector[i] == elem)
            {
                mGradVector.erase(mGradVector.begin() + i);
                break;
            }
        }
    }
    else if (elem->grad == STATIC && grad != STATIC)
    {
        mGradVector.push_back(elem);
    }

    if (elem->grad != grad)
    {
        elem->grad = grad;
    }
}

std::string Palette::getElementAt(int i)
{
    if (i < 0 || i >= getNumberOfElements())
    {
        return "";
    }
    return mColVector[i].text;
}

Palette::ColorType Palette::getColorTypeAt(int i)
{
    if (i < 0 || i >= getNumberOfElements())
    {
        return CHAT;
    }
    return mColVector[i].type;
}

void Palette::commit(bool commitNonStatic)
{
    for (ColVector::iterator i = mColVector.begin(), iEnd = mColVector.end();
         i != iEnd; ++i)
    {
        i->committedGrad = i->grad;
        i->committedDelay = i->delay;
        if (commitNonStatic || i->grad == STATIC)
        {
            i->committedColor = i->color;
        }
        else if (i->grad == PULSE)
        {
            i->committedColor = i->testColor;
        }
    }
}

void Palette::rollback()
{
    for (ColVector::iterator i = mColVector.begin(), iEnd = mColVector.end();
         i != iEnd; ++i)
    {
        if (i->grad != i->committedGrad)
        {
            setGradient(i->type, i->committedGrad);
        }
        setGradientDelay(i->type, i->committedDelay);
        setColor(i->type, i->committedColor.r, i->committedColor.g,
                 i->committedColor.b);
        if (i->grad == PULSE)
        {
            i->testColor.r = i->committedColor.r;
            i->testColor.g = i->committedColor.g;
            i->testColor.b = i->committedColor.b;
        }
    }
}

void Palette::addColor(Palette::ColorType type, int rgb,
                       Palette::GradientType grad, const std::string &text,
                       char c, int delay)
{
    const std::string *configName = &ColorTypeNames[type];
    char buffer[20];
    sprintf(buffer, "0x%06x", rgb);
    const std::string rgbString = config.getValue(*configName,
                                                  std::string(buffer));
    unsigned int rgbValue = 0;
    if (rgbString.length() == 8 && rgbString[0] == '0' && rgbString[1] == 'x')
        rgbValue = atox(rgbString);
    else
        rgbValue = atoi(rgbString.c_str());
    gcn::Color trueCol = rgbValue;
    grad = (GradientType) config.getValue(*configName + "Gradient", grad);
    delay = (int) config.getValue(*configName + "Delay", delay);
    mColVector[type].set(type, trueCol, grad, text, c, delay);

    if (grad != STATIC)
        mGradVector.push_back(&mColVector[type]);
}

void Palette::advanceGradient()
{
    if (get_elapsed_time(mRainbowTime) > 5)
    {
        int pos, colIndex, colVal, delay, numOfColors;
        // For slower systems, advance can be greater than one (advance > 1
        // skips advance-1 steps). Should make gradient look the same
        // independent of the framerate.
        int advance = get_elapsed_time(mRainbowTime) / 5;
        double startColVal, destColVal;

        for (size_t i = 0; i < mGradVector.size(); i++)
        {
            delay = mGradVector[i]->delay;

            if (mGradVector[i]->grad == PULSE)
                delay = delay / 20;

            numOfColors = (mGradVector[i]->grad == SPECTRUM ? 6 :
                           mGradVector[i]->grad == PULSE ? 127 :
                           RAINBOW_COLOR_COUNT);

            mGradVector[i]->gradientIndex =
                                    (mGradVector[i]->gradientIndex + advance) %
                                    (delay * numOfColors);

            pos = mGradVector[i]->gradientIndex % delay;
            colIndex = mGradVector[i]->gradientIndex / delay;

            if (mGradVector[i]->grad == PULSE)
            {
                colVal = (int) (255.0 * sin(M_PI * colIndex / numOfColors));

                const gcn::Color* col = &mGradVector[i]->testColor;

                mGradVector[i]->color.r = ((colVal * col->r) / 255) % (col->r + 1);
                mGradVector[i]->color.g = ((colVal * col->g) / 255) % (col->g + 1);
                mGradVector[i]->color.b = ((colVal * col->b) / 255) % (col->b + 1);
            }
            if (mGradVector[i]->grad == SPECTRUM)
            {
                if (colIndex % 2)
                { // falling curve
                    colVal = (int)(255.0 * (cos(M_PI * pos / delay) + 1) / 2);
                }
                else
                { // ascending curve
                    colVal = (int)(255.0 * (cos(M_PI * (delay - pos) / delay) +
                                   1) / 2);
                }

                mGradVector[i]->color.r =
                        (colIndex == 0 || colIndex == 5) ? 255 :
                        (colIndex == 1 || colIndex == 4) ? colVal : 0;
                mGradVector[i]->color.g =
                        (colIndex == 1 || colIndex == 2) ? 255 :
                        (colIndex == 0 || colIndex == 3) ? colVal : 0;
                mGradVector[i]->color.b =
                        (colIndex == 3 || colIndex == 4) ? 255 :
                        (colIndex == 2 || colIndex == 5) ? colVal : 0;
            }
            else if (mGradVector[i]->grad == RAINBOW)
            {
                const gcn::Color* startCol = &RAINBOW_COLORS[colIndex];
                const gcn::Color* destCol =
                        &RAINBOW_COLORS[(colIndex + 1) % numOfColors];

                startColVal = (cos(M_PI * pos / delay) + 1) / 2;
                destColVal = 1 - startColVal;

                mGradVector[i]->color.r =(int)(startColVal * startCol->r +
                                               destColVal * destCol->r);

                mGradVector[i]->color.g =(int)(startColVal * startCol->g +
                                               destColVal * destCol->g);

                mGradVector[i]->color.b =(int)(startColVal * startCol->b +
                                               destColVal * destCol->b);
            }
        }

        mRainbowTime = tick_time;
    }
}
