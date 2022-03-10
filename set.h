/*-------------------------------------------------------

    Implementation of std::set analog based on AVL tree
    For HSE algorithms & data structures course
    By Ivan Morozov, winter 2022

-------------------------------------------------------*/

#pragma once

#include <algorithm>
#include <initializer_list>

/*-------------------------------------------------------
    Class declaration
-------------------------------------------------------*/

template <typename ValueType>
class Set {
private:
    struct TreeNode;

public:
    using TreeNodeRef = TreeNode*&;

    //---------------------------------------------------
    // constructors & operator= & destructor

    Set() = default;
    template<typename iteratorType>
    Set(iteratorType first, iteratorType last);
    Set(std::initializer_list<ValueType> init);
    Set(const Set<ValueType>& other);

    Set& operator=(const Set& other);

    ~Set();

    //---------------------------------------------------
    // Methods

    size_t size() const;

    bool empty() const;

    void insert(const ValueType& value);

    void erase(const ValueType& value);

    //---------------------------------------------------
    // iterators

    class iterator {
    public:
        iterator() = default;
        iterator(TreeNode* node, const Set<ValueType>* parent);

        iterator& operator++();      // ++it
        iterator operator++(int);    // it++

        iterator& operator--();      // --it
        iterator operator--(int);    // it--

        const ValueType& operator*() const;
        const ValueType* operator->() const;

        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;

    private:
        TreeNode* node = nullptr;
        const Set<ValueType>* parent = nullptr;
    };

    iterator begin() const;

    iterator end() const;

    //---------------------------------------------------
    // Search methods

    iterator find(const ValueType& key) const;

    iterator lower_bound(const ValueType& key) const;

private:
    struct TreeNode {
        ValueType key = 0;
        size_t height = 1, cnt = 1;
        TreeNode* left = nullptr, * right = nullptr, * parent = nullptr;

        TreeNode() = default;
        explicit TreeNode(const ValueType& key, TreeNode* parent = nullptr) : key(key), parent(parent) {}
    };

    //---------------------------------------------------
    // Helper functions

    size_t height(const TreeNode* t) const {
        return t ? t->height : 0;
    }

    size_t cnt(const TreeNode* t) const {
        return t ? t->cnt : 0;
    }

    int getBalance(const TreeNode* t) const {
        return height(t->right) - height(t->left);
    }

    void update(TreeNode* t) const {
        if (!t) return;

        t->height = std::max(height(t->left), height(t->right)) + 1;
        t->cnt = cnt(t->left) + 1 + cnt(t->right);
        t->parent = nullptr;
        if (t->left) t->left->parent = t;
        if (t->right) t->right->parent = t;
    }

    bool isKeyEqual(const ValueType& a, const ValueType& b) const {
        return !(a < b) && !(b < a);
    }

    //---------------------------------------------------
    // Balance
    /*
             u      rotateRight->        v
            / \                         / \
           v   C                       A   u
          / \                              / \
         A   B      <-rotateLeft         B   C
    */
    void rotateRight(TreeNodeRef u) {
        TreeNode* v = u->left;
        u->left = v->right;
        v->right = u;
        update(u);
        update(v);
        u = v;
    }

    void rotateLeft(TreeNodeRef v) {
        TreeNode* u = v->right;
        v->right = u->left;
        u->left = v;
        update(v);
        update(u);
        v = u;
    }

    TreeNode* balance(TreeNodeRef t) {
        if (!t) return nullptr;

        update(t);
        int t_balance = getBalance(t);
        if (t_balance == +2) {
            if (getBalance(t->right) < 0)
                rotateRight(t->right);  // Big Right Rotate
            rotateLeft(t);
        }
        else if (t_balance == -2) {
            if (getBalance(t->left) > 0)
                rotateLeft(t->left);    // Big Left Rotate
            rotateRight(t);
        }
        return t;
    }

    //---------------------------------------------------
    // Methods

    static TreeNode* findMin(TreeNode* t) {
        while (t && t->left)
            t = t->left;
        return t;
    }

    static TreeNode* findMax(TreeNode* t) {
        while (t && t->right)
            t = t->right;
        return t;
    }

    TreeNode* eraseMin(TreeNode* t) {
        if (!t->left)
            return t->right;

        t->left = eraseMin(t->left);
        return balance(t);
    }

    TreeNode* eraseMax(TreeNode* t) {
        if (!t->right)
            return t->left;

        t->right = eraseMax(t->right);
        return balance(t);
    }

    static TreeNode* nextNode(TreeNode* t) {
        if (t->right)
            return findMin(t->right);
        while (t->parent && t->parent->right == t)
            t = t->parent;
        return t->parent;
    }

    static TreeNode* prevNode(TreeNode* t) {
        if (t->left)
            return findMax(t->left);
        while (t->parent && t->parent->left == t)
            t = t->parent;
        return t->parent;
    }

    void AVLInsert(TreeNodeRef t, const ValueType& key, TreeNode* parent = nullptr) {
        if (!t)
            t = new TreeNode(key, parent);

        if (isKeyEqual(t->key, key))
            return;
        if (t->key < key)
            AVLInsert(t->right, key, t);
        else
            AVLInsert(t->left, key, t);
        
        balance(t);
    }

