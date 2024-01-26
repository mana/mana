/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include <cassert>

#include "particle.h"
#include "particlecontainer.h"

ParticleContainer::ParticleContainer(ParticleContainer *parent,
                                     bool delParent):
    mDelParent(delParent),
    mNext(parent)
{}

ParticleContainer::~ParticleContainer()
{
    clearLocally();
    if (mDelParent)
        delete mNext;
}

void ParticleContainer::clear()
{
    clearLocally();
    if (mNext)
        mNext->clear();
}

void ParticleContainer::moveTo(float x, float y)
{
    if (mNext)
        mNext->moveTo(x, y);
}

// -- particle list ----------------------------------------

ParticleList::ParticleList(ParticleContainer *parent, bool delParent):
    ParticleContainer(parent, delParent)
{}

ParticleList::~ParticleList()
{}

void ParticleList::addLocally(Particle *particle)
{
    if (particle)
    {
        // The effect may not die without the beings permission or we segfault
        particle->disableAutoDelete();
        mElements.push_back(particle);
    }
}

void ParticleList::removeLocally(Particle *particle)
{
    std::list<Particle *>::iterator it, it_end;
    for (it = mElements.begin(), it_end = mElements.end();
         it != it_end;)
    {
        if (*it == particle)
        {
            (*it)->kill();
            it = mElements.erase(it);
        }
        else
            it++;
    }
}

void ParticleList::clearLocally()
{
    for (auto &element : mElements)
        element->kill();

    mElements.clear();
}

void ParticleList::moveTo(float x, float y)
{
    ParticleContainer::moveTo(x, y);

    for (auto it = mElements.begin();
         it != mElements.end();)
    {
        (*it)->moveTo(x, y);
        if ((*it)->isExtinct())
        {
            (*it)->kill();
            it = mElements.erase(it);
        }
        else
            it++;
    }
}

// -- particle vector ----------------------------------------

ParticleVector::ParticleVector(ParticleContainer *parent, bool delParent):
    ParticleContainer(parent, delParent)
{}

ParticleVector::~ParticleVector()
{}

void ParticleVector::setLocally(int index, Particle *particle)
{
    assert(index >= 0);

    delLocally(index);

    if (mIndexedElements.size() <= (unsigned) index)
        mIndexedElements.resize(index + 1);

    if (particle)
        particle->disableAutoDelete();
    mIndexedElements[index] = particle;
}

void ParticleVector::delLocally(int index)
{
    assert(index >= 0);

    if (mIndexedElements.size() <= (unsigned) index)
        return;

    Particle *p = mIndexedElements[index];
    if (p)
    {
        mIndexedElements[index] = nullptr;
        p->kill();
    }
}

void ParticleVector::clearLocally()
{
    for (unsigned int i = 0; i < mIndexedElements.size(); i++)
        delLocally(i);
}

void ParticleVector::moveTo(float x, float y)
{
    ParticleContainer::moveTo(x, y);

    for (auto &indexedElement : mIndexedElements)
    {
        if (indexedElement)
        {
            indexedElement->moveTo(x, y);

            if (indexedElement->isExtinct())
            {
                indexedElement->kill();
                indexedElement = NULL;
            }
        }
    }
}

