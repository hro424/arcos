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
/// @brief  Templatized B+ tree
/// @file   Include/container/BPlusTree.h
/// @author Hiroo Ishikawa <ishikawa@dcl.info.waseda.ac.jp>
/// @since  February 2008
///

//$Id: BPlusTree.h 349 2008-05-29 01:54:02Z hro $

#ifndef ARC_CONTAINER_B_PLUS_TREE_H
#define ARC_CONTAINER_B_PLUS_TREE_H

#include <Debug.h>
#include <MapElement.h>
#include <List.h>
#include <Assert.h>
#include <Types.h>

//
//  B+ Tree class declaration
//

template <typename K, typename V, int SIZE> class BptNode;
template <typename K, typename V, int SIZE> class BptLeaf;

template <typename K, typename V, int SIZE>
class BPlusTree
{
protected:
    BptNode<K, V, SIZE>*    _root;

public:
    ///
    /// Construct an emptry B+ tree
    ///
    BPlusTree();

    virtual ~BPlusTree();

    ///
    /// Inserts the entry
    ///
    void Insert(K key, V value);

    ///
    /// Removes the entry
    ///
    Bool Remove(K key, V& value);

    ///
    /// Searches for the entry referenced by the key.
    ///
    /// @param key      the key for the search
    /// @param value    the value to be filled
    /// @return         true if the key is found, false otherwise
    ///
    Bool Search(K key, V& value);

    ///
    /// Replaces the entry reference by the key if the entry exists
    ///
    Bool Update(K key, V& value, V& save);

    ///
    /// Obtains the number of elements (leaves of the tree).
    ///
    size_t Length();

    ///
    /// Obtains the list that contains all elements.
    ///
    List<MapElement<K, V>*> *ToList();

    void Clear();

    void Print();
};


template <typename K, typename V, int SIZE>
class BptNode
{
private:
    BptNode<K, V, SIZE>*    _values[SIZE + 1];

    ///
    /// The current number of values
    ///
    Int                     _value_count;

protected:
    K                       _keys[SIZE];

    ///
    /// The current number of keys
    ///
    Int                     _count;

    ///
    /// The level of this node
    ///
    Int                     _level;

    BptNode() : _count(0), _level(0) { _value_count = 0; };
    BptNode(BptNode<K, V, SIZE>& obj) {};

    ///
    /// Splits this node into two.  The node is required to be full.
    ///
    virtual BptNode<K, V, SIZE>* Split();

    ///
    /// Merges the given node to this node.
    ///
    virtual void Merge(BptNode<K, V, SIZE>* node);

public:
    BptNode(BptNode<K, V, SIZE>* left, BptNode<K, V, SIZE>* right);

    virtual ~BptNode();

    virtual BptNode<K, V, SIZE>* Insert(K key, V value);

    BptNode<K, V, SIZE>* Insert(K key, BptNode<K, V, SIZE> *node);

    virtual Bool Remove(K key, V& value);

    virtual Bool Search(K key, V& value);

    virtual Bool Update(K key, V& value, V& save);

    Int Count() const { return _count; }

    ///
    /// Obtains the level of this node.
    ///
    Int Level() const { return _level; }

    ///
    /// Obtains the head element of this subtree
    ///
    virtual BptLeaf<K, V, SIZE>* Head();

    virtual void Print() const;
};


template <typename K, typename V, int SIZE>
class BptLeaf : public BptNode<K, V, SIZE>
{
private:
    V                       _values[SIZE];
    BptLeaf<K, V, SIZE>*    _next;

    virtual BptLeaf<K, V, SIZE>* Split();

    virtual void Merge(BptNode<K, V, SIZE>* node);
    void Merge(BptLeaf<K, V, SIZE>* node);

public:
    BptLeaf();

    virtual ~BptLeaf();

    BptLeaf<K, V, SIZE>* Insert(K key, V value);

    virtual Bool Remove(K key, V& value);

    virtual Bool Search(K key, V& value);

    virtual Bool Update(K key, V& value, V& save);

    virtual BptLeaf<K, V, SIZE>* Head();

    virtual void Print() const;

    friend class BPlusTree<K, V, SIZE>;
};


//---------------------------------------------------------------------------
//  BPlusTree implementation
//---------------------------------------------------------------------------

template <typename K, typename V, int SIZE>
BPlusTree<K, V, SIZE>::BPlusTree()
{
    _root = new BptLeaf<K, V, SIZE>();
}

template <typename K, typename V, int SIZE>
BPlusTree<K, V, SIZE>::~BPlusTree()
{
    delete _root;
}

