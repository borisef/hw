// TableComparisonExpert.h
#ifndef TABLE_COMPARISON_EXPERT_H
#define TABLE_COMPARISON_EXPERT_H
#include <string>
#include <unordered_map>
#include <vector>

class TableComparisonExpert {
public:
    enum Mode { SILENT, INFORMATIVE, DEBUG };

    bool ReadCSVData(const std::string& gt_csv_path, const std::string& res_csv_path,
                     const std::string& frame_column_gt, const std::string& frame_column_res);

    bool CompareRows(int frameNumber, const std::string& columnName1, const std::string& columnName2,
                     double threshold, const std::string& column_type = "float");

    bool CompareAllRows(const std::string& columnName1, const std::string& columnName2,
                        double threshold = 0.0, const std::string& column_type = "float", Mode mode = SILENT);

private:
    using Row = std::unordered_map<std::string, std::string>;
    std::vector<Row> gt_table, res_table;
    std::unordered_map<int, size_t> gt_frame_map, res_frame_map;

    std::vector<std::string> SplitCSVLine(const std::string& line);
    bool ParseCSV(const std::string& path, const std::string& frame_column,
                  std::vector<Row>& table, std::unordered_map<int, size_t>& frame_map);

    Mode m_mode = SILENT;
};

#endif // TABLE_COMPARISON_EXPERT_H