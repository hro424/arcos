/*
 *
 *  Copyright (C) 2008, Waseda University.
 *  All rights reserved.
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
/// @file   Include/arc/Bitmap.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  March, 2008
///

//$Id: Bitmap.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_BITMAP_H
#define ARC_BITMAP_H

#include <String.h>
#include <Types.h>

#include <l4/kdebug.h>

#define BITMAP_INDEX(i)     ((i) >> BITMAP_BITS)

///
/// NOTE: These methods are not atomic nor reentrant.
///
class Bitmap
{
public:
    ///
    /// The exponent of the size of a unit.
    ///
    static const size_t BITMAP_BITS = 3;

    ///
    /// The bits of a unit.  The bitmap is an array of the units.
    ///
    static const size_t UNIT_LENGTH = 1 << BITMAP_BITS;

    static const size_t BITS_PER_BYTE = UNIT_LENGTH;

private:
    static const UInt   BITMAP_MASK = UNIT_LENGTH - 1;

    ///
    /// The bitmap.
    ///
    UByte*  _map;

    ///
    /// The number of bits in this map.
    ///
    size_t  _length;

public:

    ///
    /// Creates an empty bitmap.
    ///
    /// @param size     the number of bits
    ///
    Bitmap(size_t size);

    virtual ~Bitmap();

    ///
    /// Set all the bits to 1.
    ///
    void Set();

    ///
    /// Set the specified bit to 1.
    ///
    void Set(size_t pos);

    ///
    /// Set all the bits to 0.
    ///
    void Reset();

    ///
    /// Set the specified bit to 0.
    ///
    void Reset(size_t pos);

    ///
    /// Flips all the bits.
    ///
    void Flip();

    ///
    /// Flips the specified bit.
    ///
    void Flip(size_t pos);

    ///
    /// Tests if the specified bit is 1.
    ///
    Bool Test(size_t pos);

    UByte* GetMap();

    size_t Length();
};


inline
Bitmap::Bitmap(size_t size) : _length(size)
{
    _map = new UByte[(size + BITMAP_MASK) / UNIT_LENGTH];
}

inline
Bitmap::~Bitmap()
{
    delete[] _map;
}

inline void
Bitmap::Set()
{
    memset(_map, 1, (_length + BITMAP_MASK) / UNIT_LENGTH);
}

inline void
Bitmap::Set(size_t pos)
{
    if (0 <= pos && pos < _length) {
        _map[BITMAP_INDEX(pos)] |= (1 << ((pos) & BITMAP_MASK));
    }
}

inline void
Bitmap::Reset()
{
    memset(_map, 0, (_length + BITMAP_MASK) / UNIT_LENGTH);
}

inline void
Bitmap::Reset(size_t pos)
{
    if (0 <= pos && pos < _length) {
        _map[BITMAP_INDEX(pos)] &= ~(1 << ((pos) & BITMAP_MASK));
    }
}

inline void
Bitmap::Flip()
{
    for (size_t i = 0; i < (_length + BITMAP_MASK) / UNIT_LENGTH; ++i) {
        _map[i] ^= ~0;
    }
}

inline void
Bitmap::Flip(size_t pos)
{
    if (0 <= pos && pos < _length) {
        _map[BITMAP_INDEX(pos)] ^= (1 << ((pos) & BITMAP_MASK));
    }
}

inline Bool
Bitmap::Test(size_t pos)
{
    if (0 <= pos && pos < _length) {
        return (_map[BITMAP_INDEX(pos)] >> ((pos) & BITMAP_MASK)) & 1;
    }
    else {
        return false;
    }
}

inline UByte*
Bitmap::GetMap()
{
    return _map;
}

inline size_t
Bitmap::Length()
{
    return _length;
}

#endif // ARC_BITMAP_H

