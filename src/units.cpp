/*
 *  Support for custom units
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

#include "units.h"

#include "log.h"

#include "utils/stringutils.h"
#include "utils/xml.h"

#include <climits>
#include <vector>

struct UnitLevel {
    std::string symbol;
    int count;
    int round;
};

struct UnitDescription {
    std::vector<UnitLevel> levels;
    double conversion;
    bool mix;
};

enum UnitType {
    UNIT_WEIGHT = 0,
    UNIT_CURRENCY = 1,
    UNIT_END
};

UnitDescription units[UNIT_END];


void Units::init()
{
    { // Setup default weight
        UnitDescription ud;

        ud.conversion = 1.0;
        ud.mix = false;

        UnitLevel bu;
        bu.symbol = "g";
        bu.count = 1;
        bu.round = 0;

        ud.levels.push_back(bu);

        UnitLevel ul;
        ul.symbol = "kg";
        ul.count = 1000;
        ul.round = 2;

        ud.levels.push_back(ul);

        units[UNIT_WEIGHT] = ud;
    }

    { // Setup default currency
        UnitDescription ud;

        ud.conversion = 1.0;
        ud.mix = false;

        UnitLevel bu;
        bu.symbol = "¤";
        bu.count = 1;
        bu.round = 0;

        ud.levels.push_back(bu);

        units[UNIT_CURRENCY] = ud;
    }
}

void Units::readUnitNode(xmlNodePtr node, const std::string &filename)
{
    UnitDescription ud;
    int level = 1;
    const std::string type = XML::getProperty(node, "type", "");
    ud.conversion = XML::getProperty(node, "conversion", 1);
    ud.mix = XML::getProperty(node, "mix", "no") == "yes";

    UnitLevel bu;
    bu.symbol = XML::getProperty(node, "base", "¤");
    bu.count = 1;
    bu.round = XML::getProperty(node, "round", 2);

    ud.levels.push_back(bu);

    for (auto uLevel : XML::Children(node))
    {
        if (xmlStrEqual(uLevel->name, BAD_CAST "level"))
        {
            UnitLevel ul;
            ul.symbol = XML::getProperty(uLevel, "symbol",
                                            strprintf("¤%d",level));
            ul.count = XML::getProperty(uLevel, "count", -1);
            ul.round = XML::getProperty(uLevel, "round", bu.round);

            if (ul.count > 0)
            {
                ud.levels.push_back(ul);
                level++;
            }
            else
            {
                logger->log("Error bad unit count: %d for %s in %s",
                                ul.count, ul.symbol.c_str(), bu.symbol.c_str());
            }
        }
    }

    // Add one more level for saftey
    UnitLevel &ll = ud.levels.emplace_back();
    ll.count = INT_MAX;
    ll.round = 0;

    if (type == "weight")
        units[UNIT_WEIGHT] = ud;
    else if (type == "currency")
        units[UNIT_CURRENCY] = ud;
    else
        logger->log("Error unknown unit type: %s in %s", type.c_str(), filename.c_str());

}

void Units::checkStatus()
{
}

std::string formatUnit(int value, int type)
{
    UnitDescription ud = units[type];
    UnitLevel ul;

    // Shortcut for 0; do the same for values less than 0  (for now)
    if (value <= 0)
    {
        ul = ud.levels[0];
        return strprintf("0%s", ul.symbol.c_str());
    }

    double amount = ud.conversion * value;

    // If only the first level is needed, act like mix if false
    if (ud.mix && !ud.levels.empty() && ud.levels[1].count < amount)
    {
        std::string output;
        UnitLevel pl = ud.levels[0];
        ul = ud.levels[1];
        int levelAmount = (int) amount;
        int nextAmount;

        levelAmount /= ul.count;

        amount -= levelAmount * ul.count;

        if (amount > 0)
        {
            output = strprintf("%.*f%s", pl.round, amount,
                                pl.symbol.c_str());
        }

        for (unsigned int i = 2; i < ud.levels.size(); i++)
        {
            pl = ul;
            ul = ud.levels[i];

            nextAmount = levelAmount / ul.count;
            levelAmount %= ul.count;

            if (levelAmount > 0) output = strprintf("%d%s",
                levelAmount, pl.symbol.c_str()) + output;

            if (!nextAmount)
                break;
            levelAmount = nextAmount;
        }

        return output;
    }

    for (unsigned int i = 0; i < ud.levels.size(); i++)
    {
        ul = ud.levels[i];
        if (amount < ul.count && ul.count > 0)
        {
            ul = ud.levels[i - 1];
            break;
        }
        amount /= ul.count;
    }

    return strprintf("%.*f%s", ul.round, amount, ul.symbol.c_str());
}

std::string Units::formatCurrency(int value)
{
    return formatUnit(value, UNIT_CURRENCY);
}

std::string Units::formatWeight(int value)
{
    return formatUnit(value, UNIT_WEIGHT);
}
