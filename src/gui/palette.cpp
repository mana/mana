/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
 *  Copyright (C) 2009  The Mana World Development Team
 *
 *  This file is part of Aethyra.
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

#include <math.h>

#include "palette.h"
#include "gui.h"

#include "../configuration.h"
#include "../game.h"

#include "../utils/gettext.h"
#include "../utils/stringutils.h"

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

std::string Palette::getConfigName(const std::string& typeName)
{
    std::string res = "Color" + typeName;

    int pos = 5;
    for (unsigned int i = 0; i < typeName.length(); i++)
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

const int Palette::GRADIENT_DELAY = 40;

Palette::Palette() : 
    mRainbowTime(tick_time),
    mColVector(ColVector(TYPE_COUNT)),
    mGradVector()
{
    std::string indent = "  ";
    addColor(TEXT, 0x000000, STATIC, _("Text"));
    addColor(SHADOW, 0x000000, STATIC, indent + _("Text Shadow"));
    addColor(OUTLINE, 0x000000, STATIC, indent + _("Text Outline"));
    addColor(PROGRESS_BAR, 0xffffff, STATIC, indent + _("Progress Bar Labels"));

    addColor(BACKGROUND, 0xffffff, STATIC, _("Background"));

    addColor(HIGHLIGHT, 0xebc873, STATIC, _("Highlight"), 'H');
    addColor(TAB_HIGHLIGHT, 0xff0000, PULSE, indent + _("Tab Highlight"));
    addColor(SHOP_WARNING, 0x910000, STATIC, indent +
            _("Item too expensive"));

    addColor(CHAT, 0x000000, STATIC, _("Chat"), 'C');
    addColor(GM, 0xff0000, STATIC, indent + _("GM"), 'G');
    addColor(PLAYER, 0x1fa052, STATIC, indent + _("Player"), 'Y');
    addColor(WHISPER, 0x0000ff, STATIC, indent + _("Whisper"), 'W');
    addColor(IS, 0xa08527, STATIC, indent + _("Is"), 'I');
    addColor(PARTY, 0xff00d8, STATIC, indent + _("Party"), 'P');
    addColor(SERVER, 0x8415e2, STATIC, indent + _("Server"), 'S');
    addColor(LOGGER, 0x919191, STATIC, indent + _("Logger"), 'L');
    addColor(HYPERLINK, 0xe50d0d, STATIC, indent + _("Hyperlink"), '<');

    addColor(BEING, 0xffffff, STATIC, _("Being"));
    addColor(PC, 0xffffff, STATIC, indent + _("Other Player's Names"));
    addColor(SELF, 0xff8040, STATIC, indent + _("Own Name"));
    addColor(GM_NAME, 0x00ff00, STATIC, indent + _("GM Names"));
    addColor(NPC, 0xc8c8ff, STATIC, indent + _("NPCs"));
    addColor(MONSTER, 0xff4040, STATIC, indent + _("Monsters"));

    addColor(UNKNOWN_ITEM, 0x000000, STATIC, _("Unknown Item Type"));
    addColor(GENERIC, 0x21a5b1, STATIC, indent + _("Generic"));
    addColor(HEAD, 0x527fa4, STATIC, indent + _("Hat"));
    addColor(USABLE, 0x268d24, STATIC, indent + _("Usable"));
    addColor(TORSO, 0xd12aa4, STATIC, indent + _("Shirt"));
    addColor(ONEHAND, 0xf42a2a, STATIC, indent + _("1 Handed Weapons"));
    addColor(LEGS, 0x699900, STATIC, indent + _("Pants"));
    addColor(FEET, 0xaa1d48, STATIC, indent + _("Shoes"));
    addColor(TWOHAND, 0xf46d0e, STATIC, indent + _("2 Handed Weapons"));
    addColor(SHIELD, 0x9c2424, STATIC, indent + _("Shield"));
    addColor(RING, 0x0000ff, STATIC, indent + _("Ring"));
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
        if (col->grad == STATIC || col->grad == PULSE)
        {
            config.setValue(*configName, toString(col->getRGB()));
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
        for (unsigned int i = 0; i < mGradVector.size(); i++)
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
         i != iEnd;
         ++i)
    {
        if (i->grad != i->committedGrad)
        {
            setGradient(i->type, i->committedGrad);
        }
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
                       Palette::GradientType grad,
                       const std::string &text, char c)
{
    const std::string *configName = &ColorTypeNames[type];
    gcn::Color trueCol = (int)config.getValue(*configName, rgb);
    grad = (GradientType)config.getValue(*configName + "Gradient", grad);
    mColVector[type].set(type, trueCol, grad, text, c);
    if (grad != STATIC)
    {
        mGradVector.push_back(&mColVector[type]);
    }
}

void Palette::advanceGradient ()
{
    if (get_elapsed_time(mRainbowTime) > 5)
    {
        int pos, colIndex, colVal;
        // For slower systems, advance can be greater than one (advance > 1
        // skips advance-1 steps). Should make gradient look the same
        // independent of the framerate.
        int advance = get_elapsed_time(mRainbowTime) / 5;
        double startColVal, destColVal;

        for (unsigned int i = 0; i < mGradVector.size(); i++)
        {
            mGradVector[i]->gradientIndex =
                    (mGradVector[i]->gradientIndex + advance) %
                    (GRADIENT_DELAY * ((mGradVector[i]->grad == SPECTRUM) ?
                    (mGradVector[i]->grad == PULSE) ? 255 : 6 :
                     RAINBOW_COLOR_COUNT));

            pos = mGradVector[i]->gradientIndex % GRADIENT_DELAY;
            colIndex = mGradVector[i]->gradientIndex / GRADIENT_DELAY;

            if (mGradVector[i]->grad == PULSE)
            {
                colVal = (int) (255.0 * (sin(M_PI * 
                         (mGradVector[i]->gradientIndex) / 255) + 1) / 2);

                const gcn::Color* col = &mGradVector[i]->testColor;

                mGradVector[i]->color.r = ((colVal * col->r) / 255) % (col->r + 1);
                mGradVector[i]->color.g = ((colVal * col->g) / 255) % (col->g + 1);
                mGradVector[i]->color.b = ((colVal * col->b) / 255) % (col->b + 1);
            }
            if (mGradVector[i]->grad == SPECTRUM)
            {
                if (colIndex % 2)
                { // falling curve
                    colVal = (int)(255.0 * (cos(M_PI * pos / GRADIENT_DELAY) +
                             1) / 2);
                }
                else
                { // ascending curve
                    colVal = (int)(255.0 * (cos(M_PI * (GRADIENT_DELAY-pos) /
                             GRADIENT_DELAY) + 1) / 2);
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
                        &RAINBOW_COLORS[(colIndex + 1) % RAINBOW_COLOR_COUNT];

                startColVal = (cos(M_PI * pos / GRADIENT_DELAY) + 1) / 2;
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

