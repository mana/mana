/*
 *  The Mana World
 *  Copyright 2008 The Mana World Development Team
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
 */

#include <cassert>

#include "particlecontainer.h"


ParticleContainer::ParticleContainer(ParticleContainer *parent, bool delParent) :
    mDelParent(delParent),
    mNext(parent)
{};

ParticleContainer::~ParticleContainer()
{
    clearLocally();
    if (mNext && mDelParent)
        delete mNext;
}

void
ParticleContainer::addLocally(Particle *particle)
{
    if (particle)
    {
        // The effect may not die without the beings permission or we segfault
        particle->disableAutoDelete();
        mElements.push_back(particle);
    }
}

void
ParticleContainer::removeLocally(Particle *particle)
{
    for (std::list<Particle *>::iterator it = mElements.begin();
         it != mElements.end(); it++)
        if (*it == particle) {
            (*it)->kill();
            mElements.erase(it);
        }
}

void
ParticleContainer::clear()
{
    clearLocally();
    if (mNext)
        mNext->clear();
}

void
ParticleContainer::clearLocally()
{
    for (std::list<Particle *>::iterator it = mElements.begin();
         it != mElements.end(); it++)
        (*it)->kill();
    
    mElements.clear();
}

void
ParticleContainer::setPositions(float x, float y)
{
    for (std::list<Particle *>::iterator it = mElements.begin();
         it != mElements.end();)
    {
        (*it)->setPosition(x, y);
        if ((*it)->isExtinct())
        {
            (*it)->kill();
            it = mElements.erase(it);
        }
        else
            it++;
    }
}




ParticleVector::ParticleVector(ParticleContainer *next, bool delParent)
{
    ParticleContainer::ParticleContainer(next, delParent);
}

void
ParticleVector::setLocally(int index, Particle *particle)
{
    assert(index >= 0);

    delLocally(index);

    if (mIndexedElements.size() <= (unsigned) index)
        mIndexedElements.resize(index + 1, NULL);

    mIndexedElements[index] = particle;
}

void
ParticleVector::delLocally(int index)
{
    assert(index >= 0);

    if (mIndexedElements.size() <= (unsigned) index)
        return;

    Particle *p = mIndexedElements[index];
    if (p)
    {
        mIndexedElements[index] = NULL;
        p->kill();
    }
}

void
ParticleVector::clearLocally()
{
    for (unsigned int i = 0; i < mIndexedElements.size(); i++)
        delLocally(i);
}

void
ParticleVector::setPositions(float x, float y)
{
    ParticleContainer::setPositions(x, y);

    for (std::vector<Particle *>::iterator it = mIndexedElements.begin();
         it != mIndexedElements.end(); it++)
        if (*it)
        {
            (*it)->setPosition(x, y);

            if ((*it)->isExtinct())
            {
                (*it)->kill();
                *it = NULL;
            }
        }
}

