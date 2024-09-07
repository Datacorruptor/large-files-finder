#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <windows.h>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <sstream>


using namespace std;

struct FileInfo {
    string filePath;
    uint64_t fileSize;

    FileInfo(string path, uint64_t size) : filePath(path), fileSize(size) {}
};

// Recursively find files on disk and collect their size
uint64_t findFiles(const string& directory, vector<FileInfo>& files, const string& obj_type) {
	
	uint64_t sum = 0;
	
    WIN32_FIND_DATA fileData;
    HANDLE hFind = FindFirstFile((directory + "\\*").c_str(), &fileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        string fileName = fileData.cFileName;
        if (fileName == "." || fileName == "..") {
            continue;  // skip special directories
        }

        string filePath = directory + "\\" + fileName;
        if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // If it's a directory, recurse into it
            uint64_t d_sum = findFiles(filePath, files, obj_type);
			if (obj_type == "D" || obj_type == "B"){
				files.emplace_back(filePath, d_sum);
			}
			sum += d_sum;
			
			
        } else {
            // It's a file, add to the list
            LARGE_INTEGER fileSize;
            fileSize.HighPart = fileData.nFileSizeHigh;
            fileSize.LowPart = fileData.nFileSizeLow;
			if (obj_type == "F" || obj_type == "B"){
				files.emplace_back(filePath, fileSize.QuadPart);
			}
			sum += fileSize.QuadPart;
        }
    } while (FindNextFile(hFind, &fileData) != 0);

    FindClose(hFind);
	return sum;
}

// Comparator function to sort files by size
bool compareBySize(const FileInfo& a, const FileInfo& b) {
    return a.fileSize > b.fileSize;
}


// Prints to the provided buffer a nice number of bytes (KB, MB, GB, etc)
string pretty_bytes(uint64_t bytes)
{
    string suffixes[7];
    suffixes[0] = "B";
    suffixes[1] = "KB";
    suffixes[2] = "MB";
    suffixes[3] = "GB";
    suffixes[4] = "TB";
    suffixes[5] = "PB";
    suffixes[6] = "EB";
    int s = 0; // which suffix to use
    double count = bytes;
    while (count >= 1024 && s < 7)
    {
        s++;
        count /= 1024;
    }
	
	stringstream buffer;
	buffer.precision(2);
    buffer << fixed << count << " " << suffixes[s];
	return buffer.str();
}

int main() {
	
	string drive;
    cout << "Enter the path to scan (e.g., C: or C:\\Program Files ): ";
    //cin >> drive;
	getline(std::cin, drive);
	
	string obj_type;
	cout << "Enter the object to scan\nF - files only\nD - directories only\nB - Both\n";
	getline(std::cin, obj_type);
	
    vector<FileInfo> files;

    cout << "Scanning the path " << drive <<", this may take a while..." << endl;
	
	auto start = std::chrono::high_resolution_clock::now();
    findFiles(drive, files, obj_type);
	
	

    cout << "Total objects found: " << files.size() << endl;

    // Sort files by size in descending order
    sort(files.begin(), files.end(), compareBySize);

    // Print top 100 largest objects
    int count = min(100, static_cast<int>(files.size()));
    cout << "\nTop " << count << " largest objects:\n";
    for (int i = 0; i < count; ++i) {
		
        cout << setw(3) << i + 1 << ". " 
		     << setw(10) << pretty_bytes(files[i].fileSize)
             << " (" << files[i].fileSize << " bytes) - " 
             << files[i].filePath << endl;
    }
	
	auto end = std::chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;
	
	cout << "\nScan completed in " << diff.count() << " seconds." << std::endl;
	cout << "Press Enter to quit" << std::endl;
	string exi;
    getline(std::cin, exi);

    return 0;
}
