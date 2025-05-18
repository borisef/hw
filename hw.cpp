#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include "ImageComparisonExpert.h"
#include "TableComparisonExpert.h"


using namespace std;

int main()
{
   

    ImageComparisonExpert img_expert(5, 5, 2, 2, 100, 15, ImageComparisonExpert::DEBUG, true);

    bool result1 = img_expert.Compare("/home/borisef/projects/cpp/hw/gs1.png", "/home/borisef/projects/cpp/hw/gs3.png");
    std::cout << "Comparison result: " << (result1 ? "PASS" : "FAIL") << std::endl;
    
    bool result2 = img_expert.CompareFolders(
        "/home/borisef/projects/cpp/hw/gt_folder",
        "/home/borisef/projects/cpp/hw/res_folder",
        "gt_",
        "res_",
        "png",
        "png"
    );

    std::cout << "Folder comparison result: " << (result2 ? "PASS" : "FAIL") << std::endl;
    if (result2 == false) {
        std::cout << "Some images failed the comparison." << std::endl;
    } else {
        std::cout << "All images passed the comparison." << std::endl;
    }

    TableComparisonExpert table_expert;

    if (!table_expert.ReadCSVData("/home/borisef/projects/cpp/hw/gt_folder/gt.csv", 
        "/home/borisef/projects/cpp/hw/res_folder/res.csv", "FrameNum", "frame_id")) {
         std::cerr << "Failed to read CSV files." << std::endl;
         return 1;
     }

    bool result3 = table_expert.CompareAllRows("p1", "prob1", 0.2, "float", TableComparisonExpert::INFORMATIVE);
    std::cout << (result3 ? "All rows match." : "Discrepancies found.") << std::endl;

    bool result4 = table_expert.CompareAllRows("p5", "p1rob1", 0.0, "float", TableComparisonExpert::INFORMATIVE);
    std::cout << (result4 ? "All rows match." : "Discrepancies found.") << std::endl;

    return 0;
}