#include "include/ORBextractor.h"
#include "include/FAST.h"
#include <string>
#include <iostream>
#include <iterator>
#include <opencv2/imgproc/imgproc.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfor-loop-analysis"

#ifndef NDEBUG
#   define D(x) x
#   include <opencv2/highgui/highgui.hpp>
#   include <opencv2/features2d/features2d.hpp>
#   include <chrono>
#else
#   define D(x)
#endif



namespace ORB_SLAM2
{

const int PATCH_SIZE = 31;
const int EDGE_THRESHOLD = 19;

const int CIRCLE_SIZE = 16;
const int CIRCLE_OFFSETS[16][2] =
        {{0,  3}, { 1,  3}, { 2,  2}, { 3,  1}, { 3, 0}, { 3, -1}, { 2, -2}, { 1, -3},
         {0, -3}, {-1, -3}, {-2, -2}, {-3, -1}, {-3, 0}, {-3,  1}, {-2,  2}, {-1,  3}};
const int PIXELS_TO_CHECK[16] =
        {0, 8, 2, 10, 4, 12, 6, 14, 1, 9, 3, 11, 5, 13, 7, 15};
const int CIRCULAR_ROWS[16] = {15, 15, 15, 15, 14, 14, 14, 13, 13, 12, 11, 10, 9, 8, 6, 3};

static int bit_pattern_31_[256*4] =
        {
                8,-3, 9,5/*mean (0), correlation (0)*/,
                4,2, 7,-12/*mean (1.12461e-05), correlation (0.0437584)*/,
                -11,9, -8,2/*mean (3.37382e-05), correlation (0.0617409)*/,
                7,-12, 12,-13/*mean (5.62303e-05), correlation (0.0636977)*/,
                2,-13, 2,12/*mean (0.000134953), correlation (0.085099)*/,
                1,-7, 1,6/*mean (0.000528565), correlation (0.0857175)*/,
                -2,-10, -2,-4/*mean (0.0188821), correlation (0.0985774)*/,
                -13,-13, -11,-8/*mean (0.0363135), correlation (0.0899616)*/,
                -13,-3, -12,-9/*mean (0.121806), correlation (0.099849)*/,
                10,4, 11,9/*mean (0.122065), correlation (0.093285)*/,
                -13,-8, -8,-9/*mean (0.162787), correlation (0.0942748)*/,
                -11,7, -9,12/*mean (0.21561), correlation (0.0974438)*/,
                7,7, 12,6/*mean (0.160583), correlation (0.130064)*/,
                -4,-5, -3,0/*mean (0.228171), correlation (0.132998)*/,
                -13,2, -12,-3/*mean (0.00997526), correlation (0.145926)*/,
                -9,0, -7,5/*mean (0.198234), correlation (0.143636)*/,
                12,-6, 12,-1/*mean (0.0676226), correlation (0.16689)*/,
                -3,6, -2,12/*mean (0.166847), correlation (0.171682)*/,
                -6,-13, -4,-8/*mean (0.101215), correlation (0.179716)*/,
                11,-13, 12,-8/*mean (0.200641), correlation (0.192279)*/,
                4,7, 5,1/*mean (0.205106), correlation (0.186848)*/,
                5,-3, 10,-3/*mean (0.234908), correlation (0.192319)*/,
                3,-7, 6,12/*mean (0.0709964), correlation (0.210872)*/,
                -8,-7, -6,-2/*mean (0.0939834), correlation (0.212589)*/,
                -2,11, -1,-10/*mean (0.127778), correlation (0.20866)*/,
                -13,12, -8,10/*mean (0.14783), correlation (0.206356)*/,
                -7,3, -5,-3/*mean (0.182141), correlation (0.198942)*/,
                -4,2, -3,7/*mean (0.188237), correlation (0.21384)*/,
                -10,-12, -6,11/*mean (0.14865), correlation (0.23571)*/,
                5,-12, 6,-7/*mean (0.222312), correlation (0.23324)*/,
                5,-6, 7,-1/*mean (0.229082), correlation (0.23389)*/,
                1,0, 4,-5/*mean (0.241577), correlation (0.215286)*/,
                9,11, 11,-13/*mean (0.00338507), correlation (0.251373)*/,
                4,7, 4,12/*mean (0.131005), correlation (0.257622)*/,
                2,-1, 4,4/*mean (0.152755), correlation (0.255205)*/,
                -4,-12, -2,7/*mean (0.182771), correlation (0.244867)*/,
                -8,-5, -7,-10/*mean (0.186898), correlation (0.23901)*/,
                4,11, 9,12/*mean (0.226226), correlation (0.258255)*/,
                0,-8, 1,-13/*mean (0.0897886), correlation (0.274827)*/,
                -13,-2, -8,2/*mean (0.148774), correlation (0.28065)*/,
                -3,-2, -2,3/*mean (0.153048), correlation (0.283063)*/,
                -6,9, -4,-9/*mean (0.169523), correlation (0.278248)*/,
                8,12, 10,7/*mean (0.225337), correlation (0.282851)*/,
                0,9, 1,3/*mean (0.226687), correlation (0.278734)*/,
                7,-5, 11,-10/*mean (0.00693882), correlation (0.305161)*/,
                -13,-6, -11,0/*mean (0.0227283), correlation (0.300181)*/,
                10,7, 12,1/*mean (0.125517), correlation (0.31089)*/,
                -6,-3, -6,12/*mean (0.131748), correlation (0.312779)*/,
                10,-9, 12,-4/*mean (0.144827), correlation (0.292797)*/,
                -13,8, -8,-12/*mean (0.149202), correlation (0.308918)*/,
                -13,0, -8,-4/*mean (0.160909), correlation (0.310013)*/,
                3,3, 7,8/*mean (0.177755), correlation (0.309394)*/,
                5,7, 10,-7/*mean (0.212337), correlation (0.310315)*/,
                -1,7, 1,-12/*mean (0.214429), correlation (0.311933)*/,
                3,-10, 5,6/*mean (0.235807), correlation (0.313104)*/,
                2,-4, 3,-10/*mean (0.00494827), correlation (0.344948)*/,
                -13,0, -13,5/*mean (0.0549145), correlation (0.344675)*/,
                -13,-7, -12,12/*mean (0.103385), correlation (0.342715)*/,
                -13,3, -11,8/*mean (0.134222), correlation (0.322922)*/,
                -7,12, -4,7/*mean (0.153284), correlation (0.337061)*/,
                6,-10, 12,8/*mean (0.154881), correlation (0.329257)*/,
                -9,-1, -7,-6/*mean (0.200967), correlation (0.33312)*/,
                -2,-5, 0,12/*mean (0.201518), correlation (0.340635)*/,
                -12,5, -7,5/*mean (0.207805), correlation (0.335631)*/,
                3,-10, 8,-13/*mean (0.224438), correlation (0.34504)*/,
                -7,-7, -4,5/*mean (0.239361), correlation (0.338053)*/,
                -3,-2, -1,-7/*mean (0.240744), correlation (0.344322)*/,
                2,9, 5,-11/*mean (0.242949), correlation (0.34145)*/,
                -11,-13, -5,-13/*mean (0.244028), correlation (0.336861)*/,
                -1,6, 0,-1/*mean (0.247571), correlation (0.343684)*/,
                5,-3, 5,2/*mean (0.000697256), correlation (0.357265)*/,
                -4,-13, -4,12/*mean (0.00213675), correlation (0.373827)*/,
                -9,-6, -9,6/*mean (0.0126856), correlation (0.373938)*/,
                -12,-10, -8,-4/*mean (0.0152497), correlation (0.364237)*/,
                10,2, 12,-3/*mean (0.0299933), correlation (0.345292)*/,
                7,12, 12,12/*mean (0.0307242), correlation (0.366299)*/,
                -7,-13, -6,5/*mean (0.0534975), correlation (0.368357)*/,
                -4,9, -3,4/*mean (0.099865), correlation (0.372276)*/,
                7,-1, 12,2/*mean (0.117083), correlation (0.364529)*/,
                -7,6, -5,1/*mean (0.126125), correlation (0.369606)*/,
                -13,11, -12,5/*mean (0.130364), correlation (0.358502)*/,
                -3,7, -2,-6/*mean (0.131691), correlation (0.375531)*/,
                7,-8, 12,-7/*mean (0.160166), correlation (0.379508)*/,
                -13,-7, -11,-12/*mean (0.167848), correlation (0.353343)*/,
                1,-3, 12,12/*mean (0.183378), correlation (0.371916)*/,
                2,-6, 3,0/*mean (0.228711), correlation (0.371761)*/,
                -4,3, -2,-13/*mean (0.247211), correlation (0.364063)*/,
                -1,-13, 1,9/*mean (0.249325), correlation (0.378139)*/,
                7,1, 8,-6/*mean (0.000652272), correlation (0.411682)*/,
                1,-1, 3,12/*mean (0.00248538), correlation (0.392988)*/,
                9,1, 12,6/*mean (0.0206815), correlation (0.386106)*/,
                -1,-9, -1,3/*mean (0.0364485), correlation (0.410752)*/,
                -13,-13, -10,5/*mean (0.0376068), correlation (0.398374)*/,
                7,7, 10,12/*mean (0.0424202), correlation (0.405663)*/,
                12,-5, 12,9/*mean (0.0942645), correlation (0.410422)*/,
                6,3, 7,11/*mean (0.1074), correlation (0.413224)*/,
                5,-13, 6,10/*mean (0.109256), correlation (0.408646)*/,
                2,-12, 2,3/*mean (0.131691), correlation (0.416076)*/,
                3,8, 4,-6/*mean (0.165081), correlation (0.417569)*/,
                2,6, 12,-13/*mean (0.171874), correlation (0.408471)*/,
                9,-12, 10,3/*mean (0.175146), correlation (0.41296)*/,
                -8,4, -7,9/*mean (0.183682), correlation (0.402956)*/,
                -11,12, -4,-6/*mean (0.184672), correlation (0.416125)*/,
                1,12, 2,-8/*mean (0.191487), correlation (0.386696)*/,
                6,-9, 7,-4/*mean (0.192668), correlation (0.394771)*/,
                2,3, 3,-2/*mean (0.200157), correlation (0.408303)*/,
                6,3, 11,0/*mean (0.204588), correlation (0.411762)*/,
                3,-3, 8,-8/*mean (0.205904), correlation (0.416294)*/,
                7,8, 9,3/*mean (0.213237), correlation (0.409306)*/,
                -11,-5, -6,-4/*mean (0.243444), correlation (0.395069)*/,
                -10,11, -5,10/*mean (0.247672), correlation (0.413392)*/,
                -5,-8, -3,12/*mean (0.24774), correlation (0.411416)*/,
                -10,5, -9,0/*mean (0.00213675), correlation (0.454003)*/,
                8,-1, 12,-6/*mean (0.0293635), correlation (0.455368)*/,
                4,-6, 6,-11/*mean (0.0404971), correlation (0.457393)*/,
                -10,12, -8,7/*mean (0.0481107), correlation (0.448364)*/,
                4,-2, 6,7/*mean (0.050641), correlation (0.455019)*/,
                -2,0, -2,12/*mean (0.0525978), correlation (0.44338)*/,
                -5,-8, -5,2/*mean (0.0629667), correlation (0.457096)*/,
                7,-6, 10,12/*mean (0.0653846), correlation (0.445623)*/,
                -9,-13, -8,-8/*mean (0.0858749), correlation (0.449789)*/,
                -5,-13, -5,-2/*mean (0.122402), correlation (0.450201)*/,
                8,-8, 9,-13/*mean (0.125416), correlation (0.453224)*/,
                -9,-11, -9,0/*mean (0.130128), correlation (0.458724)*/,
                1,-8, 1,-2/*mean (0.132467), correlation (0.440133)*/,
                7,-4, 9,1/*mean (0.132692), correlation (0.454)*/,
                -2,1, -1,-4/*mean (0.135695), correlation (0.455739)*/,
                11,-6, 12,-11/*mean (0.142904), correlation (0.446114)*/,
                -12,-9, -6,4/*mean (0.146165), correlation (0.451473)*/,
                3,7, 7,12/*mean (0.147627), correlation (0.456643)*/,
                5,5, 10,8/*mean (0.152901), correlation (0.455036)*/,
                0,-4, 2,8/*mean (0.167083), correlation (0.459315)*/,
                -9,12, -5,-13/*mean (0.173234), correlation (0.454706)*/,
                0,7, 2,12/*mean (0.18312), correlation (0.433855)*/,
                -1,2, 1,7/*mean (0.185504), correlation (0.443838)*/,
                5,11, 7,-9/*mean (0.185706), correlation (0.451123)*/,
                3,5, 6,-8/*mean (0.188968), correlation (0.455808)*/,
                -13,-4, -8,9/*mean (0.191667), correlation (0.459128)*/,
                -5,9, -3,-3/*mean (0.193196), correlation (0.458364)*/,
                -4,-7, -3,-12/*mean (0.196536), correlation (0.455782)*/,
                6,5, 8,0/*mean (0.1972), correlation (0.450481)*/,
                -7,6, -6,12/*mean (0.199438), correlation (0.458156)*/,
                -13,6, -5,-2/*mean (0.211224), correlation (0.449548)*/,
                1,-10, 3,10/*mean (0.211718), correlation (0.440606)*/,
                4,1, 8,-4/*mean (0.213034), correlation (0.443177)*/,
                -2,-2, 2,-13/*mean (0.234334), correlation (0.455304)*/,
                2,-12, 12,12/*mean (0.235684), correlation (0.443436)*/,
                -2,-13, 0,-6/*mean (0.237674), correlation (0.452525)*/,
                4,1, 9,3/*mean (0.23962), correlation (0.444824)*/,
                -6,-10, -3,-5/*mean (0.248459), correlation (0.439621)*/,
                -3,-13, -1,1/*mean (0.249505), correlation (0.456666)*/,
                7,5, 12,-11/*mean (0.00119208), correlation (0.495466)*/,
                4,-2, 5,-7/*mean (0.00372245), correlation (0.484214)*/,
                -13,9, -9,-5/*mean (0.00741116), correlation (0.499854)*/,
                7,1, 8,6/*mean (0.0208952), correlation (0.499773)*/,
                7,-8, 7,6/*mean (0.0220085), correlation (0.501609)*/,
                -7,-4, -7,1/*mean (0.0233806), correlation (0.496568)*/,
                -8,11, -7,-8/*mean (0.0236505), correlation (0.489719)*/,
                -13,6, -12,-8/*mean (0.0268781), correlation (0.503487)*/,
                2,4, 3,9/*mean (0.0323324), correlation (0.501938)*/,
                10,-5, 12,3/*mean (0.0399235), correlation (0.494029)*/,
                -6,-5, -6,7/*mean (0.0420153), correlation (0.486579)*/,
                8,-3, 9,-8/*mean (0.0548021), correlation (0.484237)*/,
                2,-12, 2,8/*mean (0.0616622), correlation (0.496642)*/,
                -11,-2, -10,3/*mean (0.0627755), correlation (0.498563)*/,
                -12,-13, -7,-9/*mean (0.0829622), correlation (0.495491)*/,
                -11,0, -10,-5/*mean (0.0843342), correlation (0.487146)*/,
                5,-3, 11,8/*mean (0.0929937), correlation (0.502315)*/,
                -2,-13, -1,12/*mean (0.113327), correlation (0.48941)*/,
                -1,-8, 0,9/*mean (0.132119), correlation (0.467268)*/,
                -13,-11, -12,-5/*mean (0.136269), correlation (0.498771)*/,
                -10,-2, -10,11/*mean (0.142173), correlation (0.498714)*/,
                -3,9, -2,-13/*mean (0.144141), correlation (0.491973)*/,
                2,-3, 3,2/*mean (0.14892), correlation (0.500782)*/,
                -9,-13, -4,0/*mean (0.150371), correlation (0.498211)*/,
                -4,6, -3,-10/*mean (0.152159), correlation (0.495547)*/,
                -4,12, -2,-7/*mean (0.156152), correlation (0.496925)*/,
                -6,-11, -4,9/*mean (0.15749), correlation (0.499222)*/,
                6,-3, 6,11/*mean (0.159211), correlation (0.503821)*/,
                -13,11, -5,5/*mean (0.162427), correlation (0.501907)*/,
                11,11, 12,6/*mean (0.16652), correlation (0.497632)*/,
                7,-5, 12,-2/*mean (0.169141), correlation (0.484474)*/,
                -1,12, 0,7/*mean (0.169456), correlation (0.495339)*/,
                -4,-8, -3,-2/*mean (0.171457), correlation (0.487251)*/,
                -7,1, -6,7/*mean (0.175), correlation (0.500024)*/,
                -13,-12, -8,-13/*mean (0.175866), correlation (0.497523)*/,
                -7,-2, -6,-8/*mean (0.178273), correlation (0.501854)*/,
                -8,5, -6,-9/*mean (0.181107), correlation (0.494888)*/,
                -5,-1, -4,5/*mean (0.190227), correlation (0.482557)*/,
                -13,7, -8,10/*mean (0.196739), correlation (0.496503)*/,
                1,5, 5,-13/*mean (0.19973), correlation (0.499759)*/,
                1,0, 10,-13/*mean (0.204465), correlation (0.49873)*/,
                9,12, 10,-1/*mean (0.209334), correlation (0.49063)*/,
                5,-8, 10,-9/*mean (0.211134), correlation (0.503011)*/,
                -1,11, 1,-13/*mean (0.212), correlation (0.499414)*/,
                -9,-3, -6,2/*mean (0.212168), correlation (0.480739)*/,
                -1,-10, 1,12/*mean (0.212731), correlation (0.502523)*/,
                -13,1, -8,-10/*mean (0.21327), correlation (0.489786)*/,
                8,-11, 10,-6/*mean (0.214159), correlation (0.488246)*/,
                2,-13, 3,-6/*mean (0.216993), correlation (0.50287)*/,
                7,-13, 12,-9/*mean (0.223639), correlation (0.470502)*/,
                -10,-10, -5,-7/*mean (0.224089), correlation (0.500852)*/,
                -10,-8, -8,-13/*mean (0.228666), correlation (0.502629)*/,
                4,-6, 8,5/*mean (0.22906), correlation (0.498305)*/,
                3,12, 8,-13/*mean (0.233378), correlation (0.503825)*/,
                -4,2, -3,-3/*mean (0.234323), correlation (0.476692)*/,
                5,-13, 10,-12/*mean (0.236392), correlation (0.475462)*/,
                4,-13, 5,-1/*mean (0.236842), correlation (0.504132)*/,
                -9,9, -4,3/*mean (0.236977), correlation (0.497739)*/,
                0,3, 3,-9/*mean (0.24314), correlation (0.499398)*/,
                -12,1, -6,1/*mean (0.243297), correlation (0.489447)*/,
                3,2, 4,-8/*mean (0.00155196), correlation (0.553496)*/,
                -10,-10, -10,9/*mean (0.00239541), correlation (0.54297)*/,
                8,-13, 12,12/*mean (0.0034413), correlation (0.544361)*/,
                -8,-12, -6,-5/*mean (0.003565), correlation (0.551225)*/,
                2,2, 3,7/*mean (0.00835583), correlation (0.55285)*/,
                10,6, 11,-8/*mean (0.00885065), correlation (0.540913)*/,
                6,8, 8,-12/*mean (0.0101552), correlation (0.551085)*/,
                -7,10, -6,5/*mean (0.0102227), correlation (0.533635)*/,
                -3,-9, -3,9/*mean (0.0110211), correlation (0.543121)*/,
                -1,-13, -1,5/*mean (0.0113473), correlation (0.550173)*/,
                -3,-7, -3,4/*mean (0.0140913), correlation (0.554774)*/,
                -8,-2, -8,3/*mean (0.017049), correlation (0.55461)*/,
                4,2, 12,12/*mean (0.01778), correlation (0.546921)*/,
                2,-5, 3,11/*mean (0.0224022), correlation (0.549667)*/,
                6,-9, 11,-13/*mean (0.029161), correlation (0.546295)*/,
                3,-1, 7,12/*mean (0.0303081), correlation (0.548599)*/,
                11,-1, 12,4/*mean (0.0355151), correlation (0.523943)*/,
                -3,0, -3,6/*mean (0.0417904), correlation (0.543395)*/,
                4,-11, 4,12/*mean (0.0487292), correlation (0.542818)*/,
                2,-4, 2,1/*mean (0.0575124), correlation (0.554888)*/,
                -10,-6, -8,1/*mean (0.0594242), correlation (0.544026)*/,
                -13,7, -11,1/*mean (0.0597391), correlation (0.550524)*/,
                -13,12, -11,-13/*mean (0.0608974), correlation (0.55383)*/,
                6,0, 11,-13/*mean (0.065126), correlation (0.552006)*/,
                0,-1, 1,4/*mean (0.074224), correlation (0.546372)*/,
                -13,3, -9,-2/*mean (0.0808592), correlation (0.554875)*/,
                -9,8, -6,-3/*mean (0.0883378), correlation (0.551178)*/,
                -13,-6, -8,-2/*mean (0.0901035), correlation (0.548446)*/,
                5,-9, 8,10/*mean (0.0949843), correlation (0.554694)*/,
                2,7, 3,-9/*mean (0.0994152), correlation (0.550979)*/,
                -1,-6, -1,-1/*mean (0.10045), correlation (0.552714)*/,
                9,5, 11,-2/*mean (0.100686), correlation (0.552594)*/,
                11,-3, 12,-8/*mean (0.101091), correlation (0.532394)*/,
                3,0, 3,5/*mean (0.101147), correlation (0.525576)*/,
                -1,4, 0,10/*mean (0.105263), correlation (0.531498)*/,
                3,-6, 4,5/*mean (0.110785), correlation (0.540491)*/,
                -13,0, -10,5/*mean (0.112798), correlation (0.536582)*/,
                5,8, 12,11/*mean (0.114181), correlation (0.555793)*/,
                8,9, 9,-6/*mean (0.117431), correlation (0.553763)*/,
                7,-4, 8,-12/*mean (0.118522), correlation (0.553452)*/,
                -10,4, -10,9/*mean (0.12094), correlation (0.554785)*/,
                7,3, 12,4/*mean (0.122582), correlation (0.555825)*/,
                9,-7, 10,-2/*mean (0.124978), correlation (0.549846)*/,
                7,0, 12,-2/*mean (0.127002), correlation (0.537452)*/,
                -1,-6, 0,-11/*mean (0.127148), correlation (0.547401)*/
        };

float ORBextractor::IntensityCentroidAngle(const uchar* pointer, int step)
{
    //m10 ~ x^1y^0, m01 ~ x^0y^1
    int x, y, m01 = 0, m10 = 0;

    int half_patch = PATCH_SIZE / 2;

    for (x = -half_patch; x <= half_patch; ++x)
    {
        m10 += x * pointer[x];
    }

    for (y = 1; y <= half_patch; ++y)
    {
        int cols = CIRCULAR_ROWS[y];
        int sumY = 0;
        for (x = -cols; x <= cols; ++x)
        {
            int uptown = pointer[x + y*step];
            int downtown = pointer[x - y*step];
            sumY += uptown - downtown;
            m10 += x * (uptown + downtown);
        }
        m01 += y * sumY;
    }

    return cv::fastAtan2((float)m01, (float)m10);
}


bool ORBextractor::ResponseComparison(const cv::KeyPoint &k1, const cv::KeyPoint &k2)
{
    return (k1.response > k2.response);
}
void ORBextractor::RetainBestN(std::vector<cv::KeyPoint> &kpts, int N)
{
    if (N >= kpts.size())
        return;

    std::sort(kpts.begin(), kpts.end(), ResponseComparison);
    kpts.resize(N);

    //std::vector<cv::KeyPoint> tempKpts;
    //tempKpts.reserve(N);

    /*
    for (int i = 0; i < N; ++i)
    {
        tempKpts.emplace_back(kpts[i]);
    }
     */
    //kpts = tempKpts;
}




ORBextractor::ORBextractor(int _nfeatures, float _scaleFactor, int _nlevels,
                           int _iniThFAST, int _minThFAST):
        nfeatures(_nfeatures), scaleFactor(_scaleFactor), nlevels(_nlevels),
        iniThFAST(_iniThFAST), minThFAST(_minThFAST), threshold_tab_min{}, threshold_tab_init{}, pixelOffset{}

{

    scaleFactorVec.resize(nlevels);
    invScaleFactorVec.resize(nlevels);
    imagePyramid.resize(nlevels);
    nfeaturesPerLevelVec.resize(nlevels);
    pixelOffset.resize(nlevels * CIRCLE_SIZE);

    //TODO: superfluous once FAST is separated
    continuousPixelsRequired = CIRCLE_SIZE / 2;
    onePointFiveCircles = CIRCLE_SIZE + continuousPixelsRequired + 1;

    iniThFAST = std::min(255, std::max(0, iniThFAST));
    minThFAST = std::min(iniThFAST, std::max(0, minThFAST));

    //initialize threshold tabs for init and min threshold
    int i;
    for (i = 0; i < 512; ++i)
    {
        int v = i - 255;
        if (v < -iniThFAST)
        {
            threshold_tab_init[i] = (uchar)1;
            threshold_tab_min[i] = (uchar)1;
        } else if (v > iniThFAST)
        {
            threshold_tab_init[i] = (uchar)2;
            threshold_tab_min[i] = (uchar)2;

        } else
        {
            threshold_tab_init[i] = (uchar)0;
            if (v < -minThFAST)
            {
                threshold_tab_min[i] = (uchar)1;
            } else if (v > minThFAST)
            {
                threshold_tab_min[i] = (uchar)2;
            } else
                threshold_tab_min[i] = (uchar)0;
        }
    }
    /*
    for (i = -255; i <= 255; ++i)
    {
        threshold_tab_init[i+255] = (uchar)(i < -iniThFAST ? 1 : i > iniThFAST ? 2 : 0);
        threshold_tab_min[i+255] = (uchar)(i < -minThFAST ? 1 : i > minThFAST ? 2 : 0);
    }
     */


    scaleFactorVec[0] = 1.f;
    invScaleFactorVec[0] = 1.f;

    for (i = 1; i < nlevels; ++i) {
        scaleFactorVec[i] = scaleFactor * scaleFactorVec[i - 1];
        invScaleFactorVec[i] = 1 / scaleFactorVec[i];
    }


    float fac = 1.f / scaleFactor;
    float nDesiredFeaturesPerScale = nfeatures * (1.f - fac) / (1.f - (float) pow((double) fac, (double) nlevels));

    int sumFeatures = 0;
    for (i = 0; i < nlevels - 1; ++i)
    {
        nfeaturesPerLevelVec[i] = cvRound(nDesiredFeaturesPerScale);
        sumFeatures += nfeaturesPerLevelVec[i];
        nDesiredFeaturesPerScale *= fac;
    }
    nfeaturesPerLevelVec[nlevels-1] = std::max(nfeatures - sumFeatures, 0);

    const int nPoints = 512;
    const auto tempPattern = (const cv::Point*) bit_pattern_31_;
    std::copy(tempPattern, tempPattern+nPoints, std::back_inserter(pattern));
}


void ORBextractor::operator()(cv::InputArray inputImage, cv::InputArray mask,
                              std::vector<cv::KeyPoint> &resultKeypoints, cv::OutputArray outputDescriptors)
{
    if (inputImage.empty())
        return;

    cv::Mat image = inputImage.getMat();
    assert(image.type() == CV_8UC1);

    ComputeScalePyramid(image);


    for (int lvl = 0; lvl < nlevels; ++lvl)
    {
        for (int i = 0; i < CIRCLE_SIZE; ++i)
        {
            pixelOffset[lvl*CIRCLE_SIZE + i] =
                    CIRCLE_OFFSETS[i][0] + CIRCLE_OFFSETS[i][1] * (int)imagePyramid[lvl].step1();
        }
    }


    std::vector<std::vector<cv::KeyPoint>> allKpts;
    DivideAndFAST(allKpts, false, false);

    ComputeAngles(allKpts);

    cv::Mat BRIEFdescriptors;
    int nkpts = 0;
    for (int lvl = 0; lvl < nlevels; ++lvl)
    {
        nkpts += (int)allKpts[lvl].size();
    }
    if (nkpts <= 0)
    {
        outputDescriptors.release();
    } else
    {
        outputDescriptors.create(nkpts, 32, CV_8U);
        BRIEFdescriptors = outputDescriptors.getMat();
    }

    resultKeypoints.clear();
    resultKeypoints.reserve(nkpts);

    ComputeDescriptors(allKpts, BRIEFdescriptors);

    for (int lvl = 1; lvl < nlevels; ++lvl)
    {

        float scale = scaleFactorVec[lvl];
        for (auto &kpt : allKpts[lvl])
        {
            kpt.pt *= scale;

        }
    }
    //TODO: scale kpts size

    for (int lvl = 0; lvl < nlevels; ++lvl)
    {
        resultKeypoints.insert(resultKeypoints.end(), allKpts[lvl].begin(), allKpts[lvl].end());
    }
}


void ORBextractor::ComputeAngles(std::vector<std::vector<cv::KeyPoint>> &allkpts)
{
    for (int lvl = 0; lvl < nlevels; ++lvl)
    {
        for (auto &kpt : allkpts[lvl])
        {
            kpt.angle = IntensityCentroidAngle(&imagePyramid[lvl].at<uchar>(cvRound(kpt.pt.y), cvRound(kpt.pt.x)),
                                               imagePyramid[lvl].step1());
        }
    }
}


void ORBextractor::ComputeDescriptors(std::vector<std::vector<cv::KeyPoint>> &allkpts, cv::Mat &descriptors)
{
    const auto degToRadFactor = (float)(CV_PI/180.f);
    const cv::Point* p = &pattern[0];

    int current = 0;

    for (int lvl = 0; lvl < nlevels; ++lvl)
    {
        cv::Mat lvlClone = imagePyramid[lvl].clone();
        cv::GaussianBlur(lvlClone, lvlClone, cv::Size(7, 7), 2, 2, cv::BORDER_REFLECT_101);

        const int step = (int)lvlClone.step;


        int i = 0, nkpts = allkpts[lvl].size();
        for (int k = 0; k < nkpts; ++k, ++current)
        {
            const cv::KeyPoint &kpt = allkpts[lvl][k];
            auto descPointer = descriptors.ptr<uchar>(current);        //ptr to beginning of current descriptor
            const uchar* pixelPointer = &lvlClone.at<uchar>(cvRound(kpt.pt.y), cvRound(kpt.pt.x));  //ptr to kpt in img

            float angleRad = kpt.angle * degToRadFactor;
            auto a = (float)cos(angleRad), b = (float)sin(angleRad);

            int byte = 0, v0, v1, idx0, idx1;
            for (i = 0; i <= 512; i+=2)
            {
                if (i > 0 && i%16 == 0) //working byte full
                {
                    descPointer[i/16 - 1] = (uchar)byte;  //write current byte to descriptor-mat
                    byte = 0;      //reset working byte
                    if (i == 512)  //break out after writing very last byte, so oob indices aren't accessed
                        break;
                }

                idx0 = cvRound(p[i].x*a - p[i].y*b) + cvRound(p[i].x*b + p[i].y*a)*step;
                idx1 = cvRound(p[i+1].x*a - p[i+1].y*b) + cvRound(p[i+1].x*b + p[i+1].y*a)*step;

                v0 = pixelPointer[idx0];
                v1 = pixelPointer[idx1];

                byte |= (v0 < v1) << ((i%16)/2); //write comparison bit to current byte
            }
        }
    }
}


/**
 * @param distributeKpts: true-->Kpts distributed over image via quadtree
 *                        false-->best N kpts retained by response (per octave)
 * @param divideImage:  true-->divide image into cellSize x cellSize cells, run FAST per cell
 *                      false-->run FAST over whole image
 * @param cellSize: 16 < cellSize < (min(rows, cols) of lowest level image)
 */
void ORBextractor::DivideAndFAST(std::vector<std::vector<cv::KeyPoint>> &allKeypoints,
        bool distributeKpts, bool divideImage, int cellSize)
{
    allKeypoints.resize(nlevels);

    const int minimumX = EDGE_THRESHOLD - 3, minimumY = minimumX;

    if (!divideImage)
    {
        for (int lvl = 0; lvl < nlevels; ++lvl)
        {
            std::vector<cv::KeyPoint> levelKpts;
            levelKpts.clear();
            levelKpts.reserve(nfeatures*10);

            const int maximumX = imagePyramid[lvl].cols - EDGE_THRESHOLD + 3;
            const int maximumY = imagePyramid[lvl].rows - EDGE_THRESHOLD + 3;

            //std::cout << "lvl " << lvl << ": minX=" << minimumX << ", maxX=" << maximumX <<
            //   ", minY=" << minimumY << ", maxY=" << maximumY << "\n";

            cv::Range colSelect(minimumX, maximumX);
            cv::Range rowSelect(minimumY, maximumY);
            cv::Mat levelMat = imagePyramid[lvl](rowSelect, colSelect);

            this->FAST(levelMat, levelKpts, iniThFAST, lvl);

            if (levelKpts.empty())
                this->FAST(levelMat, levelKpts, minThFAST, lvl);

            if(levelKpts.empty())
                continue;

            allKeypoints[lvl].reserve(nfeaturesPerLevelVec[lvl]);

            if (distributeKpts)
            {
                DistributeKeypoints(levelKpts, minimumX, maximumX, minimumY, maximumY, lvl);
            } else
            {
                RetainBestN(levelKpts, nfeaturesPerLevelVec[lvl]);
            }

            allKeypoints[lvl] = levelKpts;

            for (auto &kpt : allKeypoints[lvl])
            {
                kpt.pt.y += minimumY;
                kpt.pt.x += minimumX;
                //kpt.angle = IntensityCentroidAngle(&imagePyramid[lvl].at<uchar>(
                //        cvRound(kpt.pt.x), cvRound(kpt.pt.y)), imagePyramid[lvl].step1());
            }
        }
    }
    else
    {
        int c = std::min(imagePyramid[nlevels-1].rows, imagePyramid[nlevels-1].cols);
        assert(cellSize < c && cellSize > 16);
        for (int lvl = 0; lvl < nlevels; ++lvl)
        {
            std::vector<cv::KeyPoint> levelKpts;
            levelKpts.clear();
            levelKpts.reserve(nfeatures*10);

            const int maximumX = imagePyramid[lvl].cols - EDGE_THRESHOLD + 3;
            const int maximumY = imagePyramid[lvl].rows - EDGE_THRESHOLD + 3;
            const int width = maximumX - minimumX;
            const int height = maximumY - minimumY;

            const int npatchesInX = width / cellSize;
            const int npatchesInY = height / cellSize;
            const int patchWidth = std::ceil(width / npatchesInX);
            const int patchHeight = std::ceil(height / npatchesInY);


            for (int py = 0; py < npatchesInY; ++py)
            {
                int startY = minimumY + py * patchHeight;
                int endY = startY + patchHeight + 6;

                if (startY >= maximumY)
                    continue;

                if (endY > maximumY)
                    endY = maximumY;


                for (int px = 0; px < npatchesInX; ++px)
                {
                    int startX = minimumX + px * patchWidth;
                    int endX = startX + patchWidth + 6;


                    if (startX >= maximumX)
                        continue;

                    if (endX > maximumX)
                        endX = maximumX;

                    cv::Range colSelect(startX, endX);
                    cv::Range rowSelect(startY, endY);
                    cv::Mat patch = imagePyramid[lvl](rowSelect, colSelect);
                    std::vector<cv::KeyPoint> patchKpts;

                    fast(patch, patchKpts, iniThFAST, lvl);

                    if (patchKpts.empty())
                        this->FAST(patch, patchKpts, minThFAST, lvl);

                    if(patchKpts.empty())
                        continue;

                    for (auto &kpt : patchKpts)
                    {
                        kpt.pt.y += py * patchHeight;
                        kpt.pt.x += px * patchWidth;
                        levelKpts.emplace_back(kpt);
                    }
                }
            }

            allKeypoints[lvl].reserve(nfeatures);

            if (distributeKpts)
            {
                DistributeKeypoints(levelKpts, minimumX, maximumX, minimumY, maximumY, lvl);
            } else
            {
                RetainBestN(levelKpts, nfeaturesPerLevelVec[lvl]);
            }

            allKeypoints[lvl] = levelKpts;



            for (auto &kpt : allKeypoints[lvl])
            {
                kpt.pt.y += minimumY;
                kpt.pt.x += minimumX;
                //kpt.angle = IntensityCentroidAngle(&imagePyramid[lvl].at<uchar>(
                //        cvRound(kpt.pt.x), cvRound(kpt.pt.y)), imagePyramid[lvl].step1());
            }
        }
    }
}


void ORBextractor::DistributeKeypoints(std::vector<cv::KeyPoint>& kpts, const int &minX,
                                                            const int &maxX, const int &minY, const int &maxY, const int &lvl)
{
    const int nroots = cvRound((maxX-minX)/(maxY-minY));

    int N = nfeaturesPerLevelVec[lvl];
    int nodeWidth = cvRound((float)(maxX - minX) / nroots);

    std::list<ExtractorNode> nodesList;

    std::vector<ExtractorNode*> rootVec;
    rootVec.resize(nroots);

    for (int i = 0; i < nroots; ++i)
    {
        int x0 = minX + nodeWidth * i;
        int x1 = minX + nodeWidth * (i+1);
        int y0 = minY;
        int y1 = maxY;
        ExtractorNode n;
        n.UL = cv::Point2i(x0, y0);
        n.UR = cv::Point2i(x1, y0);
        n.LL = cv::Point2i(x0, y1);
        n.LR = cv::Point2i(x1, y1);
        n.nodeKpts.reserve(kpts.size());

        nodesList.push_back(n);
        rootVec[i] = &nodesList.back();
    }

    for (auto &kpt : kpts)
    {
        rootVec[(int)(kpt.pt.x / nodeWidth)]->nodeKpts.emplace_back(kpt);
    }

    /*
    //debug TODO:remove
    std::cout << "\nnodes:\n";
    for (int i = 0; i < nroots; ++i)
    {
        std::cout << "kpts in node " << i << ", with boundaries (" << rootVec[i].UL << ", " << rootVec[i].UR <<
            ", " << rootVec[i].LL << ", " << rootVec[i].LR << "):\n";
        for (auto &kpt : rootVec[i].nodeKpts)
        {
            std::cout << "kpt " << kpt.pt << "\n";
        }
    }
    */

    auto current = nodesList.begin();

    bool omegadoom = false;
    int lastSize = 0;
    while (!omegadoom)
    {
        current = nodesList.begin();
        lastSize = nodesList.size();

        while (current != nodesList.end())
        {
            if (current->nodeKpts.empty())
            {
                current = nodesList.erase(current);
            }

            if (current->leaf)
            {
                ++current;
                continue;
            }

            ExtractorNode n1, n2, n3, n4;
            current->DivideNode(n1, n2, n3, n4);
            if (!n1.nodeKpts.empty())
            {
                nodesList.push_front(n1);
                if (n1.nodeKpts.size() == 1)
                    n1.leaf = true;
            }
            if (!n2.nodeKpts.empty())
            {
                nodesList.push_front(n2);
                if (n2.nodeKpts.size() == 1)
                    n2.leaf = true;
            }
            if (!n3.nodeKpts.empty())
            {
                nodesList.push_front(n3);
                if (n3.nodeKpts.size() == 1)
                    n3.leaf = true;
            }
            if (!n4.nodeKpts.empty())
            {
                nodesList.push_front(n4);
                if (n4.nodeKpts.size() == 1)
                    n4.leaf = true;
            }

            current = nodesList.erase(current);

            if (nodesList.size() == lastSize)
            {
                omegadoom = true;
                break;
            }

            if (nodesList.size() >= N)
            {
                omegadoom = true;
                break;
            }
        }
    }

    std::vector<cv::KeyPoint> resKpts;
    resKpts.reserve(N);
    for (auto &node : nodesList)
    {
        RetainBestN(node.nodeKpts, 1);
        resKpts.emplace_back(node.nodeKpts[0]);
    }
    kpts = resKpts;
    //TODO: refine implementation?
}


void ExtractorNode::DivideNode(ORB_SLAM2::ExtractorNode &n1, ORB_SLAM2::ExtractorNode &n2, ORB_SLAM2::ExtractorNode &n3,
                               ORB_SLAM2::ExtractorNode &n4)
{
    int middleX = UL.x + (int)std::ceil((float)(UR.x - UL.x)/2.f);
    int middleY = UL.y + (int)std::ceil((float)(LL.y - UL.y)/2.f);

    cv::Point2i M (middleX, middleY);
    cv::Point2i upperM (middleX, UL.y);
    cv::Point2i lowerM (middleX, LL.y);
    cv::Point2i leftM (UL.x, middleY);
    cv::Point2i rightM (UR.x, middleY);

    n1.UL = UL, n1.UR = upperM, n1.LL = leftM, n1.LR = M;
    n2.UL = upperM, n2.UR = UR, n2.LL = M, n2.LR = rightM;
    n3.UL = leftM, n3.UR = M, n3.LL = LL, n3.LR = lowerM;
    n4.UL = M, n4.UR = rightM, n4.LL = lowerM, n4.LR = LR;

    for (auto &kpt : nodeKpts)
    {
        if (kpt.pt.x < middleX)
        {
            if(kpt.pt.y < middleY)
                n1.nodeKpts.emplace_back(kpt);
            else
                n3.nodeKpts.emplace_back(kpt);

        }
        else
        {
            if (kpt.pt.y < middleY)
                n2.nodeKpts.emplace_back(kpt);
            else
                n4.nodeKpts.emplace_back(kpt);
        }
    }
}



/*
//TODO: move to separate file
void ORBextractor::FAST(cv::Mat &img, std::vector<cv::KeyPoint> &keypoints, int threshold, int level)
{
    keypoints.clear();

    int offset[CIRCLE_SIZE];
    for (int i = 0; i < CIRCLE_SIZE; ++i)
    {
        offset[i] = pixelOffset[level*CIRCLE_SIZE + i];
    }

    if (!(threshold == minThFAST || threshold == iniThFAST))
        //only initial or min threshold should be passed
        return;

    uchar *threshold_tab;
    if (threshold == iniThFAST)
        threshold_tab = threshold_tab_init;
    else
        threshold_tab = threshold_tab_min;


    uchar cornerScores[img.cols*3];
    int cornerPos[img.cols*3];

    memset(cornerScores, 0, img.cols*3);
    memset(cornerPos, 0, img.cols*3);

    uchar* currRowScores = &cornerScores[0];
    uchar* prevRowScores = &cornerScores[img.cols];
    uchar* pprevRowScores = &cornerScores[img.cols*2];

    int* currRowPos = &cornerPos[0];
    int* prevRowPos = &cornerPos[img.cols];
    int* pprevRowPos = &cornerPos[img.cols*2];


    int i, j, k, ncandidates = 0, ncandidatesprev = 0;

    for (i = 3; i < img.rows-2; ++i)
    {
        const uchar* pointer = img.ptr<uchar>(i) + 3;

        ncandidatesprev = ncandidates;
        ncandidates = 0;

        int* tempPos = pprevRowPos;
        uchar* tempScores = pprevRowScores;

        pprevRowPos = prevRowPos;
        pprevRowScores = prevRowScores;
        prevRowPos = currRowPos;
        prevRowScores = currRowScores;

        currRowPos = tempPos;
        currRowScores = tempScores;

        memset(currRowPos, 0, img.cols);
        memset(currRowScores, 0, img.cols);


        for (j = 3; j < img.cols-3; ++j, ++pointer)
        {
            int val = pointer[0];                           //value of central pixel
            const uchar *tab = &threshold_tab[255] - val;       //shift threshold tab by val


            int discard = tab[pointer[offset[PIXELS_TO_CHECK[0]]]]
                          | tab[pointer[offset[PIXELS_TO_CHECK[1]]]];

            if (discard == 0)
                continue;

            bool gotoNextCol = false;
            for (k = 2; k < 16; k+=2)
            {
                discard &= tab[pointer[offset[PIXELS_TO_CHECK[k]]]]
                           | tab[pointer[offset[PIXELS_TO_CHECK[k+1]]]];
                if (k == 6 && discard == 0)
                {
                    gotoNextCol = true;
                    break;
                }
                if (k == 14 && discard == 0)
                {
                    gotoNextCol = true;
                }

            }
            if (gotoNextCol) // initial FAST-check failed
                continue;



            if (discard & 1) // check for continuous circle of pixels darker than threshold
            {
                int compare = val - threshold;
                int contPixels = 0;

                for (k = 0; k < onePointFiveCircles; ++k)
                {
                    int a = pointer[offset[k%CIRCLE_SIZE]];
                    if (a < compare)
                    {
                        ++contPixels;
                        if (contPixels > continuousPixelsRequired)
                        {
                            currRowPos[ncandidates++] = j;

                            currRowScores[j] = CornerScore(pointer, offset, threshold);
                            break;
                        }
                    } else
                        contPixels = 0;
                }
            }

            if (discard & 2) // check for continuous circle of pixels brighter than threshold
            {
                int compare = val + threshold;
                int contPixels = 0;

                for (k = 0; k < onePointFiveCircles; ++k)
                {
                    int a = pointer[offset[k%CIRCLE_SIZE]];
                    if (a > compare)
                    {
                        ++contPixels;
                        if (contPixels > continuousPixelsRequired)
                        {
                            currRowPos[ncandidates++] = j;

                            currRowScores[j] = CornerScore(pointer, offset, threshold);
                            break;
                        }
                    } else
                        contPixels = 0;
                }
            }
        }

        if (i == 3)   //skip first row
            continue;

        for (k = 0; k < ncandidatesprev; ++k)
        {
            int pos = prevRowPos[k];
            int score = prevRowScores[pos];

            if (score > pprevRowScores[pos-1] && score > pprevRowScores[pos] && score > pprevRowScores[pos+1] &&
                score > prevRowScores[pos+1] && score > prevRowScores[pos-1] &&
                score > currRowScores[pos-1] && score > currRowScores[pos] && score > currRowScores[pos+1])
            {
                keypoints.emplace_back(cv::KeyPoint((float)pos, (float)(i-1),
                                                    7.f, -1, (float)score, level));
            }
        }
    }
}


int ORBextractor::CornerScore(const uchar* pointer, const int offset[], int threshold)
{
    int val = pointer[0];
    int i;
    int diff[onePointFiveCircles];
    for (i = 0; i < onePointFiveCircles; ++i)
    {
        diff[i] = (val - pointer[offset[i%CIRCLE_SIZE]]);
    }

    int a0 = threshold;
    for (i = 0; i < CIRCLE_SIZE; i+=2)
    {
        int a = std::min(diff[i+1], diff[i+2]);
        a = std::min(a, diff[i+3]);
        if (a <= a0)
            continue;
        a = std::min(a, diff[i+4]);
        a = std::min(a, diff[i+5]);
        a = std::min(a, diff[i+6]);
        a = std::min(a, diff[i+7]);
        a = std::min(a, diff[i+8]);
        a0 = std::max(a0, std::min(a, diff[i]));
        a0 = std::max(a0, std::min(a, diff[i+9]));
    }

    int b0 = -a0;
    for (i = 0; i < CIRCLE_SIZE; i+=2)
    {
        int b = std::max(diff[i+1], diff[i+2]);
        b = std::max(b, diff[i+3]);
        b = std::max(b, diff[i+4]);
        b = std::max(b, diff[i+5]);
        if (b >= b0)
            continue;
        b = std::max(b, diff[i+6]);
        b = std::max(b, diff[i+7]);
        b = std::max(b, diff[i+8]);
        b0 = std::min(b0, std::max(b, diff[i]));
        b0 = std::min(b0, std::max(b, diff[i+9]));
    }

    return -b0 - 1;
}
*/

void ORBextractor::ComputeScalePyramid(cv::Mat &image)
{
    for (int lvl = 0; lvl < nlevels; ++ lvl)
    {
        int width = (int)cvRound(image.cols * invScaleFactorVec[lvl]); // 1.f / getScale(lvl));
        int height = (int)cvRound(image.rows * invScaleFactorVec[lvl]); // 1.f / getScale(lvl));
        int doubleEdge = EDGE_THRESHOLD * 2;
        int borderedWidth = width + doubleEdge;
        int borderedHeight = height + doubleEdge;

        //Size sz(width, height);
        //Size borderedSize(borderedWidth, borderedHeight);

        cv::Mat borderedImg(borderedHeight, borderedWidth, image.type());
        cv::Range rowRange(EDGE_THRESHOLD, height + EDGE_THRESHOLD);
        cv::Range colRange(EDGE_THRESHOLD, width + EDGE_THRESHOLD);

        //imagePyramid[lvl] = borderedImg(cv::Rect(EDGE_THRESHOLD, EDGE_THRESHOLD, width, height));
        imagePyramid[lvl] = borderedImg(rowRange, colRange);


        if (lvl)
        {
            cv::resize(imagePyramid[lvl-1], imagePyramid[lvl], cv::Size(width, height), 0, 0, CV_INTER_LINEAR);

            cv::copyMakeBorder(imagePyramid[lvl], borderedImg, EDGE_THRESHOLD, EDGE_THRESHOLD, EDGE_THRESHOLD,
                               EDGE_THRESHOLD, cv::BORDER_REFLECT_101+cv::BORDER_ISOLATED);
        }
        else
        {
            cv::copyMakeBorder(image, borderedImg, EDGE_THRESHOLD, EDGE_THRESHOLD, EDGE_THRESHOLD, EDGE_THRESHOLD,
                               cv::BORDER_REFLECT_101);
        }
    }
}

/*
float ORBextractor::getScale(int lvl)
{
    return std::pow(scaleFactor, (double)lvl);
}
*/

//TODO:remove from master
//debug
D(
        void ORBextractor::Tests(cv::InputArray inputImage, bool myImplementation,
                                 std::vector<cv::KeyPoint> &resKeypoints, cv::OutputArray outputDescriptors)
        {
            if (inputImage.empty())
                return;
            cv::Mat image = inputImage.getMat();
            assert(image.type() == CV_8UC1);

            ComputeScalePyramid(image);


            for (int lvl = 0; lvl < nlevels; ++lvl)
            {
                int step = (int)imagePyramid[lvl].step1();
                for (int i = 0; i < CIRCLE_SIZE; ++i)
                {
                    pixelOffset[lvl*CIRCLE_SIZE + i] = CIRCLE_OFFSETS[i][0] + CIRCLE_OFFSETS[i][1] * step;
                    //pixelOffset[lvl][i] = CIRCLE_OFFSETS[i][0] + CIRCLE_OFFSETS[i][1] * image.step;
                    //std::cout << "imgstep[" << lvl << "]=" << step << "\n";
                }
            }


            resKeypoints.clear();
            resKeypoints.reserve(nfeatures);

            std::vector<std::vector<cv::KeyPoint>> allMyKeypoints;

            //first bool: distribute, 2nd bool: divide
            DivideAndFAST(allMyKeypoints, true, false, 20);

            FAST fast (iniThFAST, minThFAST);
            fast(image, allMyKeypoints[0], pixelOffset, iniThFAST, 0);



            //DistributeKeypoints(allMyKeypoints[0], 0, 1000, 0, 440, 0);

            cv::Mat BRIEFdescriptors;
            int nkpts = 0;
            for (int lvl = 0; lvl < nlevels; ++lvl)
            {
                nkpts += (int)allMyKeypoints[lvl].size();
            }
            if (nkpts <= 0)
            {
                outputDescriptors.release();
            } else
            {
                outputDescriptors.create(nkpts, 32, CV_8U);
                BRIEFdescriptors = outputDescriptors.getMat();
            }

            ComputeAngles(allMyKeypoints);
            ComputeDescriptors(allMyKeypoints, BRIEFdescriptors);

            for (int lvl = 1; lvl < nlevels; ++lvl)
            {
                float scale = scaleFactorVec[lvl];
                for (auto &kpt : allMyKeypoints[lvl])
                {
                    kpt.pt *= scale;
                }
            }

            for (int lvl = 0; lvl < nlevels; ++lvl)
            {
                resKeypoints.insert(resKeypoints.end(), allMyKeypoints[lvl].begin(), allMyKeypoints[lvl].end());
            }

        }

        void ORBextractor::printInternalValues()
        {
            std::cout << "\nInitialization-values of ORBextractor:" << std::endl <<
                      "nFeatures: " << nfeatures << std::endl <<
                      "scaleFactor: " << scaleFactor << std::endl <<
                      "nLevels: " << nlevels << std::endl <<
                      "initial FAST Threshold: " << iniThFAST << std::endl <<
                      "minimum FAST Threshold: " << minThFAST << std::endl;
        }

        template <class T>
        void ORBextractor::PrintArray(T *array, const std::string &name, int start, int end)
        {
            std::cout << std::endl;
            for (int i = start; i < end; ++i) {
                std::cout << name << "[" << i << "] = " << (int) array[i] << std::endl;
            }

        }

        void ORBextractor::PrintKeypoints(std::vector<cv::KeyPoint> &kpts)
        {
            std::cout << "\nKeypoints found: " << std::endl;
            for (auto &kpt : kpts)
            {
                std::cout << "kpt1: (" << kpt.pt.x << ", " << kpt.pt.y << ")\n";
            }
        }

        void ORBextractor::PrintKeypoints(std::vector<cv::KeyPoint> &kpts, int start, int end)
        {
            if (end <= start)
                return;
            std::cout << "\nKeypoints from " << start << " to " <<  end-1 << ":\n";
            for (int i = start; i < end; ++i)
            {
                std::cout << "kpt[" << i << "]: (" << kpts[i].pt.x << "," << kpts[i].pt.y << ")\n";
            }
        }

        void ORBextractor::PrintKeypoints(std::vector<cv::KeyPoint> &kpts, int start, int end, bool printResponse)
        {
            if (end <= start)
                return;
            std::cout << "\nKeypoints from " << start << " to " <<  end-1 << ":\n";
            for (int i = start; i < end; ++i)
            {
                std::cout << "kpt[" << i << "]: (" << kpts[i].pt.x << "," << kpts[i].pt.y << ")";
                if (printResponse)
                {
                    std::cout << ", with cornerscore of " << kpts[i].response;
                }
                std::cout << "\n";
            }
        }


        void ORBextractor::CompareKeypointVectors(std::vector<cv::KeyPoint> &vec1, std::vector<cv::KeyPoint> &vec2)
        {
            if (vec1.size() != vec2.size())
            {
                std::cout << "\nVectors are not the same size:\n";
                std::cout << "Size of vector 1: " << vec1.size() << std::endl;
                std::cout << "Size of vector 2: " << vec2.size() << std::endl;
            }

            int dif = 0;
            int same = 0;

            bool first = true;
            int i;

            for (i = 0; i < vec1.size(); ++i)
            {
                if (vec1[i].pt.x == vec2[i].pt.x &&
                    vec1[i].pt.y == vec2[i].pt.y &&
                    vec1[i].angle == vec2[i].angle)
                {
                    ++same;
                    //std::cout << "\nkeypoints at " << i << " are identical\n";
                    continue;
                }
                if (first)
                {
                    std::cout << "\nFirst differing kpt is at index " << i << std::endl;
                    first = false;
                }
                ++dif;
            }

            if (dif == 0)
            {
                std::cout << "\nVectors are identical!\n";
            } else
            {
                std::cout << "\n" << dif << " keypoints were different, " << same <<
                          " keypoints were identical (and at the same index)\n";
            }
        }


)
}
#pragma clang diagnostic pop