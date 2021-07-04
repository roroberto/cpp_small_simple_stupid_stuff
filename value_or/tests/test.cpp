#include "pch.h"
#include "..\value_or.h"
#include <optional>

using namespace s4;

struct A
{
    int va;

    bool operator== (const A&) const = default;
};

struct B : public A
{
    int vb;

    bool operator== (const B&) const = default;
};

struct C : public B
{
    int vc;

    bool operator== (const C&) const = default;
};

struct D : public C
{
    int vd;

    bool operator== (const D&) const = default;
};

template<typename T>
T& t(T& v)
{
    return v;
}

template<typename T>
const T& ct(const T& v)
{
    return v;
}


template<typename DT, typename T, typename PT>
void test_ptr_value_or(DT &default_value, T &value, PT &p, PT &n)
{
    EXPECT_EQ(value_or(default_value, n), default_value);
    EXPECT_EQ(value_or(default_value, p), value);
    EXPECT_EQ(value_or(default_value, nullptr, p), value);
    EXPECT_EQ(value_or(default_value, 0, p), value);
    EXPECT_EQ(value_or(default_value, NULL, p), value);
    EXPECT_EQ(value_or(default_value, n, p), value);
    EXPECT_EQ(value_or(default_value, p, n), value);

    EXPECT_EQ(value_or(t(default_value), n), default_value);
    EXPECT_EQ(value_or(default_value, t(n)), default_value);
    EXPECT_EQ(value_or(default_value, t(p)), value);
    EXPECT_EQ(value_or(default_value, nullptr, t(p)), value);
    EXPECT_EQ(value_or(default_value, 0, t(p)), value);
    EXPECT_EQ(value_or(default_value, NULL, t(p)), value);
    EXPECT_EQ(value_or(default_value, n, t(p)), value);
    EXPECT_EQ(value_or(default_value, t(p), n), value);

    EXPECT_EQ(t(value_or(default_value, n)), default_value);
    EXPECT_EQ(t(value_or(default_value, p)), value);
    EXPECT_EQ(t(value_or(default_value, nullptr, p)), value);
    EXPECT_EQ(t(value_or(default_value, 0, p)), value);
    EXPECT_EQ(t(value_or(default_value, NULL, p)), value);
    EXPECT_EQ(t(value_or(default_value, n, p)), value);
    EXPECT_EQ(t(value_or(default_value, p, n)), value);
}


template<typename DT, typename T1, typename T2, typename T3, typename T4>
void test_value_or(DT default_value, T1 init_value1, T2 init_value2, T3 init_value3, T4 init_value4)
{
    EXPECT_EQ(value_or(default_value, nullptr), default_value);
    EXPECT_EQ(value_or(default_value, 0), default_value);
    EXPECT_EQ(value_or(default_value, NULL), default_value);
    EXPECT_EQ(value_or(default_value, nullptr, nullptr), default_value);
    
    T1* p = &init_value1;
    T1* n = nullptr;
    test_ptr_value_or(default_value, init_value1, p, n);
    
    T1* const cp = &init_value1;
    T1* const cpn = nullptr;
    test_ptr_value_or(default_value, init_value1, cp, cpn);
    EXPECT_EQ(value_or(default_value, ct(cp), ct(cpn)), init_value1);
    EXPECT_EQ(value_or(default_value, ct(cpn), ct(cp)), init_value1);
    EXPECT_EQ(ct(value_or(default_value, ct(cpn), ct(cp))), init_value1);
    
    std::unique_ptr<T2> up = std::make_unique<T2>(init_value2);
    std::unique_ptr<T2> upn = nullptr;
    test_ptr_value_or(default_value, init_value2, up, upn);
        
    std::shared_ptr<T3> sp = std::make_shared<T3>(init_value3);
    std::shared_ptr<T3> spn = nullptr;
    test_ptr_value_or(default_value, init_value3, sp, spn);

    std::weak_ptr<T3> wp = sp;
    std::weak_ptr<T3> wpn = spn;
    test_ptr_value_or(default_value, init_value3, wp, wpn);

    std::optional<T4> o = init_value4;
    std::optional<T4> on = std::nullopt;
    test_ptr_value_or(default_value, init_value4, o, on);

    DT v = value_or(default_value, n, upn, spn, on, wpn);
    EXPECT_EQ(value_or(default_value, n, upn, spn, wpn, on), default_value);
    EXPECT_EQ(value_or(default_value, p, upn, spn, wpn, on), init_value1);
    EXPECT_EQ(value_or(default_value, n, up, spn, wpn, on), init_value2);
    EXPECT_EQ(value_or(default_value, n, upn, sp, wpn, on), init_value3);
    EXPECT_EQ(value_or(default_value, n, upn, spn, wp, on), init_value3);
    EXPECT_EQ(value_or(default_value, n, upn, spn, wpn, o), init_value4);
    EXPECT_EQ(value_or(default_value, n, upn, spn, o, wpn), init_value4);
}

TEST(Testvalue_or, NullTest) 
{
  EXPECT_EQ(value_or(1,nullptr), 1);
}
TEST(Testvalue_or, IntTest)
{
    test_value_or(1, 2, 3, 4, 5);
}

TEST(Testvalue_or, IntVectorTest)
{
    std::vector<int> ints1{ 1,1,2,3,4,5 };
    std::vector<int> ints2{ 2,1,2,3,4,5 };
    std::vector<int> ints3{ 3,1,2,3,4,5 };
    std::vector<int> ints4{ 4,1,2,3,4,5 };
    std::vector<int> def_ints{ 6, 7 };
    test_value_or(def_ints, ints1, ints2, ints3, ints4);
}

TEST(Testvalue_or, ConstIntVectorTest)
{
    const std::vector<int> ints1{ 1,1,2,3,4,5 };
    const std::vector<int> ints2{ 2,1,2,3,4,5 };
    const std::vector<int> ints3{ 3,1,2,3,4,5 };
    const std::vector<int> ints4{ 4,1,2,3,4,5 };
    const std::vector<int> def_ints{ 6, 7 };
    test_value_or(def_ints, ints1, ints2, ints3, ints4);
}

TEST(Testvalue_or, StringTest)
{
    std::string s1 = "a" ;
    std::string s2 = "b";
    std::string s3 = "c";
    std::string s4 = "d";
    std::string def = "z";
    test_value_or(def, s1, s2, s3, s4);
}

TEST(Testvalue_or, ConstStringTest)
{
    const std::string s1 = "a";
    const std::string s2 = "b";
    const std::string s3 = "c";
    const std::string s4 = "d";
    const std::string def = "z";
    test_value_or(def, s1, s2, s3, s4);
}

TEST(Testvalue_or, DerivedObj)
{
    A a{ 1 };
    B b1{ 2,3 };
    C c2{ 4,5,6 };
    D d3{ 6,7,8,9 };
    B b4{ 8,9 };
    test_value_or(a, b1, c2, d3, b4);
}