#include "print.h"
#include <cstdarg>
#include <cstdio>

using namespace std;
namespace banli {

print::~print()
{
    outf.close();
}

void print::open(const std::string& __s, std::ios_base::openmode __mode = std::ios_base::out)
{
    outf.open(__s, __mode);
}

print& print::display(uint32_t level, const char* str, ...)
{
    char buf[MAX_LINE_SIZE];
    va_list args;
    va_start(args, str);
    vsnprintf(buf, MAX_LINE_SIZE, str, args);
    va_end(args);

    if ((level == debug && !out_in_std) || (level == result && out_in_std)) {
        cout << buf;
    } else if (level == result) {
        outf << buf;
    }

    return *this;
}

print& print::operator<<(print& (*_pfn)(print&))
{
    return _pfn(*this);
}

}