template <typename K, typename V, int SIZE>
void
BPlusTree<K, V, SIZE>::Clear()
{
    delete _root;
    _root = new BptLeaf<K, V, SIZE>();
}

template <typename K, typename V, int SIZE>
void
BPlusTree<K, V, SIZE>::Insert(K key, V value)
{
    BptNode<K, V, SIZE>*   deriv = _root->Insert(key, value);
    if (deriv != 0) {
        // Create a new root
        BptNode<K, V, SIZE>* new_root = new BptNode<K, V, SIZE>(_root, deriv);
        _root = new_root;
    }
}

template <typename K, typename V, int SIZE>
Bool
BPlusTree<K, V, SIZE>::Remove(K key, V& value)
{
    return _root->Remove(key, value);
}

template <typename K, typename V, int SIZE>
Bool
BPlusTree<K, V, SIZE>::Search(K key, V& value)
{
    return _root->Search(key, value);
}

template <typename K, typename V, int SIZE>
Bool
BPlusTree<K, V, SIZE>::Update(K key, V& value, V& save)
{
    return _root->Update(key, value, save);
}

template <typename K, typename V, int SIZE>
size_t
BPlusTree<K, V, SIZE>::Length()
{
    size_t                  length = 0;
    for (BptLeaf<K, V, SIZE>* ptr = _root->Head();
         ptr != 0; ptr = ptr->_next) {
        length += ptr->_count;
    }
    return length;
}

template <typename K, typename V, int SIZE>
List<MapElement<K, V>*>*
BPlusTree<K, V, SIZE>::ToList()
{
    List<MapElement<K, V>*>*    list = new List<MapElement<K, V>*>;
    BptLeaf<K, V, SIZE>*        ptr = _root->Head();

    while (ptr != 0) {
        for (Int i = 0; i < ptr->_count; i++) {
            list->Append(new MapElement<K, V>(ptr->_keys[i], ptr->_values[i]));
        }
        ptr = ptr->_next;
    }

    return list;
}

template <typename K, typename V, int SIZE>
void
BPlusTree<K, V, SIZE>::Print()
{
    _root->Print();
    //printf("\n");
}


//---------------------------------------------------------------------------
//  BptNode implementation
//---------------------------------------------------------------------------

template <typename K, typename V, int SIZE>
BptNode<K, V, SIZE>::BptNode(BptNode<K, V, SIZE>* left,
                             BptNode<K, V, SIZE>* right)
{
    // Handles nodes
    _keys[0] = right->Head()->_keys[0];
    _values[0] = left;
    _values[1] = right;
    _count = 1;
    _value_count = 2;
    _level = left->_level + 1;
}

template <typename K, typename V, int SIZE>
BptNode<K, V, SIZE>::~BptNode()
{
    if (_level > 0) {
        for (Int i = 0; i < _count + 1; i++) {
            delete _values[i];
        }
    }
}

template <typename K, typename V, int SIZE>
BptNode<K, V, SIZE>*
BptNode<K, V, SIZE>::Insert(K key, V value)
{
    BptNode<K, V, SIZE>* deriv;

    int i;
    for (i = 0; i < _count; i++) {
        if (key < _keys[i]) {
            break;
        }
    }
    deriv = _values[i]->Insert(key, value);

    if (deriv != 0) {
        deriv = this->Insert(deriv->Head()->_keys[0], deriv);
    }

    return deriv;
}

template <typename K, typename V, int SIZE>
BptNode<K, V, SIZE>*
BptNode<K, V, SIZE>::Insert(K key, BptNode<K, V, SIZE>* node)
{
    // Note: we need to deal with keys and values differently because
    //       their lengths are different. 

    BptNode<K, V, SIZE>* deriv = 0;

    // Split the node if it's full.
    if (_value_count == SIZE + 1) {
        deriv = Split();
        if (key > deriv->_values[0]->_keys[0]) {
            deriv->Insert(key, node);
            EXIT;
            return deriv;
        }
    }

    // Find a room for the key
    Int i;
    for (i = 0; i < _count && _keys[i] < key; i++) ;

    // Make the room
    for (Int j = _value_count - 1; j >= i; j--) {
        _values[j + 1] = _values[j];
    }

    for (Int j = _count - 1; j >= i; j--) {
        _keys[j + 1] = _keys[j];
    }

    _keys[i] = key;
    _values[i + 1] = node;
    _count++;
    _value_count++;

    return deriv;
}

