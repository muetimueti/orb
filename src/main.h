#ifndef ORBEXTRACTOR_MAIN_H
#define ORBEXTRACTOR_MAIN_H

#include <string>
#include <opencv2/core/core.hpp>
#include "ORBextractor.h"

#include <unistd.h>

#ifndef NDEBUG
#  define D(x) x
#else
# define D(x)
#endif

void DisplayKeypoints(cv::Mat &image, std::vector<cv::KeyPoint> &keypoints, cv::Scalar &color,
                      int thickness = 1, int radius = 8, int drawAngular = 0);

D(

void measureExecutionTime(int numIterations, ORB_SLAM2::ORBextractor &extractor, cv::Mat &image);
void AddRandomKeypoints(std::vector<cv::KeyPoint> &keypoints);
)

#endif //ORBEXTRACTOR_MAIN_H
