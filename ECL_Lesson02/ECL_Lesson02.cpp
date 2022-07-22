
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

    float pitch; // 수직 시야각
    float yaw; // 수평 시야각
};


void GenView(Mat* pano, Camera* Player) // yaw = 경도, pitch = 위도
{
    float u, v;
    float phi, theta;

    Mat view;
    view = Mat::zeros(Player->heightPixel, Player->widthPixel, pano->type());
    
    /*
        Camera angle
    */
    
    u = (Player->yaw / pano->size().width);
    v = (Player->pitch / pano->size().height);

    phi = 2 * u * PI;
    theta = v * PI;

    int xPixel = 0;
    int yPixel = 0;
    for (int j = Player->pitch - (Player->heightPixel / 2); j < (Player->heightPixel / 2) + Player->pitch; j++) {
        v = 1 - ((float)j / pano->size().height);
        theta = v * PI;
        for (int i = Player->yaw - (Player->widthPixel / 2); i < (Player->widthPixel / 2) + Player->yaw; i++)
        {
            u = ((float)i / pano->size().width);
            phi = 2 * u * PI;            
            float x, y, z; // 단위 벡터
            x = cos(phi) * sin(theta) * -1;
            y = sin(phi) * sin(theta) * -1;
            z = cos(theta);

            float xa, ya, za;
            float a;

            cout << x << " " << y << " " << z << " " << endl;

            a = max(abs(x), max(abs(y), abs(z)));

            /*
                큐브 면 중 하나에 있는 단위 벡터와 평행한 벡터.
                이 때, ya가 -1인지 1인지(Left, Right) 값을 보고 평면을 결정.
                ya가 1 or -1이라면 y벡터의 변화가 없다는 뜻. 즉 xz평면만 고려한다는 의미.
                xa와 za도 동일하게 적용.
            */
            xa = x / a;
            ya = y / a;
            za = z / a;

           // view.at<Vec3b>(yPixel, abs((phi * Player->widthPixel))) = pano->at<Vec3b>(0, 0);
        }
    }
    imshow("img", view);
    waitKey(0);
 //   return view;
}

int main(void) {    

    /* 파노라마 이미지 불러오기 */
    Mat img = imread("Panorama.png"); //자신이 저장시킨 이미지 이름이 입력되어야 함, 확장자까지

    Camera player;
    player.widthPixel = img.size().width / 2;
    player.heightPixel = img.size().height / 2;

    player.pitch = 50;
    player.yaw = 250;

    Mat quarterImg = Mat::zeros(player.heightPixel, player.widthPixel, img.type());
    GenView(&img, &player);

    if (img.empty()) {
        cerr << "Image load failed!" << endl;
        return -1;
    }

    namedWindow("img");
    //imshow("img", img);
    /*
    while (true) {
        int keycode = waitKeyEx();

        if (keycode == 0x250000 && player.yaw - 5 > 0) // yaw left
        {
            player.yaw -= 5;
            quarterImg = GenView(&img, &player);
            
        }
        else if (keycode == 0x270000 && player.yaw + 5 < img.size().width) // yaw right
        {
            player.yaw += 5;
            quarterImg = GenView(&img, &player);
        }
        else if (keycode == 0x260000) // pitch up
        {
            player.pitch += 5;
            quarterImg = GenView(&img, &player);
        }
        else if (keycode == 0x280000) // pitch down
        {
            player.pitch -= 5;
            quarterImg = GenView(&img, &player);
        }
    }*/
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