template <typename K, typename V, int SIZE>
Bool
BptNode<K, V, SIZE>::Remove(K key, V& value)
{
    Int i;

    for (i = 0; i < _count && _keys[i] <= key; i++) ;
    if (!_values[i]->Remove(key, value)) {
        return FALSE;
    }

    // Merge children if
    // 1. The number of ths children of this node is more than 3 (it will
    //    turn out to be n - 1 after the merge).
    // 2. The total number of childrens of the children to be merged fits the
    //    capacity

    if (1 < _count &&
        0 < i &&
        _values[i - 1]->_count + _values[i]->_count < SIZE) {

        _values[i - 1]->Merge(_values[i]);

        _count--;
        for (Int j = i - 1; j < _count; j++) {
            _keys[j] = _keys[j + 1];
        }
        _keys[_count] = 0;

        _value_count--;
        for (Int j = i; j < _value_count; j++) {
            _values[j] = _values[j + 1];
        }
        _values[_value_count] = 0;
    }

    if (1 < _count &&
        i < _count - 1 &&
        _values[i]->_count + _values[i + 1]->_count < SIZE) {

        _values[i]->Merge(_values[i + 1]);

        _count--;
        for (Int j = i; j < _count; j++) {
            _keys[j] = _keys[j + 1];
        }
        _keys[_count] = 0;
        
        _value_count--;
        for (Int j = i + 1; j < _value_count; j++) {
            _values[j] = _values[j + 1];
        }
        _values[_value_count] = 0;
    }
    
    return TRUE;
}

template <typename K, typename V, int SIZE>
Bool
BptNode<K, V, SIZE>::Search(K key, V& value)
{
    Int i;
    for (i = 0; i < _count && _keys[i] <= key; i++) ;
    return _values[i]->Search(key, value);
}

template <typename K, typename V, int SIZE>
Bool
BptNode<K, V, SIZE>::Update(K key, V& value, V& save)
{
    Int i;
    for (i = 0; i < _count && _keys[i] <= key; i++) ;
    return _values[i]->Update(key, value, save);
}

template <typename K, typename V, int SIZE>
BptNode<K, V, SIZE>*
BptNode<K, V, SIZE>::Split()
{
    // Note: we need to deal with keys and values differently because
    //       their lengths are different. 

    BptNode<K, V, SIZE> *second = new BptNode<K, V, SIZE>();

    // Split keys
    Int offset = _count / 2;
    second->_count = _count - offset - 1;
    for (Int i = 0; i < second->_count; i++) {
        second->_keys[i] = _keys[i + offset + 1];
        _keys[i + offset + 1] = 0;
    }
    _keys[offset] = 0;
    _count /= 2;

    // Split values
    //offset = _value_count / 2 + 1;
    offset++;
    second->_value_count = _value_count - offset;
    for (Int i = 0; i < second->_value_count; i++) {
        second->_values[i] = _values[i + offset];
        _values[i + offset] = 0;
    }
    _value_count = offset;

    // Copy attributes
    second->_level = _level;

    return second;
}

template <typename K, typename V, int SIZE>
void
BptNode<K, V, SIZE>::Merge(BptNode<K, V, SIZE>* node)
{
    Int offset = _value_count;

    for (Int i = 0; i < node->_value_count; i++) {
        _values[i + offset] = node->_values[i];
    }
    _value_count += node->_value_count;

    for (Int i = 0; i < node->_count; i++) {
        _keys[i + offset] = node->_keys[i];
    }

    _keys[_count] = node->Head()->_keys[0];
    _count += node->_count + 1;

    delete node;
}

template <typename K, typename V, int SIZE>
BptLeaf<K, V, SIZE>*
BptNode<K, V, SIZE>::Head()
{
    return _values[0]->Head();
}

template <typename K, typename V, int SIZE>
void
BptNode<K, V, SIZE>::Print() const
{
    /*
    Int i;
    for (i = 0; i < _count; i++) {
        _values[i]->Print();
        printf("\n");
        for (Int j = 0; j < _level; j++) {
            printf("\t");
        }
        std::cout << _keys[i] << "(lv." << _level
            << " #" << _count << " #" << _value_count << ")" << std::endl;
    }
    _values[i]->Print();
    */
}


//---------------------------------------------------------------------------
//  BptLeaf implementation
//---------------------------------------------------------------------------

template <typename K, typename V, int SIZE>
BptLeaf<K, V, SIZE>::BptLeaf() : BptNode<K, V, SIZE>()
{
    for (Int i = 0; i < SIZE; i++) {
        this->_keys[i] = 0;
        this->_values[i] = 0;
    }
    this->_next = 0;
}

