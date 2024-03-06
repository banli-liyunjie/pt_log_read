#include <vector>
#include <string>

#define TYPE_REG(FUNC)\
            FUNC(unknown,  0, "unknown")\
            FUNC(sweep_ok, 1, "sweep ok")\
            FUNC(bad_420,  2, "bad 420")\
            FUNC(bad_asic, 3, "bad asic")\
            FUNC(out_temp, 4, "out temp")\
            FUNC(out_vol,  5, "out vol")\
            FUNC(find_asic_err, 6, "find asic err")\
            FUNC(sensor_err, 7, "sensor err")\
            FUNC(env_low,  8, "env too low")\
            FUNC(env_high, 9, "env too high")


#define TYPE_ENUM(name, index, args...)\
        name = index,

#define COUNT_STRUCT(name, args...)\
        int name = 0;

#define TYPE_STRING(name, _1, str)\
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

#undef TYPE_STRING
#undef COUNT_STRUCT
#undef TYPE_ENUM
#undef TYPE_REG