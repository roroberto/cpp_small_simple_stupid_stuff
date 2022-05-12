#include <optional>
#include <vector>
#include <iostream>
#include <vector>
#include <functional>
#include <numeric>
#include <ranges>
#include "coalesce.h"



static int st = 123;

int calc_default_value() noexcept
{
    return 14;
}

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

    const int r1 = s4::coalesce(i,  pi, o, up, sp, wp);
    std::cout << r1 << std::endl;  // prints 5 because the 1st pointer not null is pi, the first parameter is the default value 
    
    int r2 = s4::coalesce(calc_default_value(), nullptr);
    std::cout << r2 << std::endl;  // prints 14 the calculated default value 

    std::vector<int> ints{ 0,1,2,3,4,5 };
    std::unique_ptr<std::vector<int>> upv = std::make_unique<std::vector<int>>();
    std::unique_ptr<std::vector<int>> upvn;
    const size_t r3 = s4::coalesce(ints, upvn).size();
    std::cout << r3 << std::endl; // prints 6, the size of ints, the default value because upwn is null

    const std::vector<int>& v = s4::coalesce(ints, upvn);
    std::cout << v.size() << std::endl;

    const size_t r4 = s4::coalesce(ints, upvn, upv).size(); 
    std::cout << r4 << std::endl; // prints 0, the size of upv

    typedef const std::vector<int> cintvect;
    cintvect intsc{ 0,1,2,3,4,5 };
    cintvect intsc2{ 1,2,3,4 };
    std::unique_ptr<cintvect> upvc = std::make_unique<cintvect>(intsc2);
    std::unique_ptr<cintvect> upvnc;
    const size_t r5 = s4::coalesce(intsc, upvnc, upvc).size(); 
    std::cout << r5 << std::endl;  // prints 4 the size of upvc
    std::cout << s4::coalesce(intsc, upvnc, upvc)[2] << std::endl; // it works not, but without checking the size of the returned vector not a great idea
    
    const int r6 = s4::coalesce(10, calc_value_null, calc_value);
    std::cout << r6 << std::endl; // prints 123, the value returned by calc_value

    auto l = []() noexcept -> int*
    {
        return nullptr;
    };

    const int r7 = s4::coalesce(10, l, calc_value);
    std::cout << r7 << std::endl;

    typedef std::function<int* ()> fu;
    fu f = l;
   
    const int r8 = s4::coalesce(2, f);
    std::cout << r8 << std::endl;

    fu fn;
    const int r8b = s4::coalesce(3, fn);
    std::cout << r8b << std::endl;

    const int r9 = s4::coalesce(calc_default_value, pi);
    std::cout << r9 << std::endl;

    std::vector<record> s{ 
        {10, {}}, 
        {{}, 20},
        {{}, {}}
    };
    
    int r10 = std::accumulate(s.begin(), s.end(), 0,
        [](int i, const record& r) { return i + s4::coalesce(0, r.v1, r.v2);  });
    std::cout << r10 << std::endl;

    for (int i : s | std::views::transform([](const record& r) { return s4::coalesce(0, r.v1, r.v2); }))
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;

    return 0;
}