    void AVLErase(TreeNodeRef t, const ValueType& key) {
        if (!t)
            return;

        if (isKeyEqual(t->key, key)) {
            TreeNode* l = t->left;
            TreeNode* r = t->right;
            delete t;
            
            t = l;
            if (r) {
                t = findMin(r);
                t->right = eraseMin(r);
                t->left = l;
            }
        } else if (t->key < key) {
            AVLErase(t->right, key);
        } else {
            AVLErase(t->left, key);
        }

        balance(t);
    }

    TreeNode* AVLFind(TreeNode* t, ValueType key) const {
        if (!t)
            return nullptr;
        if (isKeyEqual(t->key, key))
            return t;
        if (t->key < key)
            return AVLFind(t->right, key);
        else
            return AVLFind(t->left, key);
    }

    TreeNode* AVLLowerBound(TreeNode* t, ValueType key) const {
        if (!t)
            return nullptr;

        if (isKeyEqual(t->key, key))
            return t;

        if (key < t->key) {
            TreeNode* tmp = AVLLowerBound(t->left, key);
            return tmp ? tmp : t;
        }
        else {
            return AVLLowerBound(t->right, key);
        }
    }

    void copyTree(TreeNode* from, TreeNodeRef to) {
        if (!from)
            return void(to = nullptr);

        to = new TreeNode(*from);
        copyTree(from->left, to->left);
        copyTree(from->right, to->right);
        return update(to);
    }

    void deleteTree(TreeNodeRef t) {
        if (!t) return;

        deleteTree(t->left);
        deleteTree(t->right);
        delete t;
        t = nullptr;
    }

    //---------------------------------------------------

    TreeNode* root = nullptr;
};

/*-------------------------------------------------------
    Implementation
-------------------------------------------------------*/

template<typename ValueType>
template<typename iteratorType>
Set<ValueType>::Set(iteratorType first, iteratorType last) {
    for (; first != last; ++first)
        insert(*first);
}

template<typename ValueType>
Set<ValueType>::Set(std::initializer_list<ValueType> init) {
    for (const ValueType& value : init)
        insert(value);
}

template<typename ValueType>
Set<ValueType>::Set(const Set<ValueType>& other) {
    copyTree(other.root, root);
}

template<typename ValueType>
Set<ValueType>::~Set() {
    deleteTree(root);
}

template<typename ValueType>
Set<ValueType>& Set<ValueType>::operator=(const Set<ValueType>& other) {
    if (this != &other) {
        deleteTree(root);
        copyTree(other.root, root);
    }
    return *this;
}

template<typename ValueType>
size_t Set<ValueType>::size() const {
    return cnt(root);
}

template<typename ValueType>
bool Set<ValueType>::empty() const {
    return size() == 0;
}

template<typename ValueType>
void Set<ValueType>::insert(const ValueType& value) {
    AVLInsert(root, value);
}

template<typename ValueType>
void Set<ValueType>::erase(const ValueType& value) {
    AVLErase(root, value);
}

template<typename ValueType>
Set<ValueType>::iterator::iterator(TreeNode* node, const Set<ValueType>* parent) : node(node), parent(parent) {}

template<typename ValueType>
typename Set<ValueType>::iterator& Set<ValueType>::iterator::operator++() {
    node = nextNode(node);
    return *this;
}

template<typename ValueType>
typename Set<ValueType>::iterator Set<ValueType>::iterator::operator++(int) {
    iterator old(*this);
    node = nextNode(node);
    return old;
}

template<typename ValueType>
typename Set<ValueType>::iterator& Set<ValueType>::iterator::operator--() {
    node = node ? prevNode(node) : findMax(parent->root);
    return *this;
}

template<typename ValueType>
typename Set<ValueType>::iterator Set<ValueType>::iterator::operator--(int) {
    iterator old(*this);
    node = node ? prevNode(node) : findMax(parent->root);
    return old;
}

template<typename ValueType>
const ValueType& Set<ValueType>::iterator::operator*() const {
    return node->key;
}

template<typename ValueType>
const ValueType* Set<ValueType>::iterator::operator->() const {
    return &(node->key);
}

template<typename ValueType>
bool Set<ValueType>::iterator::operator==(const iterator& other) const {
    return this->node == other.node && this->parent == other.parent;
}

template<typename ValueType>
bool Set<ValueType>::iterator::operator!=(const iterator& other) const {
    return this->node != other.node || this->parent != other.parent;
}

template<typename ValueType>
typename Set<ValueType>::iterator Set<ValueType>::begin() const {
    return iterator(findMin(root), this);
}

template<typename ValueType>
typename Set<ValueType>::iterator Set<ValueType>::end() const {
    return iterator(nullptr, this);
}

template<typename ValueType>
typename Set<ValueType>::iterator Set<ValueType>::find(const ValueType& key) const {
    return iterator(AVLFind(root, key), this);
}

template<typename ValueType>
typename Set<ValueType>::iterator Set<ValueType>::lower_bound(const ValueType& key) const {
    return iterator(AVLLowerBound(root, key), this);
}


