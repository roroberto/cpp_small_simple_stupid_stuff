#include <optional>
#include <vector>
#include <iostream>
#include <vector>
#include <functional>
#include <numeric>
#include <ranges>
#include <string_view>
#include "value_or.h"





template <typename Value, typename Proj>
class test_project
{
public:
    test_project(Value v, Proj proj) 
        : _v{ v }, _proj{ proj } {}

    bool operator! () const noexcept
    {
        return !_v;
    }

    auto operator *()
    {
        return std::invoke(_proj, *_v);
    }

private:
    Value _v;
    Proj _proj;
};


class test_and_convert
{
public: 
    test_and_convert(const std::optional<int> &v) : _v{ v } {}
    constexpr explicit operator bool() const noexcept { return _v.has_value(); }
    constexpr int operator*() const noexcept { return *_v * 1000;  }
private:
    const std::optional<int>& _v;
};

class test_string
{
public:
    test_string(std::string_view v) : _s{ v } {}

    constexpr operator bool () const noexcept
    {
        return !_s.empty();
    }

    constexpr const std::string_view operator *()
    {
        return _s;
    }

private:
    std::string_view _s;
};



struct struct_i
{
    int i = 15;
    int getI() { return i; }
};

int calc_default_value() noexcept
{
    return 14;
}

static int st = 123;

int* calc_value() noexcept
{
    return &st;
}

int* calc_value_null() noexcept
{
    return nullptr;
}

int ref_f(int& i) noexcept
{
    return i;
}


struct record
{
    std::optional<int> v1;
    std::optional<int> v2;
};


