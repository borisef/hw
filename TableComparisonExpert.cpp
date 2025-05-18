#include "TableComparisonExpert.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

// Constructor for C++11 (no inline initialization)
TableComparisonExpert::TableComparisonExpert() : m_mode(SILENT) {}

std::vector<std::string> TableComparisonExpert::SplitCSVLine(const std::string& line) {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, ',')) {
        result.push_back(item);
    }
    return result;
}

bool TableComparisonExpert::ParseCSV(const std::string& path, const std::string& frame_column,
                                     std::vector<Row>& table, std::unordered_map<int, size_t>& frame_map) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    if (!std::getline(file, line)) return false;
    std::vector<std::string> headers = SplitCSVLine(line);

    size_t frame_col_idx = std::distance(headers.begin(),
        std::find(headers.begin(), headers.end(), frame_column));

    if (frame_col_idx >= headers.size()) return false;

    size_t index = 0;
    while (std::getline(file, line)) {
        std::vector<std::string> values = SplitCSVLine(line);
        if (values.size() != headers.size()) continue;

        Row row;
        for (size_t i = 0; i < headers.size(); ++i) {
            row[headers[i]] = values[i];
        }

        int frame = std::stoi(values[frame_col_idx]);
        frame_map[frame] = index++;
        table.push_back(row);
    }
    return true;
}

bool TableComparisonExpert::ReadCSVData(const std::string& gt_csv_path, const std::string& res_csv_path,
                                        const std::string& frame_column_gt, const std::string& frame_column_res) {
    gt_table.clear(); res_table.clear();
    gt_frame_map.clear(); res_frame_map.clear();
    return ParseCSV(gt_csv_path, frame_column_gt, gt_table, gt_frame_map) &&
           ParseCSV(res_csv_path, frame_column_res, res_table, res_frame_map);
}

bool TableComparisonExpert::CompareRows(int frameNumber, const std::string& columnName1,
                                        const std::string& columnName2, double threshold,
                                        const std::string& column_type) {
    std::unordered_map<int, size_t>::const_iterator it1 = gt_frame_map.find(frameNumber);
    std::unordered_map<int, size_t>::const_iterator it2 = res_frame_map.find(frameNumber);
    if (it1 == gt_frame_map.end() || it2 == res_frame_map.end()) return false;

    const Row& row1 = gt_table[it1->second];
    const Row& row2 = res_table[it2->second];

    if (row1.find(columnName1) == row1.end() || row2.find(columnName2) == row2.end()) {
        if (m_mode != SILENT) {
            std::cout << "FAIL: Column not found in one of the tables for frame " << frameNumber << std::endl;
        }
        return false;
    }

    const std::string& val1 = row1.at(columnName1);
    const std::string& val2 = row2.at(columnName2);

    bool result = true;
    if (column_type == "bool") {
        result = (val1 == val2);
    } else if (column_type == "int") {
        result = std::abs(std::stoi(val1) - std::stoi(val2)) <= static_cast<int>(threshold);
    } else {
        result = std::abs(std::stod(val1) - std::stod(val2)) <= threshold;
    }

    if (!result && m_mode != SILENT) {
        std::cout << "FAIL: Frame " << frameNumber << " | "
                  << columnName1 << ": " << val1 << " vs "
                  << columnName2 << ": " << val2 << std::endl;
        if (m_mode == DEBUG) {
            std::cout << "Press Enter to continue...";
            std::cin.get();
        }
    }
    return result;
}

bool TableComparisonExpert::CompareAllRows(const std::string& columnName1,
                                           const std::string& columnName2,
                                           double threshold,
                                           const std::string& column_type,
                                           Mode mode) {
    m_mode = mode;
    bool all_passed = true;
    for (std::unordered_map<int, size_t>::const_iterator it = res_frame_map.begin(); it != res_frame_map.end(); ++it) {
        int frame = it->first;
        if (!CompareRows(frame, columnName1, columnName2, threshold, column_type)) {
            all_passed = false;
        }
    }
    return all_passed;
}
