#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <regex>
#include <set>
#include <unordered_set>
#include <unordered_map>

using namespace std;

void get_log_error(const char* p);
bool parse_single_file(ofstream& outf, const string& f_path, string& board_sn);

unordered_set<string> board_uset;

struct count_board{
    int total = 0;
    int sweep_ok = 0;
    int bad_420 = 0;
    int bad_asic = 0;
    int out_temp = 0;
    int out_vol = 0;
    int find_asic_err = 0;
    int sensor_err = 0;
    int env_low = 0;
    int env_high = 0;
    int unknow = 0;
} cb;

bool display_ok = false;
string self_file_name;

int main(int argc, char *argv[])
{
    if (argc > 1 && (string(argv[1]) == "-d" || string(argv[1]) == "--display"))
    {
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
    if(INVALID_HANDLE_VALUE == h_find)
    {
        cout << "error find\n";
        return;
    }

    bool type_clear = false;
    string board_sn;
    unordered_map<string, string> unknow_board;

    while(true)
    {
        if(find_file_data.dwFileAttributes & (~FILE_ATTRIBUTE_DIRECTORY))
        {
            string file_name(find_file_data.cFileName);
            if(file_name != "result.txt" && file_name != self_file_name)
            {
                cout << file_name << endl;
                string f_path(p);
                f_path += "/" + file_name;
                type_clear = parse_single_file(outf, f_path, board_sn);
                if(!type_clear)
                {
                    cb.unknow++;
                    unknow_board.emplace(pair<string, string>(file_name, board_sn == "" ? "unknow" : board_sn));
                }
            }
        }
        if(!FindNextFile(h_find, &find_file_data)) break;
    }

    outf << "\ntotal : " << cb.total << "  ||  sweep ok : " << cb.sweep_ok;
    outf << "\nbad_420 : " << cb.bad_420 << "  ||  bad asic : " << cb.bad_asic;
    outf << "\nout temp : " << cb.out_temp << "  ||  out vol : " << cb.out_vol;
    outf << "\nfind asic err : " << cb.find_asic_err << "  ||  sensor err : " << cb.sensor_err;
    outf << "\nenv temp too low : " << cb.env_low << "  ||  env temp too high : " << cb.env_high;
    outf << "\nunknow board info : " << cb.unknow << "\n";
    for(auto uk : unknow_board)
    {
        outf << uk.first << " board_sn " << uk.second << "\n";
    }
}

regex board_sn_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]board_sn = (.+)");
regex abnormal_cooling_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]abnormal cooling on asic\\[(\\d+)]");
regex asic_null_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]asic \\[(\\d+)]:NULL");
regex out_temp_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]exit info:asic temp outof range");
regex out_vol_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]exit info:asic vol outof range");
regex sensor_err_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]exit info:sensor err");
regex find_asic_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]L1: Find asics");
regex bad_420_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]level_high: \\d, level_low: 3, matched_freq: \\d+, freq_min: \\d+");
regex bad_asic_list_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]bad asic list\\[\\d+]:(\\d+)");
regex sweep_ok_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]L2: Level:  (\\d+)");
regex test_over_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]TEST OVER...");
regex env_low_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]env temp (\\d+) is too low, pattern text exit");
regex env_high_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]env temp (\\d+) is too high, pattern text exit");
regex ft_version_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]current ft_version : (.+), bin : (.+)");

