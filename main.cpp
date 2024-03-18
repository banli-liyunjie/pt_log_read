#include "count_board.h"
#include "regex_info.h"
#include "result_print/print.h"
#include <fstream>
#include <iostream>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <windows.h>

using namespace std;
using banli::level;
using banli::print;

void get_log_error(const char* exe_path, const char* log_path);
type_board parse_single_file(print& outf, const string& f_path, string& board_sn);

unordered_set<string> board_uset;
count_board cb;

bool display_ok = false;
bool display_temp = false;
bool display_version = false;
bool out_in_std = false;
string self_file_name;

int main(int argc, char *argv[]){
    char log_dir[MAX_PATH] = { 0 };
    for (int i = 1; i < argc; ++i) {
        if ((string(argv[i]) == "-o" || string(argv[i]) == "--ok")) {
            cout << "should display the ok board" << endl;
            display_ok = true;
        } else if ((string(argv[i]) == "-t" || string(argv[i]) == "--temp")) {
            cout << "should display the env temperature" << endl;
            display_temp = true;
        } else if ((string(argv[i]) == "-v" || string(argv[i]) == "--version")) {
            cout << "should display the soft version" << endl;
            display_version = true;
        } else if ((string(argv[i]) == "-d" || string(argv[i]) == "--directory")) {
            strncpy(log_dir, argv[++i], MAX_PATH);
        } else if ((string(argv[i]) == "-p" || string(argv[i]) == "--print")) {
            cout << "should print in std" << endl;
            out_in_std = true;
        }
    }

    std::vector<char> buffer(MAX_PATH);

    DWORD copied = GetModuleFileNameA(NULL, &buffer[0], static_cast<DWORD>(buffer.size()));

    while (copied == buffer.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        // 缓冲区太小，需要扩展
        buffer.resize(buffer.size() * 2);
        copied = GetModuleFileNameA(NULL, &buffer[0], static_cast<DWORD>(buffer.size()));
    }

    if (copied == 0) {
        std::cerr << "Error getting module file name, last error: " << GetLastError() << std::endl;
        return 1;
    }

    // 确保字符串是以空字符结尾的
    buffer[copied] = '\0';

    char* lastBackslash = strrchr(&buffer[0], '\\');
    if (lastBackslash != NULL) {
        self_file_name = string(lastBackslash + 1);
        *lastBackslash = '\0'; // 将最后一个反斜杠替换为空字符，从而去掉文件名
    }

    if (0 == strlen(log_dir))
        strncpy(log_dir, &buffer[0], MAX_PATH);
    cout << "log directory: " << log_dir << endl;
    cout << "Executable directory: " << &buffer[0] << endl;
    cout << "Executable file: " << self_file_name << endl;

    board_uset.clear();

    get_log_error(&buffer[0], log_dir);

    system("pause");

    return 0;
}

void get_log_error(const char* exe_path, const char* log_path)
{
    print outf(out_in_std);
    string path(exe_path);
    path = path + "/result.txt";
    outf.open(path, ios::out | ios::trunc);
    //********************************************

    char find_path[MAX_PATH];
    WIN32_FIND_DATA find_file_data;

    strcpy(find_path, log_path);
    strcat(find_path, "\\*.*");

    HANDLE h_find = FindFirstFile(find_path, &find_file_data);
    if(INVALID_HANDLE_VALUE == h_find){
        outf.display(level::debug, "error find, please check if the file path contains any spaces\n");
        return;
    }

    type_board tb;
    string board_sn;
    unordered_map<string, string> unknow_board;

    while(true){
        if(find_file_data.dwFileAttributes & (~FILE_ATTRIBUTE_DIRECTORY)){
            string file_name(find_file_data.cFileName);
            if(file_name != "result.txt" && file_name != self_file_name){
                outf.display(level::debug, "%s\n", file_name.c_str());
                string f_path(log_path);
                f_path += "/" + file_name;
                tb = parse_single_file(outf, f_path, board_sn);
                if(type_board::repeat != tb){
                    cb[tb]++;
                    if(type_board::unknown == tb)
                        unknow_board.emplace(pair<string, string>(file_name, board_sn == "" ? "unknown" : board_sn));
                }
            }
        }
        if(!FindNextFile(h_find, &find_file_data)) break;
    }

    outf << "\ntotal : " << cb.total << "  ||  sweep ok : " << cb[sweep_ok];
    outf << "\nbad_420 : " << cb[bad_420] << "  ||  bad asic : " << cb[bad_asic];
    outf << "\nout temp : " << cb[out_temp] << "  ||  out vol : " << cb[out_vol];
    outf << "\nfind asic err : " << cb[find_asic_err] << "  ||  sensor err : " << cb[sensor_err];
    outf << "\nenv temp too low : " << cb[env_low] << "  ||  env temp too high : " << cb[env_high];
    outf << "\nunknow board info : " << cb[unknown] << "\n";
    for(auto uk : unknow_board){
        outf << uk.first << " board_sn " << uk.second << "\n";
    }
}

