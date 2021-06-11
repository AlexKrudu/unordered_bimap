#include <random>

#include "unordered_bimap.h"
#include "test-classes.h"
#include "gtest/gtest.h"

// All tests should pass even under valgrind memcheck.

TEST(unordered_bimap, leak_check) {
    unordered_bimap<int, int> b;

    std::mt19937 e(std::random_device{}());
    for (size_t i = 0; i < 10000; i++) {
        b.insert(e(), e());
    }
}

TEST(unoreder_bimap, simple) {
    unordered_bimap<int, int> b;
    b.insert(4, 4);
    EXPECT_EQ(b.at_right(4), b.at_left(4));
}


TEST(unordered_bimap, copies) {
    unordered_bimap<int, int> b;
    b.insert(3, 4);
    unordered_bimap<int, int> b1(b);
    auto res = (*b.find_left(3));
    EXPECT_EQ(res.second, 4);
    b1.insert(4, 5);
    EXPECT_EQ(b.find_left(4), b.end());

    b1.insert(10, -10);
    b = b1;
    EXPECT_NE(b.find_right(-10), b.end());
}

TEST(unordered_bimap, insert) {
    unordered_bimap<int, int> b;
    b.insert(4, 10);
    b.insert(10, 4);
    EXPECT_EQ((*b.find_right(4)).first, 10);
    EXPECT_EQ(b.at_left(10), 4);
}

TEST(unordered_bimap, insert_move) {
    unordered_bimap<int, test_object> b;
    test_object x(3), x2(3);
    b.insert(4, std::move(x));
    EXPECT_EQ(x.a, 0);
    EXPECT_EQ(b.at_right(x2), 4);
    EXPECT_EQ(b.at_left(4), x2);

    unordered_bimap<test_object, int> b2;
    test_object y(4), y2(4);
    b2.insert(std::move(y), 3);
    EXPECT_EQ(y.a, 0);
    EXPECT_EQ(b2.at_left(y2), 3);
    EXPECT_EQ(b2.at_right(3), y2);
}

TEST(unordered_bimap, at) {
    unordered_bimap<int, int> b;
    b.insert(4, 3);

    EXPECT_THROW(b.at_left(1), std::out_of_range);
    EXPECT_THROW(b.at_right(300), std::out_of_range);
    EXPECT_EQ(b.at_left(4), 3);
    EXPECT_EQ(b.at_right(3), 4);
}

TEST(unordered_bimap, find) {
    unordered_bimap<int, int> b;
    b.insert(3, 4);
    b.insert(4, 5);
    b.insert(42, 1000);

    EXPECT_EQ((*b.find_right(5)).first, 4);
    EXPECT_EQ((*b.find_left(3)).second, 4);
    EXPECT_EQ(b.find_left(3436), b.end());
    EXPECT_EQ(b.find_right(-1000), b.end());
}

TEST(unordered_bimap, empty) {
    unordered_bimap<int, int> b;
    EXPECT_TRUE(b.empty());
    b.insert(1, 1);
    EXPECT_FALSE(b.empty());
}

TEST(unordered_bimap, insert_exist) {
    unordered_bimap<int, int> b;
    b.insert(1, 2);
    b.insert(2, 3);
    b.insert(3, 4);
    EXPECT_EQ(b.size(), 3);
    auto it = b.insert(2, -1);
    EXPECT_EQ(it, b.end());
    EXPECT_EQ(b.size(), 3);
}

TEST(bimap, erase_iterator) {
    unordered_bimap<int, int> b;
    auto it = b.insert(1, 2);
    b.insert(5, 10);
    b.insert(100, 200);
    auto it1 = b.erase(it);
    EXPECT_EQ(b.size(), 2);
    EXPECT_EQ((*it1).first, 5);

    it = b.insert(-1, -2);
    auto itr = b.erase(it);
    EXPECT_EQ(b.size(), 2);
    EXPECT_EQ(itr, b.end());
}

TEST(bimap, erase_value) {
    unordered_bimap<int, int> b;

    b.insert(111, 222);
    b.insert(333, 444);
    EXPECT_TRUE(b.erase_left(111));
    EXPECT_EQ(b.size(), 1);
    EXPECT_FALSE(b.erase_right(333333));
    EXPECT_EQ(b.size(), 1);
}

TEST(bimap, erase_range) {
    unordered_bimap<int, int> b;

    b.insert(1, 2);
    auto f = b.insert(2, 3);
    b.insert(3, 4);
    auto l = b.insert(4, 5);
    b.insert(5, 6);

    auto it = b.erase_range(f, l);
    EXPECT_EQ((*it).first, 4);
    EXPECT_EQ(b.size(), 3);

    auto f1 = b.insert(100, 4);
    auto l1 = b.insert(200, 10);

    auto it1 = b.erase_range(f1, l1);
    EXPECT_EQ((*it1).second, 10);
    EXPECT_EQ(b.size(), 4);

    b.erase_range(b.begin(), b.end());
    EXPECT_TRUE(b.empty());
}

template
struct unordered_bimap<int, non_default_constructible>;
template
struct unordered_bimap<non_default_constructible, int>;


TEST(bimap, assigment) {
    unordered_bimap<int, int> a;
    a.insert(1, 4);
    a.insert(8, 8);
    a.insert(25, 17);
    a.insert(13, 37);
    auto b = a;
    EXPECT_EQ(a.size(), b.size());
    EXPECT_EQ(a, b);
    a = a;
    b = std::move(b);
    EXPECT_EQ(a.size(), b.size());
    EXPECT_EQ(a, b);
}

TEST(unordered_bimap, ordering) {
    unordered_bimap<std::size_t, std::size_t> b;
    std::mt19937 e(std::random_device{}());
    std::vector<std::size_t> keys;
    std::vector<std::size_t> values;
    std::size_t cur_key;
    std::size_t cur_value;
    for (int i = 0; i < 1000; i++) { //pretty slow algo but who cares
        do {
            cur_key = e();
        } while (std::find(keys.begin(), keys.end(), cur_key) != keys.end());
        keys.push_back(cur_key);
        do {
            cur_value = e();
        } while (std::find(values.begin(), values.end(), cur_value) != values.end());
        values.push_back(cur_value);
        b.insert(cur_key, cur_value);
    }
    int i = 0;
    for (auto elem : b) {
        ASSERT_EQ(elem.first, keys[i]);
        ASSERT_EQ(elem.second, values[i]);
        i++;
    }
}
