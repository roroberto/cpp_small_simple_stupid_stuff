#include <optional>
#include <vector>
#include <iostream>
#include <vector>
#include <functional>
#include <numeric>
#include <ranges>
#include <string_view>
#include "coalesce.h"





template <typename Value, typename Proj>
class test_project
{
public:
    test_project(Value v, Proj proj) : _v{ v }, _proj{ proj } {}

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


struct record
{
    std::optional<int> v1;
    std::optional<int> v2;
};


int main()
{
    int i = 5;
    int* pi = &i;
    std::optional<int> o = 12;
    std::unique_ptr<int> up = std::make_unique<int>(3);
    std::shared_ptr<int> sp = std::make_shared<int>(4);
    std::weak_ptr<int> wp = sp;

    // the 1st parameter is the default value, the others are different type of pointers or std::optional
    const int r1 = s4::coalesce(i,  pi, o, up, sp, wp);
    std::cout << r1 << std::endl;  // prints 5 because the 1st pointer not null is pi
    
    // the default value can be a pointer to a function that will be called only if no null pointer has been found
    int r2 = s4::coalesce(calc_default_value, nullptr);
    std::cout << r2 << std::endl;  // prints 14 the calculated default value 
    
    int r2b = s4::coalesce(calc_default_value, pi);
    std::cout << r2b << std::endl; // prints 5, the value pointed by pi 

    
    // other examples with more complex types
    std::vector<int> ints{ 0,1,2,3,4,5 };
    std::unique_ptr<std::vector<int>> upvn;
    const size_t r3 = s4::coalesce(ints, upvn).size();
    std::cout << r3 << std::endl; // prints 6, the size of ints, the default value because upwn is null

    const std::vector<int>& v = s4::coalesce(ints, upvn);
    std::cout << v.size() << std::endl; // prints 6, the size of ints, the default value because upwn is null

    std::unique_ptr<std::vector<int>> upv = std::make_unique<std::vector<int>>();
    const size_t r4 = s4::coalesce(ints, upvn, upv).size(); 
    std::cout << r4 << std::endl; // prints 0, the size of upv


    // not only the default value can be a pointer to a function, also any other parameter. 
    // The functions will be called one after the other until an not null value is found.       
    const int r5 = s4::coalesce(10, calc_value_null, calc_value);
    std::cout << r5 << std::endl; // prints 123, the value returned by calc_value


    // also lambda can be passed
    auto l_null = []() noexcept -> int*
    {
        return nullptr;
    };

    
    const int r6 = s4::coalesce(10, l_null, calc_value);
    std::cout << r6 << std::endl;  // prints 123, l_null returns nullptr, therefore calc_value is called, that retruns 123

   
    // also std::function can be passed, in this case it will test also that the object of type std::function points to a function
    using fu = std::function<int* ()>;
    fu f = l_null;
       
    const int r7 = s4::coalesce(2, f);
    std::cout << r7 << std::endl; // it prints 2, the default value because l_null, the function pointed by f, returns nullptr

    fu fn; 
    const int r7b = s4::coalesce(3, fn); 
    std::cout << r7b << std::endl; // it prints 2, the default value because fn points to no function

        
    // let's say that we have a data feed that we want to process, and for each record we must use the 1st value not null, if there is
    std::vector<record> s1
    { 
        {10, {}}, 
        {{}, 20},
        {{}, {}}
    };
    
    int r8 = std::accumulate(s1.begin(), s1.end(), 0,
        [](int i, const record& r) { return i + s4::coalesce(0, r.v1, r.v2);  });
    std::cout << r8 << std::endl; // prints 30 = 10 + 20 + 0

    for (int i : s1 | std::views::transform([](const record& r) { return s4::coalesce(0, r.v1, r.v2); }))
    {
        std::cout << i << " ";  // prints first 10, then 20 and at the end 0
    }
    std::cout << std::endl;


    // another data feed, the 1st field of the record is in grams and the second in Kg
    std::vector<record> s2
    {
        {1000, {}},
        {{},    2},
        {{},   {}}
    };


    int r9 = std::accumulate(s2.begin(), s2.end(), 0,
        [](int i, const record& r) 
        { return i + 
            s4::coalesce(0, r.v1, 
                test_project(r.v2, [](int i) {return i * 1000; })); // with the help of test_project and a lambda 
                                                                    // we convert the second field in grams
        }
    );
    std::cout << r9 << std::endl;  // prints 3000 = 1000 + 2 * 1000

    struct_i si;
    struct_i* psi = &si;
    int r10 = s4::coalesce(10, test_project(psi, &struct_i::getI));
    std::cout << r10 << std::endl;  // prints 15 the value of si.i
  
     
    return 0;
}