#define GET_DIS_STR [&](void* ptr) {                                          \
    string str = "";                                                          \
    char c_str[100];                                                          \
    sprintf(c_str, "%s %s %s \0", board_sn.c_str(), ft.c_str(), bin.c_str()); \
    str += string(c_str);                                                     \
    if (display_version) {                                                    \
        str += "[" + soft_version.substr(0, 6);                               \
        str += " - " + commit_time + "] ";                                    \
    }                                                                         \
    if (display_temp) {                                                       \
        sprintf(c_str, "[env temp : %d] \0", env_temp);                       \
        str += string(c_str);                                                 \
    }                                                                         \
    str += get_board_type_str(tb);                                            \
    switch (tb) {                                                             \
    case type_board::bad_420:                                                 \
    case type_board::bad_asic:                                                \
        str += " [bad asic : ";                                               \
        for (auto asic : asic_bad)                                            \
            str += to_string(asic) + " ";                                     \
        str += "]";                                                           \
        break;                                                                \
    case type_board::out_temp:                                                \
    case type_board::out_vol:                                                 \
        if (abnormal_asic.size() > 0) {                                       \
            str += " [abnormal asic : ";                                      \
            for (auto asic : abnormal_asic)                                   \
                str += to_string(asic) + " ";                                 \
            str += "]";                                                       \
        }                                                                     \
        if (asic_null.size() > 0) {                                           \
            str += " [asic null : ";                                          \
            for (auto asic : asic_null)                                       \
                str += to_string(asic) + " ";                                 \
            str += "]";                                                       \
        }                                                                     \
        break;                                                                \
    case type_board::env_low:                                                 \
    case type_board::env_high:                                                \
        str += " [current env temp is : " + to_string(env_temp) + "]";        \
        break;                                                                \
    case type_board::sweep_ok:                                                \
        str += " [level :" + to_string(level) + "]";                          \
        break;                                                                \
    default:                                                                  \
        break;                                                                \
    }                                                                         \
    str += "\n";                                                              \
    return str;                                                               \
}

type_board parse_single_file(print& outf, const string& f_path, string& board_sn)
{
    ifstream file;
    file.open(f_path, ios::in);
    char buf[2048];
    cmatch m;

    set<int> abnormal_asic;
    set<int> asic_null;
    set<int> asic_bad;

    type_board tb = type_board::unknown;

    string ft = "", bin = "";
    string soft_version = "", commit_time = "";
    int env_temp = -9999;
    int level = 0;

    board_sn = "";
    while (file.getline(buf,size_t(buf))){
        REG(CHECK_LINE_DATA);
    }

    bool is_display = !(type_board::unknown == tb || type_board::repeat == tb || (type_board::sweep_ok == tb && false == display_ok));
    if(is_display){
        auto get_dis_str = GET_DIS_STR;
        outf << get_dis_str(nullptr);
    }

    file.close();
    return tb;
}