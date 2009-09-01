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
/// @file   Include/container/List.h
/// @brief  Generic linked list and iterator
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  January 2008
///


#ifndef ARC_CONTAINER_LIST_H
#define ARC_CONTAINER_LIST_H

#include <Types.h>

template <typename T> class List;
template <typename T> class Iterator;

template <typename T>
class ListElement
{
protected:
    ListElement<T>* _next;
    T               _object;

public:
    ListElement(const T& obj) : _next(0), _object(obj) {}
    virtual ~ListElement() {}

    friend class List<T>;
    friend class Iterator<T>;
};


template <typename T>
class List
{
protected:
    ListElement<T>* _head;
    Iterator<T>     _it;
    size_t          _length;

public:
    List() : _head(0), _length(0) {}

    ~List()
    {
        ListElement<T>* cur = _head;
        while (cur != 0) {
            ListElement<T>* elem = cur;
            cur = cur->_next;
            delete elem;
        }
        _length = 0;
    }

    ///
    /// Adds the object to the top of the list.  Duprication of the same
    /// object is allowed.
    ///
    void Add(const T& obj)
    {
        ListElement<T>* elem = new ListElement<T>(obj);
        elem->_next = _head;
        _head = elem;
        _length++;
    }

    ///
    /// Appends the object to the end of the list.  Duprication of the same
    /// object is allowed.
    ///
    void Append(const T& obj)
    {
        ListElement<T>** cur = &_head;
        while (*cur != 0) {
            cur = &((*cur)->_next);
        }

        ListElement<T>* elem = new ListElement<T>(obj);
        elem->_next = 0;
        *cur = elem;
        _length++;
    }

    ///
    /// Removes the object from the list.  The first occurance is removed if
    /// there are multiple of the object exist in the list.
    ///
    void Remove(const T& obj)
    {
        ListElement<T>** cur = &_head;

        while (*cur != 0) {     // Because an illegal object may be given.
            if ((*cur)->_object == obj) {
                ListElement<T> *elem = *cur;
                *cur = elem->_next;
                delete elem;
                _length--;
                break;
            }
            cur = &((*cur)->_next);
        }
    }

    ///
    /// Obtains the current number of the elements.
    ///
    size_t Length() { return _length; }

    Iterator<T>& GetIterator()
    {
        _it.Initialize(_head);
        return _it;
    }
};


template <typename T>
class Iterator
{
protected:
    ListElement<T>* _cursor;

    void Initialize(ListElement<T>* head) { _cursor = head; }

public:
    Iterator() : _cursor(0) {}

    virtual ~Iterator() {}

    bool HasNext() { return _cursor != 0; }

    T Next()
    {
        T obj = _cursor->_object;
        _cursor = _cursor->_next;
        return obj;
    }

    friend class List<T>;
};

#endif // ARC_ROOT_LIST_H

