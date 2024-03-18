#pragma once
#include <fstream>
#include <ios>
#include <iostream>

#define MAX_LINE_SIZE 2048

namespace banli {

enum level {
    debug = 0,
    result = 1,
};

class print {
public:
    print(bool out)
        : out_in_std(out)
    {
    }
    ~print();
    void open(const std::string& __s, std::ios_base::openmode __mode);
    print& display(uint32_t level, const char* str, ...);
    template <typename _T>
    print& operator<<(const _T& _type) // default level : result
    {
        if (out_in_std)
            std::cout << _type;
        else
            outf << _type;
        return *this;
    }
    print& operator<<(print& (*_pfn)(print&));

private:
    std::ofstream outf;
    bool out_in_std = false;
};
inline print& endl(print& _print)
{
    return _print.operator<<("\n");
}

}