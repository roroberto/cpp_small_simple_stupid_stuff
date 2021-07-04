# cpp_small_simple_stupid_stuff

In this repository there are C++ simple libraries. They are supposed to be very small, max a few hundreds lines.
They must cover a specific need of C++ developers.

If you like something just use it! If it does not cover completly what you need let me know, or just branch it and adjust it. 

## value_or
The fist library is value_or, just one function. It is similar to the std::optional.value_or: it returns the the first paramter not null, if it does not find it, then it returns a default value. 
It is very similar to a coalesce SQL function.

```c++
value_or(T&& default_value, Args&&... to_test_v).
```
 It looks for a not null value in to_test_v. If it does not find it, then value_or returns default_value. 

Here an example
```C++
#include <optional>
#include <vector>
#include <iostream>
#include "value_or.h"


int main()
{
    int i = 5;
    int* pi = &i;
    std::optional<int> o = 12;
    std::unique_ptr<int> up = std::make_unique<int>(3);
    std::shared_ptr<int> sp = std::make_shared<int>(4);
    std::weak_ptr<int> wp = sp;

    int r = s4::value_or(10, nullptr, pi, o, up, sp, wp);
    std::cout << r << std::endl;  // prints 5 because the 1st pointer not null is pi, 10 is the default value 

    std::vector<int> ints{ 0,1,2,3,4,5 };
    std::unique_ptr<std::vector<int>> upv = std::make_unique<std::vector<int>>();
    std::unique_ptr<std::vector<int>> upvn;
    size_t r2 = s4::value_or(ints, upvn).size();
    std::cout << r2 << std::endl; // prints 6, the size of ints, the default value because upwn is null

    size_t r3 = s4::value_or(ints, upvn, upv).size();
    std::cout << r3 << std::endl; // prints 0, the size of upv
}
```
