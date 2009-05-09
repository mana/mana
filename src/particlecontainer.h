/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
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

#ifndef PARTICLE_CONTAINER_H
#define PARTICLE_CONTAINER_H

#include <list>
#include <vector>

class Particle;

/**
 * Set of particle effects.  May be stacked with other ParticleContainers.  All
 * operations herein affect such stacked containers, unless the operations end
 * in `Locally'.
 */
class ParticleContainer
{
public:
    /**
     * Constructs a new particle container and assumes responsibility for
     * its parent (for all operations defined herein, except when ending in `Locally')
     *
     * delParent means that the destructor should also free the parent.
     */
    ParticleContainer(ParticleContainer *parent = NULL, bool delParent = true);
    virtual ~ParticleContainer();

    /**
     * Kills and removes all particle effects
     */
    void clear();

    /**
     * Kills and removes all particle effects (only in this container)
     */
    virtual void clearLocally() {}

    /**
     * Sets the positions of all elements
     */
    virtual void moveTo(float x, float y);

protected:
    bool mDelParent;                    /**< Delete mNext in destructor */
    ParticleContainer *mNext;           /**< Contained container, if any */
};

/**
 * Linked list of particle effects.
 */
class ParticleList : public ParticleContainer
{
public:
    ParticleList(ParticleContainer *parent = NULL, bool delParent = true);
    virtual ~ParticleList();

    /**
     * Takes control of and adds a particle
     */
    void addLocally(Particle *);

    /**
     * `kills' and removes a particle
     */
    void removeLocally(Particle *);

    virtual void clearLocally();

    virtual void moveTo(float x, float y);

protected:
    std::list<Particle *> mElements;    /**< Contained particle effects */
};

/**
 * Particle container with indexing facilities
 */
class ParticleVector : public ParticleContainer
{
public:
    ParticleVector(ParticleContainer *parent = NULL, bool delParent = true);
    virtual ~ParticleVector();

    /**
     * Sets a particle at a specified index.  Kills the previous particle
     * there, if needed.
     */
    virtual void setLocally(int index, Particle *particle);

    /**
     * Removes a particle at a specified index
     */
    virtual void delLocally(int index);

    virtual void clearLocally();
    virtual void moveTo(float x, float y);

protected:
    std::vector<Particle *> mIndexedElements;
};

#endif