template <typename K, typename V, int SIZE>
BptLeaf<K, V, SIZE>::~BptLeaf()
{
}

template <typename K, typename V, int SIZE>
BptLeaf<K, V, SIZE>*
BptLeaf<K, V, SIZE>::Insert(K key, V value)
{
    BptLeaf<K, V, SIZE>* deriv = 0;

    assert(this->_count <= SIZE);

    if (this->_count == SIZE) {
        // This leaf is full.
        deriv = Split();
        if (key > deriv->_keys[0]) {
            deriv->Insert(key, value);
            return deriv;
        }
    }

    // Look for a room for the key
    Int i;
    for (i = 0; i < this->_count && this->_keys[i] < key; i++) ;

    // Avoid duplication
    if (key == this->_keys[i]) {
        DOUT("avoid duplication\n");
        return 0;
    }

    // Make a room for the key and the value
    for (Int j = this->_count - 1 ; j >= i; j--) {
        this->_keys[j + 1] = this->_keys[j];
        this->_values[j + 1] = this->_values[j];
    }

    this->_keys[i] = key;
    this->_values[i] = value;
    this->_count++;

    return deriv;
}

template <typename K, typename V, int SIZE>
Bool
BptLeaf<K, V, SIZE>::Remove(K key, V& value)
{
    assert(this->_count <= SIZE);
    // Linear-search for the key
    Int i;
    for (i = 0; i < this->_count; i++) {
        if (this->_keys[i] == key) {
            value = this->_values[i];
            // Let the remaining elements move over
            for (Int j = i; j < this->_count; j++) {
                this->_keys[j] = this->_keys[j + 1];
                this->_values[j] = this->_values[j + 1];
            }
            this->_count--;
            this->_keys[this->_count] = 0;
            this->_values[this->_count] = 0;
            return TRUE;
        }
    }


    return FALSE;
}

template <typename K, typename V, int SIZE>
Bool
BptLeaf<K, V, SIZE>::Search(K key, V& value)
{
    assert(this->_count <= SIZE);
    for (Int i = 0; i < this->_count; i++) {
        if (this->_keys[i] == key) {
            value = this->_values[i];
            return TRUE;
        }
    }
    return FALSE;
}

template <typename K, typename V, int SIZE>
Bool
BptLeaf<K, V, SIZE>::Update(K key, V& value, V& save)
{
    assert(this->_count <= SIZE);
    for (Int i = 0; i < this->_count; i++) {
        if (this->_keys[i] == key) {
            save = this->_values[i];
            this->_values[i] = value;
            return TRUE;
        }
    }
    return FALSE;
}

template <typename K, typename V, int SIZE>
BptLeaf<K, V, SIZE>*
BptLeaf<K, V, SIZE>::Split()
{
    BptLeaf<K, V, SIZE> *second = new BptLeaf<K, V, SIZE>();

    UInt offset = this->_count / 2;
    second->_count = this->_count - offset;
    for (Int i = 0; i < second->_count; i++) {
        second->_keys[i] = this->_keys[i + offset];
        second->_values[i] = this->_values[i + offset];
    }
    this->_count = offset;

    // Update the linked list
    second->_next = this->_next;
    this->_next = second;

    return second;
}

template <typename K, typename V, int SIZE>
void
BptLeaf<K, V, SIZE>::Merge(BptNode<K, V, SIZE>* node)
{
    this->Merge(static_cast<BptLeaf<K, V, SIZE>*>(node));
}


template <typename K, typename V, int SIZE>
void
BptLeaf<K, V, SIZE>::Merge(BptLeaf<K, V, SIZE>* node)
{
    for (Int i = 0; i < node->_count; i++) {
        this->_keys[this->_count + i] = node->_keys[i];
        this->_values[this->_count + i] = node->_values[i];
    }

    this->_count += node->_count;
    this->_next = node->_next;

    delete node;
}

template <typename K, typename V, int SIZE>
BptLeaf<K, V, SIZE>*
BptLeaf<K, V, SIZE>::Head()
{
    return this;
}

template <typename K, typename V, int SIZE>
void
BptLeaf<K, V, SIZE>::Print() const
{
    /*
    std::cout << "\t\t\t\t\t\t\t\t[";
    for (Int i = 0; i < this->_count; i++) {
        std::cout << this->_keys[i] << ",";
    }
    std::cout << "]";
    */
}

#endif // ARC_CONTAINER_B_PLUS_TREE_H

