#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include "regex_info.h"
#include "count_board.h"

using namespace std;

void get_log_error(const char* p);
type_board parse_single_file(ofstream& outf, const string& f_path, string& board_sn);

unordered_set<string> board_uset;
count_board cb;

bool display_ok = false;
string self_file_name;

int main(int argc, char *argv[]){
    if (argc > 1 && (string(argv[1]) == "-d" || string(argv[1]) == "--display")){
        cout << "should display the ok board" << endl;
        display_ok = true;
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

    std::cout << "Executable directory: " << &buffer[0] << std::endl;
    std::cout << "Executable file: " << self_file_name << std::endl;

    board_uset.clear();

    get_log_error(&buffer[0]);
    
    system("pause");

    return 0;
}

void get_log_error(const char* p)
{
    ofstream outf;
    string path(p);
    path = path + "/result.txt";
    outf.open(path, ios::out | ios::trunc);
    //********************************************

    char find_path[MAX_PATH];
    WIN32_FIND_DATA find_file_data;

    strcpy(find_path, p);
    strcat(find_path, "\\*.*");

    HANDLE h_find = FindFirstFile(find_path, &find_file_data);
    if(INVALID_HANDLE_VALUE == h_find){
        cout << "error find\n";
        return;
    }

    type_board tb;
    string board_sn;
    unordered_map<string, string> unknow_board;

    while(true){
        if(find_file_data.dwFileAttributes & (~FILE_ATTRIBUTE_DIRECTORY)){
            string file_name(find_file_data.cFileName);
            if(file_name != "result.txt" && file_name != self_file_name){
                cout << file_name << endl;
                string f_path(p);
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

#define GET_DIS_STR [&](void* ptr){\
            string str = "";\
            char c_str[100];\
            sprintf(c_str, "%s %s %s \0", board_sn.c_str(), ft.c_str(), bin.c_str());\
            str += string(c_str);\
            str += get_board_type_str(tb);\
            switch(tb){\
                case type_board::bad_420 :\
                case type_board::bad_asic : str += " [bad asic : "; for(auto asic : asic_bad) str += to_string(asic) + " "; str +="]";  break;\
                case type_board::out_temp :\
                case type_board::out_vol :\
                    if(abnormal_asic.size() > 0){\
                        str += " [abnormal asic : ";\
                        for(auto asic : abnormal_asic)\
                            str += to_string(asic) + " ";\
                        str += "]";\
                    }\
                    if(asic_null.size() > 0){\
                        str += " [asic null : ";\
                        for(auto asic : asic_null)\
                            str += to_string(asic) + " ";\
                        str += "]";\
                    }break;\
                case type_board::env_low:\
                case type_board::env_high: str += " [current env temp is : " + to_string(env_temp) + "]"; break;\
                default: break;\
            }\
            str += "\n";\
            return str;\
}

type_board parse_single_file(ofstream& outf, const string& f_path, string& board_sn){
    ifstream file;
    file.open(f_path, ios::in);
    char buf[2048];
    cmatch m;

    set<int> abnormal_asic;
    set<int> asic_null;
    set<int> asic_bad;

    type_board tb = type_board::unknown;

    string ft, bin;
    int env_temp = 0;

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