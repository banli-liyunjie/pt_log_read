#include "count_board.h"
#include "regex_info.h"
#include "result_print/print.h"
#include <filesystem>
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

void get_log_error(const string& exe_path, const string& log_path);
type_board parse_single_file(print& outf, const string& f_path, string& board_sn);
void copy_fail_log(print& outf, const string& file_name, const string& f_path);

unordered_set<string> board_uset;
count_board cb;

bool display_ok = false;
bool display_temp = false;
bool display_version = false;
bool out_in_std = false;
string self_file_name;

int main(int argc, char *argv[]){
    string log_dir = "";
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
            log_dir = string(argv[++i]);
        } else if ((string(argv[i]) == "-p" || string(argv[i]) == "--print")) {
            cout << "should print in std" << endl;
            out_in_std = true;
        }
    }

    filesystem::path directory_path;
    try {
        wchar_t buffer[MAX_PATH] = { 0 };
        if (GetModuleFileNameW(NULL, buffer, MAX_PATH) == 0) {
            cerr << "Error getting module file name, last error: " << GetLastError() << endl;
            return 1;
        }
        filesystem::path full_path(buffer);
        self_file_name = full_path.filename().string();
        directory_path = full_path.parent_path();

        cout << "Executable name: " << full_path.filename() << endl;
        cout << "Directory path: " << directory_path << endl;

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    if (log_dir == "")
        log_dir = directory_path.string();
    cout << "log directory: " << log_dir << endl;

    board_uset.clear();

    get_log_error(directory_path.string(), log_dir);

    if (!out_in_std)
        system("pause");

    return 0;
}

void get_log_error(const string& exe_path, const string& log_path)
{
    print outf(out_in_std);
    string path(exe_path);
    path = path + "/result.txt";
    outf.open(path, ios::out | ios::trunc);
    //********************************************

    WIN32_FIND_DATA find_file_data;

    string find_path = log_path + "/*.*";

    HANDLE h_find = FindFirstFile(find_path.c_str(), &find_file_data);
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
                    if (type_board::sweep_ok != tb)
                        copy_fail_log(outf, f_path, log_path);
                }
            }
        }
        if(!FindNextFile(h_find, &find_file_data)) break;
    }
    PRINT_RESULT(GET_PARA);
    for (auto uk : unknow_board) {
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

void copy_fail_log(print& outf, const string& file_name, const string& f_path)
{
    filesystem::path target_dir = std::filesystem::path(f_path) / "fail_logs";
    if (!filesystem::exists(target_dir)) {
        if (!filesystem::create_directory(target_dir)) {
            outf.display(level::debug, "make dir failed!\n");
            return;
        }
    }
    if (!filesystem::exists(file_name)) {
        outf.display(level::debug, "file is not exists!\n");
        return;
    }
    filesystem::path target_file = target_dir / std::filesystem::path(file_name).filename();
    try {
        if (filesystem::exists(target_file))
            filesystem::remove(target_file);
        filesystem::copy(file_name, target_file, filesystem::copy_options::overwrite_existing);
    } catch (std::filesystem::filesystem_error& e) {
        outf.display(level::debug, "Copying failed: " + string(e.what()) + "\n");
    } catch (const std::exception& e) {
        outf.display(level::debug, "An error occurred: " + string(e.what()) + "\n");
    }
}
