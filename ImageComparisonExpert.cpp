#include "ImageComparisonExpert.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <filesystem>

ImageComparisonExpert::ImageComparisonExpert(double thr_min, double thr_max, double thr_mean, double thr_median,
                                             int minBadPixels, int maxAbsDiff, Mode mode, bool convert2gray)
    : m_thr_min(thr_min), m_thr_max(thr_max), m_thr_mean(thr_mean), m_thr_median(thr_median),
      m_minBadPixels(minBadPixels), m_maxAbsDiff(maxAbsDiff), m_mode(mode), m_convert2gray(convert2gray) {}


void ImageComparisonExpert::Print(const std::string& msg) {
    if (m_mode != QUIET) {
        std::cout << msg << std::endl;
    }
}

void ImageComparisonExpert::ShowDebug(const cv::Mat& img1, const cv::Mat& img2, const cv::Mat& diff) {
    if (m_mode != DEBUG) return;

    cv::Mat img1_disp, img2_disp, diff_disp;

    auto prep = [](const cv::Mat& src) {
        cv::Mat gray, bgr;
        if (src.channels() == 1) gray = src;
        else cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
        gray.convertTo(gray, CV_8U);
        cv::cvtColor(gray, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    };

    img1_disp = prep(img1);
    img2_disp = prep(img2);

    if (!diff.empty()) {
        diff_disp = prep(diff);
    } else {
        diff_disp = cv::Mat(img1_disp.size(), CV_8UC3, cv::Scalar(200, 200, 200));
        cv::putText(diff_disp, "No diff image", cv::Point(10, img1_disp.rows / 2),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 1);
    }

    const int pad = 30;
    auto addTitle = [&](cv::Mat& img, const std::string& title) {
        cv::Mat top(pad, img.cols, CV_8UC3, cv::Scalar(255, 255, 255));
        cv::putText(top, title, cv::Point(10, pad - 10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 1);
        cv::vconcat(top, img, img);
    };

    addTitle(img1_disp, "Image 1");
    addTitle(img2_disp, "Image 2");
    addTitle(diff_disp, "Difference");

    cv::Mat combined;
    cv::hconcat(std::vector<cv::Mat>{img1_disp, img2_disp, diff_disp}, combined);
    cv::imshow("Debug View", combined);
    cv::waitKey(0);
}


bool ImageComparisonExpert::Compare(const std::string& pathImage1, const std::string& pathImage2) {
    std::string failReason;

    if (!std::filesystem::exists(pathImage1) || !std::filesystem::exists(pathImage2)) {
        failReason = "One or both image files do not exist.";
        Print("FAIL: " + failReason);
        return false;
    }

    cv::Mat raw1 = cv::imread(pathImage1, cv::IMREAD_UNCHANGED);
    cv::Mat raw2 = cv::imread(pathImage2, cv::IMREAD_UNCHANGED);

    if (raw1.empty() || raw2.empty()) {
        failReason = "Failed to load one or both images.";
        Print("FAIL: " + failReason);
        return false;
    }

    cv::Mat img1, img2;

    // Convert or reject
    if (raw1.channels() == 3 && m_convert2gray) {
        cv::cvtColor(raw1, img1, cv::COLOR_BGR2GRAY);
        Print("Converted image 1 to grayscale.");
    } else if (raw1.channels() == 1) {
        img1 = raw1;
    } else {
        failReason = "Image 1 is not grayscale and conversion is disabled.";
        Print("FAIL: " + failReason);
        ShowDebug(raw1, raw2, cv::Mat());  // show original RGBs if available
        return false;
    }

    if (raw2.channels() == 3 && m_convert2gray) {
        cv::cvtColor(raw2, img2, cv::COLOR_BGR2GRAY);
        Print("Converted image 2 to grayscale.");
    } else if (raw2.channels() == 1) {
        img2 = raw2;
    } else {
        failReason = "Image 2 is not grayscale and conversion is disabled.";
        Print("FAIL: " + failReason);
        ShowDebug(raw1, raw2, cv::Mat());
        return false;
    }

    if (m_mode != QUIET) {
        std::cout << "Image 1 size: " << img1.cols << "x" << img1.rows << "\n";
        std::cout << "Image 2 size: " << img2.cols << "x" << img2.rows << "\n";
    }

    if (img1.size() != img2.size()) {
        failReason = "Images have different sizes.";
        Print("FAIL: " + failReason);
        ShowDebug(img1, img2, cv::Mat());
        return false;
    }

    // Statistics
    double min1, max1, mean1, median1;
    double min2, max2, mean2, median2;

    cv::minMaxLoc(img1, &min1, &max1);
    mean1 = cv::mean(img1)[0];
    std::vector<uchar> sorted1(img1.begin<uchar>(), img1.end<uchar>());
    std::sort(sorted1.begin(), sorted1.end());
    median1 = sorted1[sorted1.size() / 2];

    cv::minMaxLoc(img2, &min2, &max2);
    mean2 = cv::mean(img2)[0];
    std::vector<uchar> sorted2(img2.begin<uchar>(), img2.end<uchar>());
    std::sort(sorted2.begin(), sorted2.end());
    median2 = sorted2[sorted2.size() / 2];

    if (m_mode != QUIET) {
        std::cout << "Image 1: min=" << min1 << ", max=" << max1 << ", mean=" << mean1 << ", median=" << median1 << "\n";
        std::cout << "Image 2: min=" << min2 << ", max=" << max2 << ", mean=" << mean2 << ", median=" << median2 << "\n";
    }

    if (std::abs(min1 - min2) > m_thr_min) {
        failReason = "Min values differ too much.";
        Print("FAIL: " + failReason);
        ShowDebug(img1, img2, cv::Mat());
        return false;
    }

    if (std::abs(max1 - max2) > m_thr_max) {
        failReason = "Max values differ too much.";
        Print("FAIL: " + failReason);
        ShowDebug(img1, img2, cv::Mat());
        return false;
    }

    if (std::abs(mean1 - mean2) > m_thr_mean) {
        failReason = "Mean values differ too much.";
        Print("FAIL: " + failReason);
        ShowDebug(img1, img2, cv::Mat());
        return false;
    }

    if (std::abs(median1 - median2) > m_thr_median) {
        failReason = "Median values differ too much.";
        Print("FAIL: " + failReason);
        ShowDebug(img1, img2, cv::Mat());
        return false;
    }

    // Difference map
    cv::Mat diff;
    cv::absdiff(img1, img2, diff);
    int badPixels = cv::countNonZero(diff > m_maxAbsDiff);
    Print("Bad pixels count = " + std::to_string(badPixels));

    if (badPixels >= m_minBadPixels) {
        failReason = "Too many differing pixels.";
        Print("FAIL: " + failReason);
        ShowDebug(img1, img2, diff);
        return false;
    }

    return true;
}

bool ImageComparisonExpert::CompareFolders(
    const std::string& ground_truth_folder,
    const std::string& results_folder,
    const std::string& token1, // e.g., "gt_"
    const std::string& token2, // e.g., "res"
    const std::string& ext1,   // e.g., "png"
    const std::string& ext2    // e.g., "tif"
) {
    m_gt_map.clear();
    m_res_map.clear();

    // Lambda to extract frame number from filename
    auto extract_frame_number = [](const std::string& filename, const std::string& token, const std::string& ext) -> std::optional<int> {
        if (filename.find(token) != 0 || filename.size() <= token.size() + ext.size() + 1)
            return std::nullopt;

        std::string core = filename.substr(token.size(), filename.size() - token.size() - ext.size() - 1);
        try {
            return std::stoi(core);
        } catch (...) {
            return std::nullopt;
        }
    };

    // Populate gt_map
    for (const auto& entry : std::filesystem::directory_iterator(ground_truth_folder)) {
        std::string fname = entry.path().filename().string();
        auto fnum = extract_frame_number(fname, token1, ext1);
        if (fnum) {
            m_gt_map[*fnum] = entry.path().string();
        }
    }

    // Populate res_map
    for (const auto& entry : std::filesystem::directory_iterator(results_folder)) {
        std::string fname = entry.path().filename().string();
        auto fnum = extract_frame_number(fname, token2, ext2);
        if (fnum) {
            m_res_map[*fnum] = entry.path().string();
        }
    }

    // Sort and compare
    int failed = 0;
    for (const auto& [frame, res_path] : m_res_map) {
        auto it = m_gt_map.find(frame);
        if (it != m_gt_map.end()) {
            const std::string& gt_path = it->second;
            std::cout << "Comparing frame " << frame << ": " << gt_path << " <-> " << res_path << std::endl;
            bool result = Compare(gt_path, res_path);
            std::cout << "Result: " << (result ? "PASS" : "FAIL") << std::endl;
            if(result == false){
                failed++;
            }
        } else {
            std::cout << "Warning: No ground truth found for frame " << frame << std::endl;
            failed++;
        }
    }
    std::cout << "ImageComparisonExpert::CompareFolders, Total failed frames count is  " << failed << " out of " << m_res_map.size()
    <<std::endl;
    return failed == 0;
}
