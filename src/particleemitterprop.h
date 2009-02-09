/*
 *  The Mana World
 *  Copyright (C) 2006  The Mana World Development Team
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

#include <cmath>

/**
 * Returns a random numeric value that is larger than or equal min and smaller
 * than max
 */

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
    ParticleEmitterProp():
        changeFunc(FUNC_NONE)
    {
    }

    void set(T min, T max)
    {
        minVal=min; maxVal=max;
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
                val += (T) std::sin(M_PI * 2 * ((double)(tick%changePeriod) / (double)changePeriod)) * changeAmplitude;
                break;
            case FUNC_SAW:
                val += (T) (changeAmplitude * ((double)(tick%changePeriod) / (double)changePeriod)) * 2 - changeAmplitude;
                break;
            case FUNC_TRIANGLE:
                if ((tick%changePeriod) * 2 < changePeriod)
                {
                    val += changeAmplitude - (T)((tick%changePeriod) / (double)changePeriod) * changeAmplitude * 4;
                } else {
                    val += changeAmplitude * -3 + (T)((tick%changePeriod) / (double)changePeriod) * changeAmplitude * 4;
                    // I have no idea why this works but it does
                }
                break;
            case FUNC_SQUARE:
                if ((tick%changePeriod) * 2 < changePeriod)
                {
                    val += changeAmplitude;
                } else {
                    val -= changeAmplitude;
                }
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

    ChangeFunc changeFunc;
    T changeAmplitude;
    int changePeriod;
    int changePhase;
};