int main()
{
    const int i = 5;
    const int* pi = &i;
    std::unique_ptr<int> up = std::make_unique<int>(3);

    // the below lines are equivalent
    int r0a = s4::value_or(i, pi, up);
    int r0b = !pi ? (!up ? i : *up) : *pi;  // or r0b = pi ? *pi : (up ? *up : i), but value_or uses the operator! 
    std::cout << r0a << "=" << r0b << std::endl;

    std::optional<int> o = 12;
    std::shared_ptr<int> sp = std::make_shared<int>(4);
    std::weak_ptr<int> wp = sp;

    // the 1st parameter is the default value, the others are different type of pointers or std::optional
    const int r1 = s4::value_or(i,  pi, o, up, sp, wp);
    std::cout << r1 << std::endl;  // prints 5 because the 1st pointer not null is pi
    
    // the default value can be an invokable that will be called only if no null pointer has been found
    const int r2 = s4::value_or(calc_default_value, nullptr);
    std::cout << r2 << std::endl;  // prints 14 the calculated default value 
    
    const int r2b = s4::value_or(calc_default_value, pi);
    std::cout << r2b << std::endl; // prints 5, the value pointed by pi 

    // it is possible to specify the type of the return value, of the default value and of all the arguments
    const int r2c = s4::value_or<int, const int&, const int*&, std::optional<int>&>(i, pi, o);
    std::cout << r2c << std::endl; // prints 5, the value pointed by pi 

    const int r2d = s4::value_or( []() { return 14; }, nullptr );
    std::cout << r2d << std::endl;  // prints 14 the calculated default value 

    // the below lines are equivalent 
    const int r2e = s4::value_or(2, o);
    const int r2f = o.value_or(2);
    std::cout << r2e << "=" << r2f << std::endl;


    // other examples with more complex types
    std::vector<int> ints{ 0,1,2,3,4,5 };
    std::unique_ptr<std::vector<int>> upvn;
    const size_t r3 = s4::value_or(ints, upvn).size();
    std::cout << r3 << std::endl; // prints 6, the size of ints, the default value because upwn is null

    const std::vector<int>& v = s4::value_or(ints, upvn);
    std::cout << v.size() << std::endl; // prints 6, the size of ints, the default value because upwn is null

    std::unique_ptr<std::vector<int>> upv = std::make_unique<std::vector<int>>();
    const size_t r4 = s4::value_or(ints, upvn, upv).size(); 
    std::cout << r4 << std::endl; // prints 0, the size of upv


    // not only the default value can be a pointer to a function, also any other parameter. 
    // The functions will be called one after the other until an not null value is found.       
    const int r5 = s4::value_or(10, calc_value_null, calc_value, pi);
    std::cout << r5 << std::endl; // prints 123, the value returned by calc_value


    // also lambda can be passed
    auto l_null = []() noexcept -> int*
    {
        return nullptr;
    };

    
    const int r6 = s4::value_or(10, l_null, calc_value);
    std::cout << r6 << std::endl;  // prints 123, l_null returns nullptr, therefore calc_value is called, that returns 123

   
    // also std::function can be passed, in this case it will test also that the object of type std::function points to a function
    using fi = int* ();
    using fu = std::function<fi>;
    fu f1{};

    const int r7 = s4::value_or(2, f1);
    std::cout << r7 << std::endl; // it prints 2, the default value because l_null, the function pointed by f, returns nullptr

    fu fn; 
    const int r7b = s4::value_or(3, fn); 
    std::cout << r7b << std::endl; // it prints 3, the default value because fn points to no function

    fu fln = l_null;
    const int r7c = s4::value_or(4, fln);
    std::cout << r7c << std::endl; // it prints 4, the default value because fln points to l_null that return nullptr

    int* (*ptr_fun)() noexcept = &calc_value;
    const int r7d = s4::value_or(5, ptr_fun);
    std::cout << r7d << std::endl; // it prints 123, the value returned by calc_value

    int* (*ptr_funn)() noexcept = nullptr;
    const int r7e = s4::value_or(6, ptr_funn);
    std::cout << r7e << std::endl; // it prints 6, the default value


    using ffu = std::function<fu* ()>;
    ffu ffu1{};
    fu fdefault{ &calc_value };
    //fu r7d = s4::value_or(fdefault, ffu1);
      

    // let's say that we have a data feed that we want to process, and for each record we must use the 1st value not null, if there is
    std::vector<record> s1
    { 
        {10, {}}, 
        {{}, 20},
        {{}, {}}
    };
    
    const int r8 = std::accumulate(s1.begin(), s1.end(), 0,
        [](int i, const record& r) { return i + s4::value_or(0, r.v1, r.v2);  });
    std::cout << r8 << std::endl; // prints 30 = 10 + 20 + 0

    for (int n : s1 | std::views::transform([](const record& r) { return s4::value_or(0, r.v1, r.v2); }))
    {
        std::cout << n << " ";  // prints first 10, then 20 and at the end 0
    }
    std::cout << std::endl;


    // another data feed, the 1st field of the record is in grams and the second in Kg
    std::vector<record> s2
    {
        {1000, {}},
        {{},    2},
        {{},   {}}
    };

    const int r9 = std::accumulate(s2.begin(), s2.end(), 0,
        [](int tot, const record& r) 
        { return tot + 
            s4::value_or(0, r.v1, 
                test_project(r.v2, [](int v) {return v * 1000; })); // with the help of test_project and a lambda 
                                                                    // we convert the second field in grams
        }
    );
    std::cout << r9 << std::endl;  // prints 3000 = 1000 + 2 * 1000

    const int r9a = std::accumulate(s2.begin(), s2.end(), 0,
        [](int tot, const record& r)
        { return tot +
        s4::value_or(0, r.v1,
            test_and_convert(r.v2)); // with the help of test_and_convert it converts the second field in grams
        }
    );
    std::cout << r9a << std::endl;  // prints 3000 = 1000 + 2 * 1000


    struct_i si;
    struct_i* psi = &si;
    const int r10 = s4::value_or(10, test_project(psi, &struct_i::getI));
    std::cout << r10 << std::endl;  // prints 15 the value of si.i


    std::vector<std::vector<std::string>> s3 =
    {
        {"", "h"},
        {"i", ""}
    };

    for (std::vector<std::string> vs : s3)
    {
        std::cout << s4::value_or(std::string_view(""), test_string(vs[0]), test_string(vs[1]));
    }
    std::cout << std::endl;
  
   

    int d = 5;
    int i2 = 10;
    int* pi2 = &i2;
    const int r11 = ref_f(s4::value_or(d, pi2));  // in this case the return value of value_or is an lvalue, 
                                                  // therefore it can be based as reference to ref_f
    std::cout << r11 << std::endl;  // prints 10 the value of i2


    const int r11b = ref_f(s4::value_or(d, calc_value())); // the return value of value_or is an lvalue, 
                                                           // therefore it can be based as reference to ref_f
    std::cout << r11b << std::endl;  // prints 123 the value of pointed by the pointer returned by calc_value()

    //const int r11c = ref_f(s4::value_or(1, calc_value())); // ERROR: the return value of value_or is an rvalue
                                                             // because the default value is an rvalue

    //const int r11d = ref_f(s4::value_or(d, p12, wp)); // ERROR: the return value of value_or is an rvalue
                                                        // because one of the parameters is a weak_pointer

    int d1 = 1;
    int value = 2;
    int* to_test0 = &value;
    int* to_test1 = &value;
    int r = !to_test0 ? (!to_test1 ? d1 : *to_test1) : *to_test0;
    // ... eqivalent to ...
    int r12 = s4::value_or(d1, to_test0, to_test1);
    std::cout << r << " = " << r11 << std::endl;

    const int cd = 2;
    std::unique_ptr<const int> up2 = std::make_unique<const int>(2);
    std::shared_ptr<int> sp2 = std::make_shared<int>(3);
    int r13 = s4::value_or(cd, up2, sp2);
    std::cout << r13 << std::endl; // prints 2

    int def_from_user;
    std::cin >> def_from_user;
    std::optional<int> on;
    if (def_from_user > 1) on = o;
    std::shared_ptr<int> sn;
    if (def_from_user > 2) sn = sp;
    int r14 = s4::value_or(def_from_user, on, sn); // the compiler transform this just in a few tests, value_or has no overahead
    std::cout << r14 << std::endl; 

    return 0;
}

