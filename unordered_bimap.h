#include <fstream>

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>

using namespace std;

template<typename Key, typename Value, typename KeyHash = std::hash<Key>, typename ValueHash = std::hash<Value>>
class unordered_bimap {
private:

    // size of hashtable has to be prime number
    const int PRIME_SIZE = 200003;

    struct node {
        Key key;
        Value value;
        node *prev = nullptr;
        node *next = nullptr;

        // Added constructors copy/move constructors for each parameter. (probably should've used std::forward)
        node(const Key &key, const Value &value, node *prev) : key(key), value(value), prev(prev) {}

        node(Key &&key, const Value &value, node *prev) : key(std::move(key)), value(value), prev(prev) {}

        node(const Key &key, Value &&value, node *prev) : key(key), value(std::move(value)), prev(prev) {}

        node(Key &&key, Value &&value, node *prev) : key(std::move(key)), value(std::move(value)), prev(prev) {}

    };

    // Pointers for a linked node list (needed for ordering)
    node *head = nullptr;
    node *tail = nullptr;

    // Hashtable is just a vector of collision lists (lists that contain values of same hash) of nodes.
    vector<list < node * >> *
    key_hashtable = new vector<list < node * >>
    (PRIME_SIZE);
    vector<list < node * >> *
    value_hashtable = new vector<list < node * >>
    (PRIME_SIZE);

    std::size_t elem_count = 0;


    struct iterator {
        node *data;

        // Storing tail pointer for valid end iterator decrement.
        node *const *tail_ptr;

        iterator() = default;

        iterator(node *data, node *const *tail_ptr) : data(data), tail_ptr(tail_ptr) {};

        iterator(std::nullptr_t, node *const *tail_ptr) : data(nullptr), tail_ptr(tail_ptr) {};

        pair<Key &, Value &> operator*() const noexcept {
            return std::pair<Key &, Value &>(data->key, data->value);
        }

        iterator &operator++() noexcept {
            data = data->next;
            return *this;
        }

        iterator operator++(int) noexcept {
            const auto cur = *this;
            data = data->next;
            return cur;
        }

        iterator &operator--() noexcept {
            if (data == nullptr) { ;
                data = *tail_ptr;
                return *this;
            }
            data = data->prev;
            return *this;
        }

        iterator operator--(int) noexcept {
            const auto cur = *this;
            data = data->prev;
            return cur;
        }

        bool operator==(const iterator &other) const noexcept {
            return this->data == other.data;
        }

        bool operator!=(const iterator &other) const noexcept {
            return this->data != other.data;
        }

    };

    // Insert a key-value node, return an iterator on inserted node or end() if key or value is already present
    iterator insert(node *elem) {
        if (present_left(elem->key) || present_right(elem->value)) {
            delete elem;
            return end();
        }
        if (tail == nullptr) {
            head = elem;
            tail = elem;
        } else {
            tail->next = elem;
        }
        tail = elem;
        std::size_t key_hash = KeyHash{}(elem->key) % PRIME_SIZE;
        std::size_t value_hash = ValueHash{}(elem->value) % PRIME_SIZE;
        auto &key_list = key_hashtable->at(key_hash);
        auto &value_list = value_hashtable->at(value_hash);
        key_list.push_back(elem);
        value_list.push_back(elem);
        elem_count++;
        return iterator(elem, &tail);
    }

    node *find_key(const Key &key) const {
        auto collision_list = key_hashtable->at(KeyHash{}(key) % PRIME_SIZE);
        auto res = find_if(collision_list.begin(), collision_list.end(), [&](node *n) {
            return key == n->key;
        });
        if (res == collision_list.end()) {
            return nullptr;
        } else {
            return *res;
        }
    }

    node *find_value(const Value &value) const {
        auto collision_list = value_hashtable->at(ValueHash{}(value) % PRIME_SIZE);
        auto res = find_if(collision_list.begin(), collision_list.end(), [&](node *n) {
            return value == n->value;
        });
        if (res == collision_list.end()) {
            return nullptr;
        } else {
            return *res;
        }
    }

public:

    // Removes all key-value pairs from bimap
    void clear() {
        auto i = begin();
        while (!empty()) {
            auto next = i;
            ++next;
            erase(i);
            i = next;
        }
    }


    ~unordered_bimap() {
        clear();
        delete this->key_hashtable;
        delete this->value_hashtable;
    }

    unordered_bimap() = default;

    unordered_bimap(const unordered_bimap &other) {
        for (auto pair : other) {
            this->insert(pair.first, pair.second);
        }
    }

    unordered_bimap(unordered_bimap &&other) noexcept: head(std::move(other.head)), tail(std::move(other.tail)),
                                                       key_hashtable(std::move(other.key_hashtable)),
                                                       value_hashtable(std::move(other.value_hashtable)),
                                                       elem_count(std::move(other.elem_count)) {}

    unordered_bimap &operator=(unordered_bimap const &other) {
        clear();
        for (auto pair : other) {
            this->insert(pair.first, pair.second);
        }
        return *this;
    }

