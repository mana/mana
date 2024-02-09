/*
 *  The Mana Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
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

#include <cmath>

static const double PI = 3.14159265;


enum ChangeFunc
{
    FUNC_NONE,
    FUNC_SINE,
    FUNC_SAW,
    FUNC_TRIANGLE,
    FUNC_SQUARE
};

template <typename T> struct ParticleEmitterProp
{
    void set(T min, T max)
    {
        minVal = min;
        maxVal = max;
    }

    void set(T val)
    {
        set(val, val);
    }

    void setFunction(ChangeFunc func, T amplitude, int period, int phase)
    {
        changeFunc = func;
        changeAmplitude = amplitude;
        changePeriod = period;
        changePhase = phase;
    }

    T value(int tick)
    {
        tick += changePhase;
        T val = (T) (minVal + (maxVal - minVal) * (rand() / ((double) RAND_MAX + 1)));

        switch (changeFunc)
        {
            case FUNC_SINE:
                val += (T) std::sin(PI * 2 * ((double)(tick % changePeriod) / (double)changePeriod)) * changeAmplitude;
                break;
            case FUNC_SAW:
                val += (T) (changeAmplitude * ((double)(tick % changePeriod) / (double)changePeriod)) * 2 - changeAmplitude;
                break;
            case FUNC_TRIANGLE:
                if ((tick % changePeriod) * 2 < changePeriod)
                {
                    val += changeAmplitude - (T)((tick % changePeriod) / (double)changePeriod) * changeAmplitude * 4;
                }
                else
                {
                    val += changeAmplitude * -3 + (T)((tick % changePeriod) / (double)changePeriod) * changeAmplitude * 4;
                    // I have no idea why this works but it does
                }
                break;
            case FUNC_SQUARE:
                if ((tick % changePeriod) * 2 < changePeriod)
                    val += changeAmplitude;
                else
                    val -= changeAmplitude;
                break;
            case FUNC_NONE:
            default:
                //nothing
                break;
        }

        return val;
    }

    T minVal;
    T maxVal;

    ChangeFunc changeFunc = FUNC_NONE;
    T changeAmplitude;
    int changePeriod;
    int changePhase;
};
