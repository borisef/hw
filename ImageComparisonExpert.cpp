#include "ImageComparisonExpert.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <sys/stat.h>
#include <vector>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
#endif



std::vector<std::string> ImageComparisonExpert::ListFilesInDirectory(const std::string& folder) {
    std::vector<std::string> files;

#ifdef _WIN32
    std::string pattern = folder + "\\*";
    WIN32_FIND_DATAA data;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &data);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(data.cFileName);
            }
        } while (FindNextFileA(hFind, &data));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(folder.c_str());
    if (dir) {
        struct dirent* ent;
        while ((ent = readdir(dir)) != nullptr) {
            if (ent->d_type == DT_REG) {
                files.push_back(ent->d_name);
            }
        }
        closedir(dir);
    }
#endif

    return files;
}



ImageComparisonExpert::ImageComparisonExpert(double thr_min, double thr_max, double thr_mean, double thr_median,
                                             int minBadPixels, int maxAbsDiff, Mode mode, bool convert2gray)
    : m_thr_min(thr_min), m_thr_max(thr_max), m_thr_mean(thr_mean), m_thr_median(thr_median),
      m_minBadPixels(minBadPixels), m_maxAbsDiff(maxAbsDiff), m_mode(mode), m_convert2gray(convert2gray) {}

void ImageComparisonExpert::Print(const std::string& msg) {
    if (m_mode != QUIET) std::cout << msg << std::endl;
}

