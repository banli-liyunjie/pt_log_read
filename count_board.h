#include <vector>
#include <string>

#define TYPE_REG(FUNC)                                          \
    FUNC(unknown, 0, "unknown", "unknown board : ")             \
    FUNC(sweep_ok, 1, "sweep ok", "sweep ok : ")                \
    FUNC(bad_420, 2, "bad 420", "bad_420 : ")                   \
    FUNC(bad_asic, 3, "bad asic", "bad asic : ")                \
    FUNC(out_temp, 4, "out temp", "out temp : ")                \
    FUNC(out_vol, 5, "out vol", "out vol : ")                   \
    FUNC(find_asic_err, 6, "find asic err", "find asic err : ") \
    FUNC(sensor_err, 7, "sensor err", "sensor err : ")          \
    FUNC(env_low, 8, "env too low", "env temp too low : ")      \
    FUNC(env_high, 9, "env too high", "env temp too high : ")

#define TYPE_ENUM(name, index, args...)\
        name = index,

#define COUNT_STRUCT(name, args...)\
        int name = 0;

#define TYPE_STRING(name, _1, str, args...) \
    str,

enum type_board{
    TYPE_REG(TYPE_ENUM)
    repeat = 9999
};

class count_board{
private:
    struct count_t{
        TYPE_REG(COUNT_STRUCT)
    } array;
public:
    int& operator [] (unsigned int index){
        return *(((int*)&array) + index);
    }
    
    int total = 0;
};

const std::vector<std::string> type_str = {
    TYPE_REG(TYPE_STRING)
};

inline std::string get_board_type_str(type_board tb){
    int max_len = 0;
    for(auto& s : type_str){
        max_len = max_len < s.size() ? s.size() : max_len;
    }
    std::string str = std::string(max_len, ' ');
    size_t st = (max_len - type_str[tb].size()) / 2;
    str.replace(st, type_str[tb].size(), type_str[tb]);
    return "(" + str + ")";
}

#define _GET_PARA(TYPE, _1, _2, INFO) TYPE, INFO,
#define GET_PARA TYPE_REG(_GET_PARA) _TAIL

#define PRINT_PARA_2(TYPE, INFO) outf << INFO << cb[type_board::TYPE] << "\n"
#define PRINT_PARA_4(TYPE_1, INFO_1, TYPE_2, INFO_2) outf << INFO_1 << cb[type_board::TYPE_1] << "  ||  " << INFO_2 << cb[type_board::TYPE_2] << "\n"
#define FUNC_3(_1, _2, args...) PRINT_PARA_2(_1, _2)
#define FUNC_5(_1, _2, _3, _4, args...) PRINT_PARA_4(_1, _2, _3, _4)
#define FX(_1, _2, _3, _4, args...) \
    PRINT_PARA_4(_1, _2, _3, _4);   \
    ITERATION_FUNC(_PRINT_MID, args)

//* call different functions based on the number of arguments
// if it cannot be expanded correctly, perhaps you need to extend this macro
#define _PRINT_MID_N(F32, F31, F30, F29, F28, F27, F26, F25, F24, F23, F22, F21, F20, F19, F18, F17, F16, F15, F14, F13, F12, F11, F10, F9, F8, F7, F6, F5, F4, F3, F2, F1, FUNC, args...) FUNC
#define PRINT_MID_N(args...) _PRINT_MID_N(_F0, ##args, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FX, FUNC_5, FUNC_4, FUNC_3, FUNC_2, FUNC_1, FUNC_0)

#define PRINT_MID(args...) PRINT_MID_N(args)(args)
#define _PRINT_MID() PRINT_MID

//* macro function iteration: delayed expansion
#define EMPTY()
#define FUNC_NAME(NAME) NAME EMPTY()
#define _ITERATION_FUNC(NAME, args...) FUNC_NAME(NAME)()(args)
#define ITERATION_FUNC(NAME, args...) _ITERATION_FUNC(NAME, args)

//* macro function iteration: multi-level expansion
// if it cannot be expanded correctly, perhaps you need to extend this macro
#define Expand(args...) _Expand1(_Expand1(_Expand1(args)))
#define _Expand1(args...) _Expand2(_Expand2(_Expand2(args)))
#define _Expand2(args...) _Expand3(_Expand3(_Expand3(args)))
#define _Expand3(args...) _Expand4(_Expand4(_Expand4(args)))
#define _Expand4(args...) _Expand5(_Expand5(_Expand5(args)))
#define _Expand5(args...) args

#define PRINT_HEAD(TYPE, INFO) outf << "\ntotal : " << cb.total << "  ||  " << INFO << cb[type_board::TYPE] << "\n"
#define PRINT_TAIL(TYPE, INFO) outf << INFO << cb[type_board::TYPE] << "\n"

#define _PRINT_RESULT(TYPE_1, INFO_1, TYPE_2, INFO_2, args...) \
    PRINT_HEAD(TYPE_2, INFO_2);                                \
    Expand(PRINT_MID(args));                                   \
    PRINT_TAIL(TYPE_1, INFO_1)
#define PRINT_RESULT(args...) _PRINT_RESULT(args)

#undef TYPE_STRING
#undef COUNT_STRUCT
#undef TYPE_ENUM