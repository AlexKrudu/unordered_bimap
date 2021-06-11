#pragma once

struct test_object {
    int a = 0;

    ~test_object() = default;

    test_object() = default;

    explicit test_object(int b) : a(b) {}

    test_object(test_object &&other) noexcept { std::swap(a, other.a); }

    friend bool operator<(test_object const &c, test_object const &b) {
        return c.a < b.a;
    }

    friend bool operator==(test_object const &c, test_object const &b) {
        return c.a == b.a;
    }
};

struct non_default_constructible {
    non_default_constructible() = delete;

    explicit non_default_constructible(int b) : a(b) {}

    non_default_constructible(non_default_constructible const &) = default;

    friend bool operator<(non_default_constructible const &c, non_default_constructible const &b) {
        return c.a < b.a;
    }

    friend bool operator==(non_default_constructible const &c, non_default_constructible const &b) {
        return c.a == b.a;
    }

    int get_a() {
        return a;
    }

private:
    int a;
};

namespace std {
    template<>
    struct hash<test_object> {
        std::size_t operator()(const test_object &x) const {
            return std::hash<int>{}(x.a);
        }
    };

    template<>
    struct hash<non_default_constructible> {
        std::size_t operator()(non_default_constructible x) const {
            return std::hash<int>{}(x.get_a());
        }
    };

}