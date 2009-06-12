/*
 *
 *  Copyright (C) 2008, Waseda University. All rights reserved.
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

///
/// @brief  The repository for mapping database snapshots
/// @file   Services/Root/SnapshotStore.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  Janurary 2008
///

//$Id: SnapshotStore.h 349 2008-05-29 01:54:02Z hro $


#ifndef ARC_ROOT_SNAPSHOT_STORE
#define ARC_ROOT_SNAPSHOT_STORE

template <typename T, int SIZE>
class SnapshotStore
{
protected:
    T       _container[SIZE];
    int     _count;
    int     _wp;

public:
    SnapshotStore();

    void Initialize();

    ///
    /// Pushes the item to the stack.  The oldest item is pushed out when
    /// the stack is full.
    ///
    /// @param item         the new item
    /// @return             the oldest item
    ///
    T Push(T item);

    ///
    /// Pops an item from the top of the stack.
    ///
    /// @return             the latest item
    ///
    T Pop();

    ///
    /// Obtains the number of elements in the stack.
    ///
    /// @return             the number of elements
    ///
    UInt Length() const;
};

template <typename T, int SIZE>
SnapshotStore<T, SIZE>::SnapshotStore() : _count(0), _wp(0)
{
}

template <typename T, int SIZE>
void
SnapshotStore<T, SIZE>::Initialize()
{
    _count = 0;
    _wp = 0;
}

template <typename T, int SIZE>
T
SnapshotStore<T, SIZE>::Push(T item)
{
    T old = 0;

    if (_count == SIZE) {
        old = _container[_wp];
    }
    else {
        _count++;
    }

    _container[_wp] = item;
    if (_wp == SIZE - 1) {
        _wp = 0;
    }
    else {
        _wp++;
    }

    return old;
}

template <typename T, int SIZE>
T
SnapshotStore<T, SIZE>::Pop()
{
    if (_count == 0) {
        return 0;
    }

    if (_wp == 0) {
        _wp = SIZE - 1;
    }
    else {
        _wp--;
    }

    T item = _container[_wp];
    _count--;

    return item;
}

template <typename T, int SIZE>
UInt
SnapshotStore<T, SIZE>::Length() const
{
    return static_cast<UInt>(_count);
}

#endif // ARC_ROOT_SNAPSHOT_REPOSITORY

