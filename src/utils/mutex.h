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

#ifndef MUTEX_H
#define MUTEX_H

#include "log.h"

#include <SDL_thread.h>

/**
 * A mutex provides mutual exclusion of access to certain data that is
 * accessed by multiple threads.
 */
class Mutex
{
public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();

private:
    Mutex(const Mutex&);  // prevent copying
    Mutex& operator=(const Mutex&);

    SDL_mutex *mMutex;
};

/**
 * A convenience class for locking a mutex.
 */
class MutexLocker
{
public:
    MutexLocker(Mutex *mutex);
    MutexLocker(MutexLocker&&);
    ~MutexLocker();

private:
    MutexLocker(const MutexLocker&);  // prevent copying
    MutexLocker& operator=(const MutexLocker&);

    Mutex *mMutex;
};


inline Mutex::Mutex()
{
    mMutex = SDL_CreateMutex();
}

inline Mutex::~Mutex()
{
    SDL_DestroyMutex(mMutex);
}

inline void Mutex::lock()
{
    if (SDL_mutexP(mMutex) == -1)
        logger->log("Mutex locking failed: %s", SDL_GetError());
}

inline void Mutex::unlock()
{
    if (SDL_mutexV(mMutex) == -1)
        logger->log("Mutex unlocking failed: %s", SDL_GetError());
}


inline MutexLocker::MutexLocker(Mutex *mutex):
    mMutex(mutex)
{
    mMutex->lock();
}

inline MutexLocker::MutexLocker(MutexLocker&& rhs):
    mMutex(rhs.mMutex)
{
    rhs.mMutex = nullptr;
}

inline MutexLocker::~MutexLocker()
{
    if (mMutex)
        mMutex->unlock();
}

#endif // MUTEX_H
