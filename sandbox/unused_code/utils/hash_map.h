#ifndef UTILS_HASH_MAP_H
#define UTILS_HASH_MAP_H

#include <cassert>
#include <string>

typedef unsigned int hash_t;

template<typename T> hash_t make_hash(const T &elem)
{
    size_t size = sizeof(elem) / sizeof(hash_t) + 1;
    hash_t buf[sizeof(elem) / sizeof(hash_t) + 1];
    memcpy(buf, &elem, sizeof(T));

    hash_t hash = buf[0];
    for (size_t i = 1; i < size; ++i)
    {
        hash += buf[i];
        hash ^= (hash << 16) ^ (buf[i] << 11);
    }

    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash ^= hash >> 6;

    return hash;
}

template<typename T> hash_t make_hash(const T *array, size_t size)
{
    hash_t *buf = new hash_t[size / sizeof(hash_t) + 1];
    memcpy(buf, array, size);
    size /= sizeof(hash_t);

    hash_t hash = buf[0];
    for (size_t i = 1; i < size; ++i)
    {
        hash += buf[i];
        hash ^= (hash << 16) ^ (buf[i] << 11);
    }

    delete[] buf;

    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash ^= hash >> 6;

    return hash;
}

template<> hash_t make_hash<std::string>(const std::string& elem)
{
    return make_hash(elem.c_str(), elem.size());
}

