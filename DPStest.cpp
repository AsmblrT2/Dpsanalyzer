#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <map>
#include <set>
#include <vector>
#include <ctime>
#include <Windows.h>

std::string GetAppDirectory() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string appPath(buffer);
    return appPath.substr(0, appPath.find_last_of("\\/"));
}

bool IsValidDate(const std::string& dateStr, std::tm& tmOut) {
    int y, m, d, H, M, S;
    if (sscanf(dateStr.c_str(), "%d/%d/%d:%d:%d:%d", &y, &m, &d, &H, &M, &S) != 6)
        return false;
    if (y < 2025) return false;  // Only allow 2025 and later
    tmOut = {};
    tmOut.tm_year = y - 1900;
    tmOut.tm_mon = m - 1;
    tmOut.tm_mday = d;
    tmOut.tm_hour = H;
    tmOut.tm_min = M;
    tmOut.tm_sec = S;
    tmOut.tm_isdst = -1;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: dps.exe <dumpfile.txt>\n";
        return 1;
    }

    std::string inputFile = argv[1];
    std::ifstream inFile(inputFile);
    if (!inFile) {
        std::cerr << "Error opening file: " << inputFile << "\n";
        return 1;
    }

    std::string appDir = GetAppDirectory();
    std::ofstream modFile(appDir + "\\modifiedextension.txt");
    std::ofstream dupFile(appDir + "\\duplicate.txt");

    std::regex pattern("!!([^!]+)!([0-9]{4}/[0-9]{2}/[0-9]{2}:[0-9]{2}:[0-9]{2}:[0-9]{2})!");

    std::map<std::string, std::set<std::string> > appDateMap;
    std::string line;

    while (std::getline(inFile, line)) {
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            std::string fullApp = match[1].str(); // app.exe
            std::string dateStr = match[2].str();

            std::tm tmDate;
            if (!IsValidDate(dateStr, tmDate))
                continue;

            // Extract extension
            std::string ext = "";
            size_t dotPos = fullApp.rfind('.');
            if (dotPos != std::string::npos) {
                ext = fullApp.substr(dotPos + 1);
            }

            // Check if extension is not "exe"
            if (_stricmp(ext.c_str(), "exe") != 0) {
                modFile << "[+] Non-EXE: " << line << "\n";
            }

            appDateMap[fullApp].insert(dateStr);
        }
    }

    // Check for duplicate apps with different compile dates
    std::map<std::string, std::set<std::string> >::const_iterator it;
    for (it = appDateMap.begin(); it != appDateMap.end(); ++it) {
        const std::string& app = it->first;
        const std::set<std::string>& dates = it->second;
        if (dates.size() > 1) {
            dupFile << "[+] Duplicate Dates for: " << app << "\n";
            std::set<std::string>::const_iterator dt;
            for (dt = dates.begin(); dt != dates.end(); ++dt) {
                dupFile << "    -> " << *dt << "\n";
            }
        }
    }

    std::cout << "[+] Analysis complete. Reports saved to:\n";
    std::cout << "    modifiedextension.txt\n";
    std::cout << "    duplicate.txt\n";

    return 0;
}
