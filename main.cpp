#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <regex>
#include <set>
#include <unordered_set>

using namespace std;

void get_log_error(const char* p);
void parse_single_file(ofstream& outf, const string& f_path);

unordered_set<string> board_uset;

int main() {
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
        *lastBackslash = '\0'; // 将最后一个反斜杠替换为空字符，从而去掉文件名
    }

    std::cout << "Executable directory: " << &buffer[0] << std::endl;

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

    while(true)
    {
        if(find_file_data.dwFileAttributes & (~FILE_ATTRIBUTE_DIRECTORY))
        {
            string file_name(find_file_data.cFileName);
            if(file_name != "main.exe" && file_name != "result.txt")
            {
                cout << file_name << endl;
                string f_path(p);
                f_path += "/" + file_name;
                parse_single_file(outf, f_path);
            }
        }
        if(!FindNextFile(h_find, &find_file_data)) break;
    }
}

regex board_sn_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]board_sn = (.+)");
regex abnormal_cooling_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]abnormal cooling on asic\\[(\\d+)]");
regex asic_null_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]asic \\[(\\d+)]:NULL");
regex out_temp_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]exit info:asic temp outof range");
regex out_vol_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]exit info:asic vol outof range");
regex sensor_err_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]exit info:sensor err");
regex find_asic_regex("\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}]L1: Find asics");



void parse_single_file(ofstream& outf, const string& f_path)
{
    ifstream file;
    file.open(f_path, ios::in);
    char buf[1024];
    cmatch m;

    string board_sn = "";
    set<int> abnormal_asic;
    set<int> asic_null;


    while (file.getline(buf,size_t(buf)))
    {
        if(regex_search(buf, m, board_sn_regex))
        {
            board_sn = m[1];
            if(board_uset.find(board_sn) != board_uset.end())
            {
                cout << "repeated board : " << board_sn << endl;
                break;
            }
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
        if(regex_match(buf, m, out_temp_regex))
        {
            if(board_sn != "" && board_uset.find(board_sn) == board_uset.end())
            {
                outf << board_sn << " (   out temp  )";
                if(abnormal_asic.size() > 0)
                {
                    outf << " [abnormal asic : ";
                    for(auto asic : abnormal_asic)
                    {
                        outf << asic << " ";
                    }
                    outf << "]";
                }
                if(asic_null.size() > 0)
                {
                    outf << " [asic null : ";
                    for(auto asic : asic_null)
                    {
                        outf << asic << " ";
                    }
                    outf << "]";
                }

                outf << "\n";

                board_uset.emplace(board_sn);
            }
            break;
        }
        if(regex_match(buf, m, out_vol_regex))
        {
            if(board_sn != "" && board_uset.find(board_sn) == board_uset.end())
            {
                outf << board_sn << " (   out vol   )";
                if(asic_null.size() > 0)
                {
                    outf << " [asic null : ";
                    for(auto asic : asic_null)
                    {
                        outf << asic << " ";
                    }
                    outf << "]";
                }

                outf << "\n";

                board_uset.emplace(board_sn);
            }
            break;
        }
        if(regex_match(buf, m, sensor_err_regex))
        {
            if(board_sn != "" && board_uset.find(board_sn) == board_uset.end())
            {
                outf << board_sn << " ( sensor err  )";
                outf << "\n";

                board_uset.emplace(board_sn);
            }
            break;
        }
        if(regex_search(buf, m, find_asic_regex))
        {
            if(board_sn != "" && board_uset.find(board_sn) == board_uset.end())
            {
                outf << board_sn << " (find asic err)";
                outf << "\n";

                board_uset.emplace(board_sn);
            }
            break;
        }
    }

    file.close();
}