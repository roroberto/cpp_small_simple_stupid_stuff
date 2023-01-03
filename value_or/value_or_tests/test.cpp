#include "../value_or/value_or.h"

#pragma warning( push )
#pragma warning( disable : 26495 )

#include "gtest/gtest.h"
#include <optional>

#pragma warning( pop )

using namespace s4;

typedef std::vector<int> vect;

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
void test_ptr_value_or(DT &default_value, T &value, PT &p, PT &n, PT &empty_ptr)
{
    EXPECT_EQ(value_or(default_value, n), default_value);
    EXPECT_EQ(value_or(default_value, p), value);
    EXPECT_EQ(value_or(default_value, nullptr, p), value);
    EXPECT_EQ(value_or(default_value, n, p), value);
    EXPECT_EQ(value_or(default_value, p, n), value);

    EXPECT_EQ(value_or(t(default_value), n), default_value);
    EXPECT_EQ(value_or(default_value, t(n)), default_value);
    EXPECT_EQ(value_or(default_value, t(p)), value);
    EXPECT_EQ(value_or(default_value, nullptr, t(p)), value);
    EXPECT_EQ(value_or(default_value, n, t(p)), value);
    EXPECT_EQ(value_or(default_value, t(p), n), value);
    EXPECT_EQ(value_or(t(default_value), p, n), value);

    EXPECT_EQ(value_or(default_value, empty_ptr), default_value);
    
    EXPECT_EQ(t(value_or(default_value, n)), default_value);
    EXPECT_EQ(t(value_or(default_value, p)), value);
    EXPECT_EQ(t(value_or(default_value, nullptr, p)), value);
    EXPECT_EQ(t(value_or(default_value, n, p)), value);
    EXPECT_EQ(t(value_or(default_value, p, n)), value);
}



template<typename DT, typename T1, typename T2, typename T3, typename T4>
void test_value_or(DT default_value, T1 init_value1, T2 init_value2, T3 init_value3, T4 init_value4)
{
    EXPECT_EQ(value_or(default_value, nullptr), default_value);
    EXPECT_EQ(value_or(default_value, nullptr, nullptr), default_value);
    
    T1* p = &init_value1;
    T1* n = nullptr;
    test_ptr_value_or(default_value, init_value1, p, n, n);
    
    T1* const cp = &init_value1;
    T1* const cpn = nullptr;
    test_ptr_value_or(default_value, init_value1, cp, cpn, cpn);
    EXPECT_EQ(value_or(default_value, ct(cp), ct(cpn)), init_value1);
    EXPECT_EQ(value_or(default_value, ct(cpn), ct(cp)), init_value1);
    EXPECT_EQ(ct(value_or(default_value, ct(cpn), ct(cp))), init_value1);
    
    std::unique_ptr<T2> up = std::make_unique<T2>(init_value2);
    std::unique_ptr<T2> upn = nullptr;
    std::unique_ptr<T2> u_empty;
    test_ptr_value_or(default_value, init_value2, up, upn, u_empty);
        
    std::shared_ptr<T3> sp = std::make_shared<T3>(init_value3);
    std::shared_ptr<T3> spn = nullptr;
    std::shared_ptr<T3> s_emptrty;
    test_ptr_value_or(default_value, init_value3, sp, spn, s_emptrty);


    std::optional<T4> o = init_value4;
    std::optional<T4> on = std::nullopt;
    std::optional<T4> o_empty;
    test_ptr_value_or(default_value, init_value4, o, on, o_empty);

    DT v = value_or(default_value, n, upn, spn, on);
    EXPECT_EQ(value_or(default_value, n, upn, spn, on), default_value);
    EXPECT_EQ(value_or(default_value, p, upn, spn, on), init_value1);
    EXPECT_EQ(value_or(default_value, n, up, spn, on), init_value2);
    EXPECT_EQ(value_or(default_value, n, upn, sp, on), init_value3);
    EXPECT_EQ(value_or(default_value, n, upn, spn, o), init_value4);
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
    vect ints1{ 1,1,2,3,4,5 };
    vect ints2{ 2,1,2,3,4,5 };
    vect ints3{ 3,1,2,3,4,5 };
    vect ints4{ 4,1,2,3,4,5 };
    vect def_ints{ 6, 7 };
    test_value_or(def_ints, ints1, ints2, ints3, ints4);
}

TEST(Testvalue_or, ConstIntVectorTest)
{
    const vect ints1{ 1,1,2,3,4,5 };
    const vect ints2{ 2,1,2,3,4,5 };
    const vect ints3{ 3,1,2,3,4,5 };
    const vect ints4{ 4,1,2,3,4,5 };
    const vect def_ints{ 6, 7 };
    test_value_or(def_ints, ints1, ints2, ints3, ints4);
}

TEST(Testvalue_or, ConstNonConstIntVectorTest)
{
    const vect ints1{ 1,1,2,3,4,5 };
    const vect ints2{ 2,1,2,3,4,5 };
    vect ints3{ 3,1,2,3,4,5 };
    vect ints4{ 4,1,2,3,4,5 };
    vect def_ints{ 6, 7 };
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

TEST(Testvalue_or, ConstNonConstStringTest)
{
    std::string s1 = "a";
    std::string s2 = "b";
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