bool parse_single_file(ofstream& outf, const string& f_path, string& board_sn)
{
    ifstream file;
    file.open(f_path, ios::in);
    char buf[2048];
    cmatch m;

    set<int> abnormal_asic;
    set<int> asic_null;
    set<int> asic_bad;

    bool is_bad_420 = false;
    bool type_clear = false;

    string ft, bin;

    board_sn = "";
    while (file.getline(buf,size_t(buf)))
    {
        if(regex_search(buf, m, board_sn_regex))
        {
            if(board_sn != "")
            {
                if(board_sn == m[1])
                    continue;
                else
                {
                    cout << "this log is abnormal"<< endl;
                    cb.total--;
                    type_clear = false;
                    //unknow 
                    break;
                }
            }
            board_sn = m[1];
            if(board_uset.find(board_sn) != board_uset.end())
            {
                cout << "repeated board : " << board_sn << endl;
                type_clear = true;
                break;
            }
            cb.total++;
            board_uset.emplace(board_sn);
            continue;
        }
        if(regex_match(buf, m, ft_version_regex))
        {
            ft = m[1];
            bin = m[2];
            continue;
        }
        if(regex_search(buf, m, abnormal_cooling_regex))
        {
            int asic = stoi(m[1]);
            abnormal_asic.emplace(asic);
            continue;
        }
        if(regex_search(buf, m, asic_null_regex))
        {
            int asic = stoi(m[1]);
            asic_null.emplace(asic);
            continue;
        }
        if(regex_match(buf, m, env_high_regex))
        {
            if(board_sn != "")
            {
                outf << board_sn << " " << ft << " " << bin << " (env too high )" << "[current env temp is : " << stoi(m[1]) << "]\n";
                cb.env_high++;
                type_clear = true;
            }
            break;
        }
        if(regex_match(buf, m, env_low_regex))
        {
            if(board_sn != "")
            {
                outf << board_sn << " " << ft << " " << bin << " ( env too low )" << "[current env temp is : " << stoi(m[1]) << "]\n";
                cb.env_low++;
                type_clear = true;
            }
            break;
        }
        if(regex_match(buf, m, out_temp_regex))
        {
            if(board_sn != "")
            {
                outf << board_sn << " " << ft << " " << bin << " (   out temp  )";
                if(abnormal_asic.size() > 0)
                {
                    outf << "[abnormal asic : ";
                    for(auto asic : abnormal_asic)
                    {
                        outf << asic << " ";
                    }
                    outf << "]";
                }
                if(asic_null.size() > 0)
                {
                    outf << "[asic null : ";
                    for(auto asic : asic_null)
                    {
                        outf << asic << " ";
                    }
                    outf << "]";
                }
                outf << "\n";
                cb.out_temp++;
                type_clear = true;
            }
            break;
        }
        if(regex_match(buf, m, out_vol_regex))
        { 
            if(board_sn != "")
            {
                outf << board_sn << " " << ft << " " << bin << " (   out vol   )";
                if(asic_null.size() > 0)
                {
                    outf << "[asic null : ";
                    for(auto asic : asic_null)
                    {
                        outf << asic << " ";
                    }
                    outf << "]";
                }
                outf << "\n";
                cb.out_vol++;
                type_clear = true;
            }
            break;
        }
        if(regex_match(buf, m, sensor_err_regex))
        {
            if(board_sn != "")
            {
                outf << board_sn << " " << ft << " " << bin << " ( sensor err  )";
                outf << "\n";
                cb.sensor_err++;
                type_clear = true;
            }
            break;
        }
        if(regex_search(buf, m, find_asic_regex))
        {
            if(board_sn != "")
            {
                outf << board_sn << " " << ft << " " << bin << " (find asic err)";
                outf << "\n";
                cb.find_asic_err++;
                type_clear = true;
            }
            break;
        }
        if(regex_match(buf, m, bad_420_regex))
        {
            is_bad_420 = true;
            continue;
        }
        if(regex_search(buf, m, bad_asic_list_regex))
        {
            int asic = stoi(m[1]);
            asic_bad.emplace(asic);
            continue;
        }
        if(regex_search(buf, m, sweep_ok_regex))
        {
            if(board_sn != "")
            {
                if(display_ok)
                {
                    outf << board_sn << " " << ft << " " << bin << " (  sweep ok   )" << "[level : " << stoi(m[1]) << "]";
                    outf << "\n";
                }
                cb.sweep_ok++;
                type_clear = true;
            }
            break;
        }
        if(regex_match(buf, m, test_over_regex))
        {
            if(board_sn != "")
            {
                if(asic_bad.size() > 0)
                {
                    if(is_bad_420)
                    {
                        cb.bad_420++;
                        outf << board_sn << " " << ft << " " << bin << " (   bad 420   )";
                    }
                    else
                    {
                        cb.bad_asic++;
                        outf << board_sn << " " << ft << " " << bin << " (   bad asic  )";
                    }
                    outf << "[bad asic : ";
                    for(auto asic : asic_bad)
                    {
                        outf << asic <<" ";
                    }
                    outf << "]\n";
                    type_clear = true;
                }
                break;
            }
        }
    }

    file.close();
    return type_clear;
}