    unordered_bimap &operator=(unordered_bimap &&other) {
        clear();
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(key_hashtable, other.key_hashtable);
        std::swap(value_hashtable, other.value_hashtable);
        std::swap(elem_count, other.elem_count);
        return *this;
    }

    // Check if element is present in key-hashtable
    bool present_left(const Key &key) {
        return find_key(key) != nullptr;
    }

    // Check if element is present in value-hashtable
    bool present_right(const Value &value) {
        return find_value(value) != nullptr;
    }

    // Overloads for both lvalue/rvalue key/value params
    iterator insert(const Key &key, const Value &value) {
        node *tmp = new node(key, value, tail);
        return insert(tmp);
    }

    iterator insert(const Key &key, Value &&value) {
        node *tmp = new node(key, std::move(value), tail);
        return insert(tmp);
    }

    iterator insert(Key &&key, const Value &value) {
        node *tmp = new node(std::move(key), value, tail);
        return insert(tmp);
    }

    iterator insert(Key &&key, Value &&value) {
        node *tmp = new node(std::move(key), std::move(value), tail);
        return insert(tmp);
    }


    // Removes an element corresponded by iterator, returns an iterator on element followed by deleted element
    iterator erase(iterator elem) {
        iterator result = iterator(elem.data->next, elem.tail_ptr);
        if (elem.data->prev != nullptr) {
            elem.data->prev->next = elem.data->next;
        } else {
            head = elem.data->next;
        }
        if (elem.data->next != nullptr) {
            elem.data->next->prev = elem.data->prev;
        } else {
            tail = elem.data->prev;
        }
        auto &key_list = key_hashtable->at(KeyHash{}(elem.data->key) % PRIME_SIZE);
        auto &value_list = value_hashtable->at(ValueHash{}(elem.data->value) % PRIME_SIZE);
        key_list.erase(std::find(key_list.begin(), key_list.end(), find_key(elem.data->key)));
        value_list.erase(std::find(value_list.begin(), value_list.end(), find_value(elem.data->value)));
        elem_count--;
        delete elem.data;
        return result;
    }

    // Removes elements in range [first, last).
    iterator erase_range(iterator first, iterator last) {
        auto it = first;
        while (it != last) {
            auto next = it;
            ++next;
            erase(it);
            it = next;
        }
        return it;
    }

    // Tries to remove element from bimap. If deletion succeeded returns true, otherwise false
    bool erase_left(const Key &key) {
        auto it = find_key(key);
        if (present_left(key)) {
            erase(iterator(it, &tail));
            return true;
        }
        return false;
    }

    bool erase_right(const Value &value) {
        auto it = find_value(value);
        if (present_right(value)) {
            erase(iterator(it, &tail));
            return true;
        }
        return false;
    }


    // Lookups element, returns an iterator on it.
    iterator find_left(const Key &key) {
        return iterator(find_key(key), &tail);
    }

    iterator find_right(const Value &value) {
        return iterator(find_value(value), &tail);
    }

    // Lookups {key, value} pair by given key, returns value.
    Value const &at_left(const Key &key) {
        node *it = find_key(key);
        if (it != nullptr) {
            return it->value;
        } else {
            throw std::out_of_range("");
        }
    }

    // Lookups {key, value} pair by given value, returns key.
    Key const &at_right(const Value &value) {
        node *it = find_value(value);
        if (it != nullptr) {
            return it->key;
        } else {
            throw std::out_of_range("");
        }
    }

    // Returns number of stored elements.
    [[nodiscard]] std::size_t size() const noexcept {
        return this->elem_count;
    }


    // Checks whether map is empty
    [[nodiscard]] bool empty() const noexcept {
        return elem_count == 0;
    }


    iterator begin() const noexcept {
        return iterator(head, &tail);
    }

    // Notice that incrementing or dereferencing end iterator is UB.
    iterator end() const noexcept {
        return iterator(nullptr, &tail);
    }
};

// comparison operators
template<typename Key, typename Value, typename KeyHash = std::hash<Key>,
        typename ValueHash = std::hash<Value>>
bool operator==(unordered_bimap<Key, Value, KeyHash, ValueHash> const &lhs,
                unordered_bimap<Key, Value, KeyHash, ValueHash> const &rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    auto it_left = lhs.begin();
    auto it_right = rhs.begin();
    while (it_left != lhs.end()) {
        if ((*it_left).first != (*it_right).first || (*it_left).second != (*it_right).second) {
            return false;
        }
        ++it_left;
        ++it_right;
    }
    return true;
}

template<typename Key, typename Value, typename KeyHash = std::hash<Key>,
        typename ValueHash = std::hash<Value>>
bool operator!=(unordered_bimap<Key, Value, KeyHash, ValueHash> const &lhs,
                unordered_bimap<Key, Value, KeyHash, ValueHash> const &rhs) {
    return !(lhs == rhs);
}