template<typename Key, typename Value> class hash_map
{
    typedef unsigned int size_t;

    struct Tree
    {
        hash_t key_hash;
        Value value;
        Tree *parent, *left, *right;

        Tree(hash_t key_hash, const Value& val, Tree *parent, Tree *left, Tree *right):
            key_hash(key_hash),
            value(val),
            parent(parent),
            left(left),
            right(right)
        {}

        ~Tree()
        {
            if (left)
                delete left;
            if (right)
                delete right;
            printf("deleting\n");
        }

        Tree* copy()
        {
            Tree *ret = new Tree(value, left, right);
            if (left)
                ret->left = left->copy();
            if (right)
                ret->right = right->copy();
        }

        void orphan()
        {
            left = right = parent = NULL;
        }
    };

public:

    class iterator
    {
        Tree *mNode;

    public:
        iterator(): mNode(NULL) {};
        explicit iterator(Tree* node): mNode(node) {}

        bool valid() const { return mNode != NULL; }

        iterator& operator ++()
        {
            assert(mNode);

            if (mNode->right)
            {
                mNode = mNode->right;
                while (mNode->left)
                    mNode = mNode->left;
            }
            else
            {
                hash_t hash = mNode->key_hash;

                mNode = mNode->parent;
                while (mNode && mNode->key_hash <= hash)
                {
                    Tree* right = mNode->right;
                    while (right && right->key_hash <= hash)
                        right = right->right;

                    if (right)
                    {
                        mNode = right;
                        break;
                    }

                    mNode = mNode->parent;
                }
            }

            return *this;
        }

        iterator operator ++(int) const
        {
            return ++iterator(*this);
        }

        iterator& operator --()
        {
            assert(mNode);

            if (mNode->left)
            {
                mNode = mNode->left;
                while (mNode->right)
                    mNode = mNode->right;
            }
            else
            {
                hash_t hash = mNode->key_hash;

                mNode = mNode->parent;
                while (mNode && mNode->key_hash >= hash)
                {
                    Tree* left = mNode->left;
                    while (left && left->key_hash >= hash)
                        left = left->left;

                    if (left)
                    {
                        mNode = left;
                        break;
                    }

                    mNode = mNode->parent;
                }
            }

            return *this;
        }

        iterator operator --(int) const
        {
            return --iterator(*this);
        }

        Value& operator *() const
        {
            assert(mNode);

            return mNode->value;
        }

        bool operator ==(const iterator& it) const { return mNode == it.mNode; }
        bool operator !=(const iterator& it) const { return mNode != it.mNode; }

        friend class hash_map<Key, Value>;
    };

private:

    Tree *mRoot;
    size_t mSize;
    iterator mBegin, mRBegin;

public:

    hash_map(): mRoot(NULL), mSize(0) {}
    hash_map(const hash_map &copy)
    {
        mRoot = copy.mRoot->copy();
    }
    /*hash_map(const hash_map &&old)
    {
        mRoot = old.mRoot;
        old.mRoot = NULL;
    }*/

    virtual ~hash_map()
    {
        if (mRoot)
            delete mRoot;
    }

    iterator insert(const Key& key, const Value& val)
    {
        hash_t hash = make_hash(key);

        if (!mRoot)
        {
            ++mSize;
            mRoot = new Tree(hash, val, NULL, NULL, NULL);
            return iterator(mRoot);
        }
        else
        {
            Tree* curr = mRoot;
            while (true)
            {
                if (curr->key_hash == hash)
                {
                    curr->value = val;
                    return iterator(curr);
                }

                if (curr->key_hash < hash)
                {
                    if (curr->right)
                        curr = curr->right;
                    else
                    {
                        curr->right = new Tree(hash, val, curr, NULL, NULL);
                        if (!mRBegin.mNode || hash > mRBegin.mNode->key_hash)
                            mRBegin = iterator(curr->right);
                        ++mSize;
                        return iterator(curr->right);
                    }
                }
                else
                {
                    if (curr->left)
                        curr = curr->left;
                    else
                    {
                        curr->left = new Tree(hash, val, curr, NULL, NULL);
                        if (!mBegin.mNode || hash < mBegin.mNode->key_hash)
                            mBegin = iterator(curr->left);
                        ++mSize;
                        return iterator(curr->right);
                    }
                }
            }
        }
    }

    iterator erase(const iterator& where)
    {
        assert(where.valid());

        iterator ret(where);
        ++ret;

        Tree **curr = where.mNode->parent ?
                        (where.mNode->parent->left == where.mNode ? &where.mNode->parent->left : &where.mNode->parent->right) :
                        &mRoot;

        if ((*curr)->right)
        {
            Tree *right = (*curr)->right;

            if (right->left)
            {
                Tree **rightmost = &((*curr)->left);
                if (*rightmost)
                {
                    while ((*rightmost)->right)
                        rightmost = &((*rightmost)->right);

                    right->left->parent = *rightmost;
                    (*rightmost)->right = right->left;
                    right->left = NULL;
                }
            }
            else
            {
                if (mRBegin.mNode == *curr)
                    mRBegin.mNode = (*curr)->parent;
            }

            right->parent = (*curr)->parent;
            right->left = (*curr)->left;
            if ((*curr)->left)
                ((*curr)->left->parent) = right;

            (*curr)->orphan();
            delete (*curr);
            *curr = right;
            --mSize;

            return ret;
        }
        else
        {
            Tree *left = (*curr)->left;
            if (left)
                left->parent = (*curr)->parent;
            else
                if (mBegin.mNode == *curr)
                    mBegin.mNode = (*curr)->parent;

            (*curr)->orphan();
            delete (*curr);
            *curr = left;
            --mSize;

            return ret;
        }

        return ret;
    }

    bool erase(const Key& key)
    {
        hash_t hash = make_hash(key);
        if (mRoot)
        {
            Tree **curr = &mRoot;
            while (*curr != NULL)
            {
                if ((*curr)->key_hash == hash)
                {
                    if ((*curr)->right)
                    {
                        Tree *right = (*curr)->right;

                        if (right->left)
                        {
                            Tree **rightmost = &((*curr)->left);
                            if (*rightmost)
                            {
                                while ((*rightmost)->right)
                                    rightmost = &((*rightmost)->right);

                                right->left->parent = *rightmost;
                                (*rightmost)->right = right->left;
                                right->left = NULL;
                            }
                        }
                        else
                        {
                            if (mRBegin.mNode == *curr)
                                mRBegin.mNode = (*curr)->parent;
                        }

                        right->parent = (*curr)->parent;
                        right->left = (*curr)->left;
                        if ((*curr)->left)
                            ((*curr)->left->parent) = right;

                        (*curr)->orphan();
                        delete (*curr);
                        *curr = right;
                        --mSize;

                        return true;
                    }
                    else
                    {
                        Tree *left = (*curr)->left;
                        if (left)
                            left->parent = (*curr)->parent;
                        else
                            if (mBegin.mNode == *curr)
                                mBegin.mNode = (*curr)->parent;

                        (*curr)->orphan();
                        delete (*curr);
                        *curr = left;
                        --mSize;

                        return true;
                    }
                }
                else if ((*curr)->key_hash < hash)
                    curr = &((*curr)->right);
                else
                    curr = &((*curr)->left);
            }
        }

        return false;
    }

    iterator find(const Key& key) const
    {
        Tree *curr = mRoot;
        hash_t hash = make_hash(key);

        while (curr != NULL)
        {
            if (curr->key_hash == hash)
                return iterator(curr);
            else if (curr->key_hash < hash)
                curr = curr->right;
            else
                curr = curr->left;
        }

        return iterator();
    }

    bool contains(const Key& key) const
    {
        return find(key).valid();
    }

    Value& operator[](const Key& key)
    {
        hash_t hash = make_hash(key);

        Tree *parent = NULL, **curr = &mRoot;
        while ((*curr != NULL) && ((*curr)->key_hash != hash))
        {
            if ((*curr)->key_hash < hash)
            {
                parent = *curr;
                curr = &((*curr)->right);
            }
            else
            {
                parent = *curr;
                curr = &((*curr)->left);
            }
        }

        if ((*curr) == NULL)
            *curr = new Tree(hash, Value(), parent, NULL, NULL);

        return (*curr)->value;
    }

    size_t size() const { return mSize; }

    iterator begin() const
    {
        return mBegin;
    }

    iterator rbegin() const
    {
        return mRBegin;
    }

    iterator end() const
    {
        return iterator();
    }

    void print_values(std::ostream& out, Tree* curr = NULL)
    {
        if (curr == NULL)
            curr = mRoot;

        out << (void*)curr->key_hash << ": " << curr->value << ", parent = " << (curr->parent ? (void*)curr->parent->key_hash : "NULL") << "\n";
        if (curr->left)
        {
            out << (void*)curr->key_hash << " left:  ";
            print_values(out, curr->left);
        }
        if (curr->right)
        {
            out << (void*)curr->key_hash << " right: ";
            print_values(out, curr->right);
        }
    }
};

#endif //UTILS_HASH_MAP_H
