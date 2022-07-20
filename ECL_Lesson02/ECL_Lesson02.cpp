
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <math.h>
constexpr auto PI = 3.141592;;

using namespace cv;
using namespace std;

float* GetCubemapCoordinate(int x, int y, int face, int edge, float* point)
{
    /*
        -1 ~ 1 사이 정규화된 좌표값 내에서 point를 정해주어야 한다.
        즉, 2의 길이가 필요하기 때문에 a, b에 2를 곱해준다.
        edge < y < 2*edge 사이 b = 2 * 1.??????f이고, 2~4의 범위를 갖는다.
    */
    float a = 2 * (x / (float)edge);
    float b = 2 * (y / (float)edge);

    if (face == 0) { point[0] = -1; point[1] = 1 - a; point[2] = 3 - b; }         // Back
    else if (face == 1) { point[0] = a - 3, point[1] = -1;     point[2] = 3 - b; }   // Left
    else if (face == 2) { point[0] = 1;      point[1] = a - 5; point[2] = 3 - b; }   // Front
    else if (face == 3) { point[0] = 7 - a;   point[1] = 1;     point[2] = 3 - b; }   // Right
    else if (face == 4) { point[0] = a - 3;   point[1] = 1 - b; point[2] = 1; }   // Top
    else if (face == 5) { point[0] = a - 3;   point[1] = b - 5; point[2] = -1; }   // Bottom

    return point;
}


class Camera{
public:
    int widthPixel; // 가로 픽셀
    int heightPixel; // 세로 픽셀

    float verAngle; // 수직 시야각
    float horAngle; // 수평 시야각
};


Mat GenView(Mat* pano, float phi, float theta) // phi = 경도, theta = 위도
{
    

    int outWidth = pano->size().width / 4;
    int outHeight = pano->size().height / 4;



    Mat view;
}

int main(void) {

    /* 파노라마 이미지 불러오기 */
    Mat img = imread("Panorama.png"); //자신이 저장시킨 이미지 이름이 입력되어야 함, 확장자까지

    Camera player;
    player.widthPixel = img.size().width / 2;
    player.heightPixel = img.size().height / 2;

    player.verAngle = 0;
    player.horAngle = 0;


    if (img.empty()) {
        cerr << "Image load failed!" << endl;
        return -1;
    }

    namedWindow("img");
    imshow("img", img);

    while (true) {
        int keycode = waitKeyEx();

        if (keycode == 0x250000 || keycode == 0x270000)
        {
            img = ~img;
            imshow("img2", img);
        }
        else if (keycode == 27 || keycode == 'q' || keycode == 'Q')
        {
            break;
        }
    }
    return 0;

    /*
    Mat SphericalToCubemap;
    SphericalToCubemap = Mat::zeros((0.75f) * img.size().width, img.size().width, img.type());

    SphericalToCubemap = CvtSph2Cub(&img);

    imshow("Spherical Panorama Original Image", img);
    imshow("Cubemap Image", SphericalToCubemap);
    waitKey(0);

    return 0;*/
}