void ImageComparisonExpert::ShowDebug(const cv::Mat& img1, const cv::Mat& img2, const cv::Mat& diff) {
    if (m_mode != DEBUG) return;

    const int pad = 30;
    auto prep = [](const cv::Mat& src) {
        cv::Mat gray, bgr;
        if (src.channels() == 1) gray = src;
        else cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
        gray.convertTo(gray, CV_8U);
        cv::cvtColor(gray, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    };

    cv::Mat img1_disp = prep(img1);
    cv::Mat img2_disp = prep(img2);
    cv::Mat diff_disp = diff.empty() ?
        cv::Mat(img1_disp.size(), CV_8UC3, cv::Scalar(200, 200, 200)) : prep(diff);

    if (diff.empty()) {
        cv::putText(diff_disp, "No diff image", cv::Point(10, img1_disp.rows / 2),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 1);
    }

    auto addTitle = [&](cv::Mat& img, const std::string& title) {
        cv::Mat top(pad, img.cols, CV_8UC3, cv::Scalar(255, 255, 255));
        cv::putText(top, title, cv::Point(10, pad - 10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 1);
        cv::vconcat(top, img, img);
    };

    addTitle(img1_disp, "Image 1");
    addTitle(img2_disp, "Image 2");
    addTitle(diff_disp, "Difference");

    cv::Mat combined;
    std::vector<cv::Mat> panels = { img1_disp, img2_disp, diff_disp };
    cv::hconcat(panels, combined);
    cv::imshow("Debug View", combined);
    cv::waitKey(0);
}

bool ImageComparisonExpert::Compare(const std::string& pathImage1, const std::string& pathImage2) {
    if (pathImage1.empty() || pathImage2.empty()) {
        Print("FAIL: Image path(s) empty.");
        return false;
    }

    cv::Mat raw1 = cv::imread(pathImage1, cv::IMREAD_UNCHANGED);
    cv::Mat raw2 = cv::imread(pathImage2, cv::IMREAD_UNCHANGED);
    if (raw1.empty() || raw2.empty()) {
        Print("FAIL: Failed to load one or both images.");
        return false;
    }

    cv::Mat img1, img2;
    if (raw1.channels() == 3 && m_convert2gray)
        cv::cvtColor(raw1, img1, cv::COLOR_BGR2GRAY);
    else if (raw1.channels() == 1)
        img1 = raw1;
    else {
        Print("FAIL: Image 1 format not supported.");
        ShowDebug(raw1, raw2, cv::Mat());
        return false;
    }

    if (raw2.channels() == 3 && m_convert2gray)
        cv::cvtColor(raw2, img2, cv::COLOR_BGR2GRAY);
    else if (raw2.channels() == 1)
        img2 = raw2;
    else {
        Print("FAIL: Image 2 format not supported.");
        ShowDebug(raw1, raw2, cv::Mat());
        return false;
    }

    if (img1.size() != img2.size()) {
        Print("FAIL: Image sizes do not match.");
        ShowDebug(img1, img2, cv::Mat());
        return false;
    }

    double min1, max1, mean1, median1;
    double min2, max2, mean2, median2;

    cv::minMaxLoc(img1, &min1, &max1);
    mean1 = cv::mean(img1)[0];
    std::vector<uchar> sorted1;
    for (int i = 0; i < img1.rows; ++i)
        for (int j = 0; j < img1.cols; ++j)
            sorted1.push_back(img1.at<uchar>(i, j));
    std::sort(sorted1.begin(), sorted1.end());
    median1 = sorted1[sorted1.size() / 2];

    cv::minMaxLoc(img2, &min2, &max2);
    mean2 = cv::mean(img2)[0];
    std::vector<uchar> sorted2;
    for (int i = 0; i < img2.rows; ++i)
        for (int j = 0; j < img2.cols; ++j)
            sorted2.push_back(img2.at<uchar>(i, j));
    std::sort(sorted2.begin(), sorted2.end());
    median2 = sorted2[sorted2.size() / 2];

    if (std::abs(min1 - min2) > m_thr_min ||
        std::abs(max1 - max2) > m_thr_max ||
        std::abs(mean1 - mean2) > m_thr_mean ||
        std::abs(median1 - median2) > m_thr_median) {
        Print("FAIL: Statistical thresholds exceeded.");
        ShowDebug(img1, img2, cv::Mat());
        return false;
    }

    cv::Mat diff;
    cv::absdiff(img1, img2, diff);
    int badPixels = 0;
    for (int i = 0; i < diff.rows; ++i)
        for (int j = 0; j < diff.cols; ++j)
            if (diff.at<uchar>(i, j) > m_maxAbsDiff) ++badPixels;

    Print("Bad pixels count = " + std::to_string(badPixels));
    if (badPixels >= m_minBadPixels) {
        Print("FAIL: Too many bad pixels.");
        ShowDebug(img1, img2, diff);
        return false;
    }

    return true;
}

int extract_frame_number(const std::string& filename, const std::string& token, const std::string& ext) {
    if (filename.find(token) != 0 || filename.size() <= token.size() + ext.size() + 1)
        return -1;

    std::string core = filename.substr(token.size(), filename.size() - token.size() - ext.size() - 1);
    try {
        return std::stoi(core);
    } catch (...) {
        return -1;
    }
    }

bool ImageComparisonExpert::ExtractFrameNumber(const std::string& filename, const std::string& token, const std::string& ext, int& outNumber) {
    if (filename.find(token) != 0) return false;
    size_t endPos = filename.rfind("." + ext);
    if (endPos == std::string::npos) return false;
    std::string core = filename.substr(token.length(), endPos - token.length());
    try {
        outNumber = std::stoi(core);
        return true;
    } catch (...) {
        return false;
    }
}




bool ImageComparisonExpert::CompareFolders(
    const std::string& ground_truth_folder,
    const std::string& results_folder,
    const std::string& token1,
    const std::string& token2,
    const std::string& ext1,
    const std::string& ext2
) {
    m_gt_map.clear();
    m_res_map.clear();



    // Ground truth files
    for (const auto& fname : ListFilesInDirectory(ground_truth_folder)) {
       int fnum = extract_frame_number(fname, token1, ext1);
        if (fnum >= 0) {
            m_gt_map[fnum] = ground_truth_folder + "/" + fname;
        }
    }

    // Result files
    for (const auto& fname : ListFilesInDirectory(results_folder)) {
        int fnum = extract_frame_number(fname, token2, ext2);
        if (fnum >= 0) {
            m_res_map[fnum] = results_folder + "/" + fname;
        }
    }

    // Compare
    int failed = 0;
    for (std::map<int, std::string>::const_iterator it_res = m_res_map.begin(); it_res != m_res_map.end(); ++it_res) {
    int frame = it_res->first;
    const std::string& res_path = it_res->second;

    std::map<int, std::string>::const_iterator it_gt = m_gt_map.find(frame);
    if (it_gt != m_gt_map.end()) {
        const std::string& gt_path = it_gt->second;
        std::cout << "Comparing frame " << frame << ": " << gt_path << " <-> " << res_path << std::endl;
        bool result = Compare(gt_path, res_path);
        std::cout << "Result: " << (result ? "PASS" : "FAIL") << std::endl;
        if (!result) {
            failed++;
        }
    } else {
        std::cout << "Warning: No ground truth found for frame " << frame << std::endl;
        failed++;
    }
}

    std::cout << "ImageComparisonExpert::CompareFolders, Total failed frames count is "
              << failed << " out of " << m_res_map.size() << std::endl;

    return failed == 0;
}
