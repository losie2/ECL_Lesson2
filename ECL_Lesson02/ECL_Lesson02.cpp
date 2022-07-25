#include "opencv2/opencv.hpp"
#include <iostream>
#include <math.h>
#include <algorithm>

using namespace cv;
using namespace std;

#define FNAME "Sample1.png"
#define FNAME_INSERT "clip.png"
#define PI 3.14159

struct Axis
{
	int x;
	int y;
	int z;
};
struct AxisDouble
{
	double x;
	double y;
	double z;
};

Mat imgView;
Mat imgPanorama;
double HorizontalViewAngle = 1.5f;
double VerticalViewAngle = 1.2f;
int ViewSize = 500;

double thetaPlus = 0;
double phiPlus = 0;

Axis getNearPixel(Mat Img, int x, int y);
Mat remapImage(Mat imgIn);
AxisDouble CalcThetaPhiToXYZ(double theta, double phi);
AxisDouble CalcCubicXYZ(double x, double y, double z);
void GenView(Mat SourceImg);
void KeyDownEvent(int ch, Mat SourceImg);
void InsertClip(Mat SourceImg);

//#######
//Image Processing
//#######
Axis getNearPixel(Mat Img, int x, int y)
{
	int col = 1;
	int row = 1;
	bool done = false;
	Axis xy;

	while (!done)
	{
		for (int i = -col; i <= col; i++)
		{
			for (int j = -row; j <= row; j++)
			{
				if (x + i <= 0 || y + j <= 0)
				{
					continue;
				}
				if (x + i >= Img.cols || y + j >= Img.rows)
				{
					continue;
				}
				// (3, 3) 범위 내에서 보간.
				if (!(Img.at<Vec3b>(Point(x + i, y + j))[0] == 0 && Img.at<Vec3b>(Point(x + i, y + j))[1] == 0 && Img.at<Vec3b>(Point(x + i, y + j))[2] == 0))
				{
					xy.x = x + i;
					xy.y = y + j;
					return xy;
				}
			}
		}
		col++;
		row++;

		if (col > 3 || row > 3)
		{
			xy.x = x;
			xy.y = y;
			return xy;
		}
	}
}

Mat remapImage(Mat imgIn)
{
	int width = imgIn.cols;
	int height = imgIn.rows;
	Mat RemapImg = Mat(height, width, CV_8UC3, Scalar(0, 0, 0));

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			Axis xy = getNearPixel(imgIn, i, j);
			RemapImg.at<Vec3b>(Point(i, j)) = imgIn.at<Vec3b>(Point(xy.x, xy.y));
		}
	}

	return RemapImg;

}


//#######
//Generate View
//#######
AxisDouble CalcThetaPhiToXYZ(double theta, double phi)
{
	AxisDouble axis;

	//theta, phi -> x,y,z 변환
	double x1 = cos(theta) * cos(phi);
	double y1 = cos(theta) * sin(phi);
	double z1 = sin(theta);

	//z축 회전 변환
	double x2 = x1 * cos(phiPlus) - y1 * sin(phiPlus);
	double y2 = x1 * sin(phiPlus) + y1 * cos(phiPlus);
	double z2 = z1;

	//y축 회전 변환
	double x3 = x2 * cos(thetaPlus) + z2 * sin(thetaPlus);
	double y3 = y2;
	double z3 = -x2 * sin(thetaPlus) + z2 * cos(thetaPlus);

	axis.x = x3;
	axis.y = y3;
	axis.z = z3;

	return axis;
}

AxisDouble CalcCubicXYZ(double x, double y, double z)
{
	AxisDouble axis;

	//큐브맵화
	double a = x;
	double x1 = x / a;
	double y1 = y / a;
	double z1 = z / a;

	axis.x = x1;
	axis.y = y1;
	axis.z = z1;

	return axis;
}
void GenView(Mat SourceImg)
{
	int width = SourceImg.cols;
	int height = SourceImg.rows;

	int Multiple = 2; // 연산의 속도를 위함
	double radius = height / 2 / Multiple; // CubemapEdgeLength / 2, Normalization => -1 ~ 0, 0 ~ 1

	int viewWidth = (int)((double)height * HorizontalViewAngle / Multiple);
	int viewHeight = (int)((double)height * VerticalViewAngle / Multiple);

	imgView = Mat(viewHeight, viewWidth, CV_8UC3, Scalar(0, 0, 0));
	resize(SourceImg, imgPanorama, Size(width, height), 0, 1);


	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			Vec3b pixel = SourceImg.at<Vec3b>(Point(i, j));

			double theta = (((double)j / height) * PI) - (PI * 0.5f);	//-pi/2 ~ pi/2
			double phi = (((double)i / width) * 2 * PI) - PI;			//-pi ~ pi

			AxisDouble axis = CalcThetaPhiToXYZ(theta, phi);

			if (axis.x > 0)
			{
				/*
					전체 구면 좌표계 내에서 x > 0 인 평면만 고려, 
					회전 변환을 거친 후 좌표계에는 변동이 없다는 점을 이용.
				*/
				AxisDouble axis2;
				axis2 = CalcCubicXYZ(axis.x, axis.y, axis.z);

				int y = (int)(axis2.y * radius) + viewWidth / 2;
				int z = (int)(axis2.z * radius) + viewHeight / 2;

				if (y >= 0 && y < viewWidth && z >= 0 && z < viewHeight)
				{

					int PointX = y;
					int PointY = z;

					if (PointX < 5 || PointX > viewWidth - 6 || PointY < 5 || PointY > viewHeight - 6)
					{
						Vec3b C = { 255, 0, 0 }; // BGR
						imgPanorama.at<Vec3b>(Point(i, j)) = C;
					}

					imgView.at<Vec3b>(Point(PointX, PointY)) = pixel;

				}
			}
		}
	}
	imgView = remapImage(imgView);
	resize(imgView, imgView, Size(ViewSize, (int)((double)ViewSize / (HorizontalViewAngle / VerticalViewAngle))), 0, 1);
	imshow("Panorama", imgPanorama);
	imshow("View", imgView);

	int ch = waitKey();
	KeyDownEvent(ch, SourceImg);
}

