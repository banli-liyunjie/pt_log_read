
#define TYPE_REG(FUNC)\
            FUNC(unknown,  0)\
            FUNC(sweep_ok, 1)\
            FUNC(bad_420,  2)\
            FUNC(bad_asic, 3)\
            FUNC(out_temp, 4)\
            FUNC(out_vol,  5)\
            FUNC(find_asic_err, 6)\
            FUNC(sensor_err, 7)\
            FUNC(env_low,  8)\
            FUNC(env_high, 9)

#define TYPE_ENUM(name, index)\
        name = index,

#define COUNT_STRUCT(name, args...)\
        int name = 0;

enum type_board{
    TYPE_REG(TYPE_ENUM)
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

#undef COUNT_STRUCT
#undef TYPE_ENUM
#undef TYPE_REG