#pragma once




#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>

namespace test_framework {

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar {
    Registrar(const std::string& name, std::function<void()> fn) {
        registry().push_back({name, std::move(fn)});
    }
};

struct AssertionFailure {
    std::string message;
};




template <typename T>
std::string to_debug_string(const T& value) {
    if constexpr (std::is_enum<T>::value) {
        return std::to_string(static_cast<typename std::underlying_type<T>::type>(value));
    } else {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
}

} 

#define TEST(test_name)                                                          \
    static void test_name();                                                     \
    static ::test_framework::Registrar registrar_##test_name(#test_name, test_name); \
    static void test_name()

#define ASSERT_TRUE(cond)                                                        \
    do {                                                                         \
        if (!(cond)) {                                                           \
            throw ::test_framework::AssertionFailure{                            \
                std::string("ASSERT_TRUE failed: ") + #cond +                    \
                " (" __FILE__ ":" + std::to_string(__LINE__) + ")"};             \
        }                                                                        \
    } while (0)

#define ASSERT_FALSE(cond)                                                       \
    do {                                                                         \
        if ((cond)) {                                                            \
            throw ::test_framework::AssertionFailure{                            \
                std::string("ASSERT_FALSE failed: ") + #cond +                   \
                " (" __FILE__ ":" + std::to_string(__LINE__) + ")"};             \
        }                                                                        \
    } while (0)

#define ASSERT_EQ(a, b)                                                          \
    do {                                                                         \
        auto _a = (a);                                                          \
        auto _b = (b);                                                          \
        if (!(_a == _b)) {                                                       \
            throw ::test_framework::AssertionFailure{                           \
                "ASSERT_EQ failed: " + std::string(#a) + " != " + std::string(#b) + \
                " (" __FILE__ ":" + std::to_string(__LINE__) + ")" +            \
                "  [left=" + ::test_framework::to_debug_string(_a) +            \
                " right=" + ::test_framework::to_debug_string(_b) + "]"};       \
        }                                                                        \
    } while (0)

inline int run_all_registered_tests() {
    using namespace test_framework;
    int passed = 0;
    int failed = 0;
    for (const auto& t : registry()) {
        try {
            t.fn();
            std::cout << "  [PASS] " << t.name << "\n";
            ++passed;
        } catch (const AssertionFailure& af) {
            std::cout << "  [FAIL] " << t.name << " -- " << af.message << "\n";
            ++failed;
        } catch (const std::exception& ex) {
            std::cout << "  [FAIL] " << t.name << " -- unexpected exception: " << ex.what() << "\n";
            ++failed;
        } catch (...) {
            std::cout << "  [FAIL] " << t.name << " -- unknown exception\n";
            ++failed;
        }
    }
    std::cout << "\n" << passed << " passed, " << failed << " failed, "
               << (passed + failed) << " total\n";
    return failed == 0 ? 0 : 1;
}


