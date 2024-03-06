#include <regex>
#include <string>

#define CALL_BOARD_SN [&](void* ptr){\
            if(board_sn != ""){\
                if(board_sn == m[1])\
                    return true;\
                else{\
                    cout << "this log is abnormal"<< endl;\
                    cb.total--;\
                    tb = type_board::unknown;\
                    return false;\
                }\
            }\
            board_sn = m[1];\
            if(board_uset.find(board_sn) != board_uset.end()){\
                cout << "repeated board : " << board_sn << endl;\
                tb = type_board::repeat;\
                return false;\
            }\
            cb.total++;\
            board_uset.emplace(board_sn);\
            return true;\
}
#define CALL_FT_VERSION [&](void* ptr){\
            ft = m[1];\
            return true;\
}
#define CALL_CHIP_BIN [&](void* ptr){\
            bin = "BIN" + std::string(m[1]);\
            return true;\
}
#define CALL_ABNORMAL [&](void* ptr){\
            int asic = stoi(m[1]);\
            abnormal_asic.emplace(asic);\
            return true;\
}
#define CALL_ASIC_NULL [&](void* ptr){\
            int asic = stoi(m[1]);\
            asic_null.emplace(asic);\
            return true;\
}
#define CALL_TEMP [&](void* ptr){\
            env_temp = stoi(m[1]);\
            return true;\
}
#define CALL_TEMP_HIGH [&](void* ptr){\
            if(board_sn != "" && tb == type_board::unknown){\
                tb = type_board::env_high;\
            }\
            return true;\
}
#define CALL_TEMP_LOW [&](void* ptr){\
            if(board_sn != "" && tb == type_board::unknown){\
                tb = type_board::env_low;\
            }\
            return true;\
}
#define CALL_OUT_TEMP [&](void* ptr){\
            if(board_sn != "" && tb == type_board::unknown){\
                tb = type_board::out_temp;\
            }\
            return true;\
}
#define CALL_OUT_VOL [&](void* ptr){\
            if(board_sn != "" && tb == type_board::unknown){\
                tb = type_board::out_vol;\
            }\
            return true;\
}
#define CALL_SENSOR_ERR [&](void* ptr){\
            if(board_sn != "" && tb == type_board::unknown){\
                tb = type_board::sensor_err;\
            }\
            return true;\
}
#define CALL_FIND_ASIC [&](void* ptr){\
            if(board_sn != "" && tb == type_board::unknown){\
                tb = type_board::find_asic_err;\
            }\
            return true;\
}
#define CALL_BAD_420 [&](void* ptr){\
            if(board_sn != "" && tb == type_board::unknown){\
                tb = type_board::bad_420;\
            }\
            return true;\
}
#define CALL_BAD_LIST [&](void* ptr){\
            int asic = stoi(m[1]);\
            asic_bad.emplace(asic);\
            return true;\
}
#define CALL_SWEEP_OK [&](void* ptr){\
            if(board_sn != "" && tb == type_board::unknown){\
                tb = type_board::sweep_ok;\
                level = stoi(m[1]);\
            }\
            return true;\
}
#define CALL_TEST_OVER [&](void* ptr){\
            if(board_sn != "" && tb == type_board::unknown){\
                if(asic_bad.size() > 0){\
                    tb = type_board::bad_asic;\
                }\
            }\
            return false;\
}

#define REG(FUNC)\
    FUNC(regex_search, board_sn_regex,         CALL_BOARD_SN)\
    FUNC(regex_match,  ft_version_regex,       CALL_FT_VERSION)\
    FUNC(regex_match,  chip_bin_regex,         CALL_CHIP_BIN)\
    FUNC(regex_search, abnormal_cooling_regex, CALL_ABNORMAL)\
    FUNC(regex_search, asic_null_regex,        CALL_ASIC_NULL)\
    FUNC(regex_match,  env_temp_regex,         CALL_TEMP)\
    FUNC(regex_match,  env_high_regex,         CALL_TEMP_HIGH)\
    FUNC(regex_match,  env_low_regex,          CALL_TEMP_LOW)\
    FUNC(regex_match,  out_temp_regex,         CALL_OUT_TEMP)\
    FUNC(regex_match,  out_vol_regex,          CALL_OUT_VOL)\
    FUNC(regex_match,  sensor_err_regex,       CALL_SENSOR_ERR)\
    FUNC(regex_search, find_asic_regex,        CALL_FIND_ASIC)\
    FUNC(regex_match,  bad_420_regex,          CALL_BAD_420)\
    FUNC(regex_search, bad_asic_list_regex,    CALL_BAD_LIST)\
    FUNC(regex_search, sweep_ok_regex,         CALL_SWEEP_OK)\
    FUNC(regex_match,  test_over_regex,        CALL_TEST_OVER)

#define CHECK_LINE_DATA(regex_fuc, regex_str, func)\
            if(regex_fuc(buf, m, regex_str)){\
                auto f = func;\
                if(f(nullptr))\
                    continue;\
                else\
                    break;\
            }

const std::regex board_sn_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]board_sn = (.+)");
const std::regex abnormal_cooling_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]abnormal cooling on asic\\[(\\d+)]");
const std::regex asic_null_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]asic \\[(\\d+)]:NULL");
const std::regex out_temp_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]exit info:asic temp outof range");
const std::regex out_vol_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]exit info:asic vol outof range");
const std::regex sensor_err_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]exit info:sensor err");
const std::regex find_asic_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]L1: Find asics");
const std::regex bad_420_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]level_high: \\d, level_low: 3, matched_freq: \\d+, freq_min: \\d+");
const std::regex bad_asic_list_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]bad asic list\\[\\d+]:(\\d+)");
const std::regex sweep_ok_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]L2: Level:  (\\d+)");
const std::regex test_over_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]TEST OVER...");
const std::regex env_low_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]env temp (\\d+) is too low, pattern text exit");
const std::regex env_high_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]env temp (\\d+) is too high, pattern text exit");
const std::regex ft_version_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]ft_version = (.+)");
const std::regex chip_bin_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]chip_bin = (\\d+)");
const std::regex env_temp_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]env temp: (\\d+)");
