#ifndef ORBEXTRACTOR_ORBEXTRACTOR_H
#define ORBEXTRACTOR_ORBEXTRACTOR_H

#include <vector>
#include "include/Distribution.h"
#include "include/FAST.h"
#include "include/FeatureFileInterface.h"

#ifndef NDEBUG
#   define D(x) x
#else
#   define D(x)
#endif


namespace ORB_SLAM2
{

class ORBextractor
{
public:

    ORBextractor(int nfeatures, float scaleFactor, int nlevels,
                 int iniThFAST, int minThFAST);

    ~ORBextractor() = default;


    void operator()( cv::InputArray image, cv::InputArray mask,
                     std::vector<knuff::KeyPoint>& keypoints,
                     cv::OutputArray descriptors);

    void operator()(cv::InputArray inputImage, cv::InputArray mask,
                                  std::vector<knuff::KeyPoint> &resultKeypoints, cv::OutputArray outputDescriptors,
                                  bool distributePerLevel = true);

    int inline GetLevels(){
        return nlevels;}

    float inline GetScaleFactor(){
        return scaleFactor;}

    std::vector<float> inline GetScaleFactors(){
        return scaleFactorVec;
    }

    std::vector<float> inline GetInverseScaleFactors(){
        return invScaleFactorVec;
    }

    std::vector<float> inline GetScaleSigmaSquares(){
        return levelSigma2Vec;
    }

    std::vector<float> inline GetInverseScaleSigmaSquares(){
        return invLevelSigma2Vec;
    }

    void inline SetDistribution(Distribution::DistributionMethod mode)
    {
        kptDistribution = mode;
    }

    Distribution::DistributionMethod inline GetDistribution()
    {
        return kptDistribution;
    }

    void inline SetScoreType(FASTdetector::ScoreType s)
    {
        fast.SetScoreType(std::forward<FASTdetector::ScoreType >(s));
    }

    FASTdetector::ScoreType inline GetScoreType()
    {
        return fast.GetScoreType();
    }

    void inline SetLevelToDisplay(int lvl)
    {
        levelToDisplay = std::min(lvl, nlevels-1);
    }

    void inline SetSoftSSCThreshold(float th)
    {
        softSSCThreshold = th;
    }

    void SetnFeatures(int n);

    void SetFASTThresholds(int ini, int min);

    void SetnLevels(int n);

    void SetScaleFactor(float s);

    void SetFeatureSavePath(std::string &path)
    {
        path += std::to_string(nfeatures) + "f_" + std::to_string(scaleFactor) + "s_" +
                std::to_string(kptDistribution) + "d/";
        fileInterface.SetPath(path);
    }
    void inline SetFeatureSaving(bool s)
    {
        saveFeatures = s;
    }
    void inline SetLoadPath(std::string &path)
    {
        loadPath = path;
    }
    void inline EnablePrecomputedFeatures(bool b)
    {
        usePrecomputedFeatures = b;
    }
    inline FeatureFileInterface* GetFileInterface()
    {
        return &fileInterface;
    }
    inline std::vector<long> GetDistributionTimes()
    {
        return timeVector;
    }

    void SetSteps();

protected:

    static float IntensityCentroidAngle(const uchar* pointer, int step);


    void ComputeAngles(std::vector<std::vector<knuff::KeyPoint>> &allkpts);

    void ComputeDescriptors(std::vector<std::vector<knuff::KeyPoint>> &allkpts, cv::Mat &descriptors);


    void DivideAndFAST(std::vector<std::vector<knuff::KeyPoint> >& allKeypoints,
                       Distribution::DistributionMethod mode = Distribution::QUADTREE_ORBSLAMSTYLE,
                       bool divideImage = true, int cellSize = 30, bool distributePerLevel = true);

    void ComputeScalePyramid(cv::Mat &image);

    std::vector<cv::Point> pattern;

    std::vector<cv::Mat> imagePyramid;

    int nfeatures;
    double scaleFactor;
    int nlevels;
    int iniThFAST;
    int minThFAST;
    bool stepsChanged;

    int levelToDisplay;

    float softSSCThreshold;

    knuff::Point prevDims;

    Distribution::DistributionMethod kptDistribution;

    std::vector<int> pixelOffset;

    std::vector<int> nfeaturesPerLevelVec;


    std::vector<float> scaleFactorVec;
    std::vector<float> invScaleFactorVec;
    std::vector<float> levelSigma2Vec;
    std::vector<float> invLevelSigma2Vec;

    FASTdetector fast;

    FeatureFileInterface fileInterface;
    bool saveFeatures;
    bool usePrecomputedFeatures;
    std::string loadPath;

    std::vector <long> timeVector;
};


}

#endif //ORBEXTRACTOR_ORBEXTRACTOR_H