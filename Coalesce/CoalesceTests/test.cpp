#include "../Coalesce/coalesce.h"
#include "gtest/gtest.h"
#include <optional>

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


template<typename T>
T test_f();


template<>
int test_f<int>()
{
    return 123;
}

template<>
int* test_f<int*>()
{
    static int i = test_f<int>();
    return &i;
}

template<>
const int* test_f<const int*>()
{
    static int i = test_f<int>();
    return &i;
}

template<>
std::string test_f<std::string>()
{
    return "test_f";
}

template<>
std::string* test_f<std::string*>()
{
    static std::string i = test_f<std::string>();
    return &i;
}


template<>
const std::string* test_f<const std::string*>()
{
    static std::string i = test_f<std::string>();
    return &i;
}

template<>
vect test_f<vect>()
{
    return vect{ 0,0,0,0 };
}

template<>
vect* test_f<vect*>()
{
    static vect i = test_f<vect>();
    return &i;
}

template<>
const vect* test_f<const vect*>()
{
    static vect i = test_f<vect>();
    return &i;
}


template<>
A test_f<A>()
{
    return A{ 0 };
}


template<>
A* test_f<A*>()
{
    static A i = test_f<A>();
    return &i;
}


template<>
const A* test_f<const A*>()
{
    static A i = test_f<A>();
    return &i;
}


template<>
B test_f<B>()
{
    return B{ 0, 0 };
}


template<>
B* test_f<B*>()
{
    static B i = test_f<B>();
    return &i;
}


template<>
const B* test_f<const B*>()
{
    static B i = test_f<B>();
    return &i;
}


template<>
C test_f<C>()
{
    return C{ 0, 0, 0 };
}


template<>
C* test_f<C*>()
{
    static C i = test_f<C>();
    return &i;
}


template<>
const C* test_f<const C*>()
{
    static C i = test_f<C>();
    return &i;
}


template<>
D test_f<D>()
{
    return D{ 0, 0, 0, 0 };
}


template<>
D* test_f<D*>()
{
    static D i = test_f<D>();
    return &i;
}


template<>
const D* test_f<const D*>()
{
    static D i = test_f<D>();
    return &i;
}

template<typename T>
T* test_fn()
{
    return nullptr;
}

template<typename T>
struct func_obj
{
    T& v;

    func_obj(T& i) : v{ i } {};

    T& operator()() 
    {
        return v;
    }

    bool operator==(const T& op) const
    {
        return v == op;
    }
};

template<typename DT, typename T, typename PT>
void test_ptr_coalesce(DT &default_value, T &value, PT &p, PT &n)
{
    EXPECT_EQ(coalesce(default_value, n), default_value);
    EXPECT_EQ(coalesce(default_value, p), value);
    EXPECT_EQ(coalesce(default_value, nullptr, p), value);
    EXPECT_EQ(coalesce(default_value, n, p), value);
    EXPECT_EQ(coalesce(default_value, p, n), value);

    EXPECT_EQ(coalesce(t(default_value), n), default_value);
    EXPECT_EQ(coalesce(default_value, t(n)), default_value);
    EXPECT_EQ(coalesce(default_value, t(p)), value);
    EXPECT_EQ(coalesce(default_value, nullptr, t(p)), value);
    EXPECT_EQ(coalesce(default_value, n, t(p)), value);
    EXPECT_EQ(coalesce(default_value, t(p), n), value);
    EXPECT_EQ(coalesce(t(default_value), p, n), value);


    EXPECT_EQ(t(coalesce(default_value, n)), default_value);
    EXPECT_EQ(t(coalesce(default_value, p)), value);
    EXPECT_EQ(t(coalesce(default_value, nullptr, p)), value);
    EXPECT_EQ(t(coalesce(default_value, n, p)), value);
    EXPECT_EQ(t(coalesce(default_value, p, n)), value);

    
    EXPECT_EQ(coalesce(func_obj{ default_value }, n), default_value);
    
    func_obj fp{ p };
    EXPECT_EQ(coalesce(default_value,fp), value);
    EXPECT_EQ(coalesce(default_value, func_obj{ p }), value);
    EXPECT_EQ(coalesce(default_value, func_obj{ n }), default_value);
    EXPECT_EQ(coalesce(func_obj{ default_value }, fp), value);
    EXPECT_EQ(coalesce(func_obj{ default_value }, func_obj{ p }), value);
    EXPECT_EQ(coalesce(func_obj{ default_value }, func_obj{ n }), default_value);
    /*
    std::function<decltype(fp)> std_fun_ni;
    EXPECT_EQ(coalesce(default_value, std_fun_ni), default_value);
    EXPECT_EQ(coalesce(default_value, std_fun_ni, fp), value);

    func_obj fpn{ n };
    std::function<decltype(fp)> std_fun_n{ fpn };
    EXPECT_EQ(coalesce(default_value, std_fun_n), default_value);
    EXPECT_EQ(coalesce(default_value, std_fun_n, fp), value);


    std::function<decltype(fp)> std_fun{ fp };
    EXPECT_EQ(coalesce(default_value, std_fun), value);
    EXPECT_EQ(coalesce(default_value, std_fun, fp), value);
    */
}