void KeyDownEvent(int ch, Mat SourceImg)
{
	switch (ch)
	{
	case 's'://Theta Up
		//if (thetaPlus <= PI / 2)
	{
		thetaPlus += 5 * (PI / 365);
		//if (thetaPlus > PI / 2)
		{
			//	thetaPlus = PI / 2;
		}
		cout << "Lat : " << -thetaPlus << ", Long : " << -phiPlus << endl;
	}

	GenView(SourceImg);
	break;

	case 'w'://Theta Down
		//if (thetaPlus >= -PI / 2)
	{
		thetaPlus -= 5 * (PI / 365);
		//if (thetaPlus < -PI / 2)
		{
			//	thetaPlus = -PI / 2;
		}
		cout << "Lat : " << -thetaPlus << ", Long : " << -phiPlus << endl;
	}

	GenView(SourceImg);
	break;

	case 'd'://Phi Down
		phiPlus -= 5 * (2 * PI / 365);

		if (phiPlus <= 0)
		{
			phiPlus = 2 * PI;
		}
		cout << "Lat : " << -thetaPlus << ", Long : " << -phiPlus << endl;

		GenView(SourceImg);
		break;

	case 'a'://Phi Up
		phiPlus += 5 * (2 * PI / 365);
		if (phiPlus >= 2 * PI)
		{
			phiPlus = 0;
		}
		cout << "Lat : " << -thetaPlus << ", Long : " << -phiPlus << endl;

		GenView(SourceImg);
		break;

	case 'k'://Insert Image
		cout << "Insert Image At Lat : " << -thetaPlus << ", Long : " << -phiPlus << endl;
		InsertClip(SourceImg);
		GenView(SourceImg);
		break;
	default:
		GenView(SourceImg);
		break;
	}
}

//#######
//Insert Image
//#######
void InsertClip(Mat SourceImg)
{
	int width = SourceImg.cols;
	int height = SourceImg.rows;

	double radius = height / 2;
    player.horAngle = 0;
	int viewWidth = (int)((double)height * HorizontalViewAngle / 2);
	int viewHeight = (int)((double)height * VerticalViewAngle / 2);

	//Find Clip
	Mat OriginClip = imread(FNAME_INSERT);
	if (OriginClip.empty())
	{
		std::cout << "File Not Found." << std::endl;
		return;
	}
	Mat Clip;
	resize(OriginClip, Clip, Size(viewWidth, viewHeight), 0, 1);


	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			Vec3b pixel = SourceImg.at<Vec3b>(Point(i, j));

			double theta = (((double)j / height) * PI) - (PI * 0.5f);	//-pi/2 ~ pi/2
			double phi = (((double)i / width) * 2 * PI) - PI;			//-pi ~ pi

			AxisDouble axis = CalcThetaPhiToXYZ(theta, phi);

			if (axis.x > 0)
			{
				AxisDouble axis2;
				axis2 = CalcCubicXYZ(axis.x, axis.y, axis.z);
    }
				int y = (int)(axis2.y * radius) + radius;
				int z = (int)(axis2.z * radius) + radius;

    while (true) {
				if (y > radius - viewWidth / 2 && y < radius + viewWidth / 2 && z > radius - viewHeight / 2 && z < radius + viewHeight / 2)
				{
					int PointX = y - ((height - viewWidth) / 2);
					if (PointX < 0)
					{
						PointX = 0;
					}
					else if (PointX > viewWidth)
					{
						PointX = viewWidth - 1;
					}
					int PointY = z - ((height - viewHeight) / 2);
					if (PointY < 0)
					{
						PointY = 0;
					}
					else if (PointY > viewHeight)
					{
						PointY = viewHeight - 1;
					}
					SourceImg.at<Vec3b>(Point(i, j)) = Clip.at<Vec3b>(Point(PointX, PointY));

				}
			}
		}
	}
}

int main()
{
	//Input image
	Mat OriginImg = imread(FNAME);
	if (OriginImg.empty())
	{
		std::cout << "File Not Found." << std::endl;
		return -1;
	}
	Mat SourceImg;
	resize(OriginImg, SourceImg, Size(1400, 700), 0, 1);

	GenView(SourceImg);

    SphericalToCubemap = CvtSph2Cub(&img);
	return 0;

    return 0;*/
}