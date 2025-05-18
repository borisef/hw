#ifndef IMAGE_COMPARISON_EXPERT_H
#define IMAGE_COMPARISON_EXPERT_H

#include <string>
#include <opencv2/opencv.hpp>
#include <map>


class ImageComparisonExpert {
public:
    enum Mode { QUIET, INFORMATIVE, DEBUG };

    ImageComparisonExpert(double thr_min, double thr_max, double thr_mean, double thr_median,
                          int minBadPixels, int maxAbsDiff, Mode mode = QUIET, bool convert2gray = false);

    bool Compare(const std::string& pathImage1, const std::string& pathImage2);

    std::vector<std::string> ListFilesInDirectory(const std::string& folder);


    bool CompareFolders(
        const std::string& ground_truth_folder,
        const std::string& results_folder,
        const std::string& token1,
        const std::string& token2,
        const std::string& ext1,
        const std::string& ext2
    );

    void SetThresholdMin(double value)    { m_thr_min = value; }
    void SetThresholdMax(double value)    { m_thr_max = value; }
    void SetThresholdMean(double value)   { m_thr_mean = value; }
    void SetThresholdMedian(double value) { m_thr_median = value; }

    void SetAllThresholds(double thr_min, double thr_max, double thr_mean, double thr_median) {
        m_thr_min = thr_min; m_thr_max = thr_max; m_thr_mean = thr_mean; m_thr_median = thr_median;
    }

    void SetMinBadPixels(int value)       { m_minBadPixels = value; }
    void SetMaxAbsDiff(int value)         { m_maxAbsDiff = value; }
    void SetMode(Mode mode)               { m_mode = mode; }
    void SetConvertToGray(bool value)     { m_convert2gray = value; }

    std::map<int, std::string> m_gt_map;
    std::map<int, std::string> m_res_map;

private:
    double m_thr_min, m_thr_max, m_thr_mean, m_thr_median;
    int m_minBadPixels, m_maxAbsDiff;
    Mode m_mode;
    bool m_convert2gray;

    void Print(const std::string& msg);
    void ShowDebug(const cv::Mat& img1, const cv::Mat& img2, const cv::Mat& diff);

    bool ExtractFrameNumber(const std::string& filename, const std::string& token, const std::string& ext, int& outNumber);
};

#endif // IMAGE_COMPARISON_EXPERT_H