template<typename DT, typename T1, typename T2, typename T3, typename T4>
void test_coalesce(DT default_value, T1 init_value1, T2 init_value2, T3 init_value3, T4 init_value4)
{
    EXPECT_EQ(coalesce(default_value, nullptr), default_value);
    EXPECT_EQ(coalesce(default_value, nullptr, nullptr), default_value);
    EXPECT_EQ(coalesce(func_obj{ default_value }, nullptr), default_value);
    EXPECT_EQ(coalesce(func_obj{ default_value }, nullptr, nullptr), default_value);
    
    T1* p = &init_value1;
    T1* n = nullptr;
    test_ptr_coalesce(default_value, init_value1, p, n);
    
    T1* const cp = &init_value1;
    T1* const cpn = nullptr;
    test_ptr_coalesce(default_value, init_value1, cp, cpn);
    EXPECT_EQ(coalesce(default_value, ct(cp), ct(cpn)), init_value1);
    EXPECT_EQ(coalesce(default_value, ct(cpn), ct(cp)), init_value1);
    EXPECT_EQ(ct(coalesce(default_value, ct(cpn), ct(cp))), init_value1);
    EXPECT_EQ(ct(coalesce(func_obj{ default_value }, ct(cpn), ct(cp))), init_value1);
    
    std::unique_ptr<T2> up = std::make_unique<T2>(init_value2);
    std::unique_ptr<T2> upn = nullptr;
    test_ptr_coalesce(default_value, init_value2, up, upn);
        
    std::shared_ptr<T3> sp = std::make_shared<T3>(init_value3);
    std::shared_ptr<T3> spn = nullptr;
    test_ptr_coalesce(default_value, init_value3, sp, spn);

    std::weak_ptr<T3> wp = sp;
    std::weak_ptr<T3> wpn = spn;
    test_ptr_coalesce(default_value, init_value3, wp, wpn);

    std::optional<T4> o = init_value4;
    std::optional<T4> on = std::nullopt;
    test_ptr_coalesce(default_value, init_value4, o, on);

    DT v = coalesce(default_value, n, upn, spn, on, wpn);
    EXPECT_EQ(coalesce(default_value, n, upn, spn, wpn, on), default_value);
    EXPECT_EQ(coalesce(default_value, p, upn, spn, wpn, on), init_value1);
    EXPECT_EQ(coalesce(default_value, n, up, spn, wpn, on), init_value2);
    EXPECT_EQ(coalesce(default_value, n, upn, sp, wpn, on), init_value3);
    EXPECT_EQ(coalesce(default_value, n, upn, spn, wp, on), init_value3);
    EXPECT_EQ(coalesce(default_value, n, upn, spn, wpn, o), init_value4);
    EXPECT_EQ(coalesce(default_value, n, upn, spn, o, wpn), init_value4);

    EXPECT_EQ(coalesce(test_f<DT>(), nullptr), test_f<DT>());
    EXPECT_EQ(coalesce(test_f<DT>(), test_f<T1*>()), test_f<T1>());
    EXPECT_EQ(coalesce(test_f<DT>(), test_f<T1*>(), test_f<T2*>()), test_f<T1>());
    EXPECT_EQ(coalesce(test_f<DT>(), test_fn<T1>(), test_f<T2*>()), test_f<T2>());
    EXPECT_EQ(coalesce(test_f<DT>(), test_fn<T1>(), test_fn<T2>()), test_f<DT>());
}

TEST(UnitTestExtras, Test_func_obj)
{
    int i = 0;
    func_obj fo(i);
    EXPECT_EQ(fo(), i);
    EXPECT_EQ(func_obj{ i }, i);

    std::unique_ptr<int> up = std::make_unique<int>(10);;
    func_obj fup{ up };
    EXPECT_EQ(*fup(), 10);
}

TEST(Testcoalesce, NullTest) 
{
  EXPECT_EQ(coalesce(1,nullptr), 1);
}
TEST(Testcoalesce, IntTest)
{
    test_coalesce(1, 2, 3, 4, 5);
}

TEST(Testcoalesce, IntVectorTest)
{
    vect ints1{ 1,1,2,3,4,5 };
    vect ints2{ 2,1,2,3,4,5 };
    vect ints3{ 3,1,2,3,4,5 };
    vect ints4{ 4,1,2,3,4,5 };
    vect def_ints{ 6, 7 };
    test_coalesce(def_ints, ints1, ints2, ints3, ints4);
}

TEST(Testcoalesce, ConstIntVectorTest)
{
    const vect ints1{ 1,1,2,3,4,5 };
    const vect ints2{ 2,1,2,3,4,5 };
    const vect ints3{ 3,1,2,3,4,5 };
    const vect ints4{ 4,1,2,3,4,5 };
    const vect def_ints{ 6, 7 };
    test_coalesce(def_ints, ints1, ints2, ints3, ints4);
}

TEST(Testcoalesce, StringTest)
{
    std::string s1 = "a" ;
    std::string s2 = "b";
    std::string s3 = "c";
    std::string s4 = "d";
    std::string def = "z";
    test_coalesce(def, s1, s2, s3, s4);
}

TEST(Testcoalesce, ConstStringTest)
{
    const std::string s1 = "a";
    const std::string s2 = "b";
    const std::string s3 = "c";
    const std::string s4 = "d";
    const std::string def = "z";
    test_coalesce(def, s1, s2, s3, s4);
}

TEST(Testcoalesce, DerivedObj)
{
    A a{ 1 };
    B b1{ 2,3 };
    C c2{ 4,5,6 };
    D d3{ 6,7,8,9 };
    B b4{ 8,9 };
    test_coalesce(a, b1, c2, d3, b4);
}