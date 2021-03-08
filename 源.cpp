#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>
#include <math.h>
#include<string>

using namespace cv;
using namespace std;

//测量两点之间距离
int distance(Point a,Point b)
{
	int x2, y2, result;
	x2 = pow(a.x - b.x,2);
    y2 = pow(a.y - b.y, 2);
	result = x2 + y2;
	return result;
}

int main()
{
	//图片集包含在工程文件
	Mat picture_1 = imread("..//.//tu//A.jpg");
	//用于找出定位角
	Mat picture_2, picture_3, picture_4, picture_5;
	//用于框选二维码
	Mat drawing1,drawing2, drawing3, drawing4, drawing5, drawing6;

	imshow("1", picture_1);


	cvtColor(picture_1, picture_2, COLOR_BGR2GRAY);
	blur(picture_2, picture_2, Size(1, 1));
	//imshow("2", picture_2);

	//均值
	equalizeHist(picture_2, picture_2);
	//imshow("3", picture_2);

	//自适应二值化
	adaptiveThreshold(picture_2, picture_2, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 35, 15);
	//imshow("4", picture_2);


	//使图片尺寸一致
	picture_3 = Mat::zeros(picture_2.size(), CV_8UC3);
	picture_4 = Mat::zeros(picture_2.size(), CV_8UC3);
	picture_5 = Mat::zeros(picture_2.size(), CV_8UC3);
	drawing1 = Mat::zeros(picture_2.size(), CV_8UC3);
	drawing2 = Mat::zeros(picture_2.size(), CV_8UC3);
	drawing3 = Mat::zeros(picture_2.size(), CV_8UC3);
	drawing4 = Mat::zeros(picture_2.size(), CV_8UC3);
	drawing5 = Mat::zeros(picture_2.size(), CV_8UC3);
	drawing6 = Mat::zeros(picture_2.size(), CV_8UC3);

	vector <vector<Point>> contours, contours2, contours3;
	vector<Vec4i> hierarchy;

	//找出轮廓
	findContours(picture_2, contours, hierarchy, RETR_TREE, CHAIN_APPROX_TC89_KCOS, Point(0, 0));

	//初步筛选轮廓
	int ic = 0, parentIdx = -1;
	for (int i = 0; i < contours.size(); i++)
	{
		if (hierarchy[i][2] != -1 && ic == 0)
		{
			parentIdx = i;
			ic++;
		}
		else if (hierarchy[i][2] != -1)
		{
			ic++;
		}
		else if (hierarchy[i][2] == -1)
		{
			ic = 0;
			parentIdx = -1;
		}
		if (ic == 2)
		{
			contours2.push_back(contours[parentIdx]);
			ic = 0;
			parentIdx = -1;
		}
	}
	drawContours(picture_4, contours2, -1, CV_RGB(255, 255, 255), 1, 8);
	//imshow("6", picture_4);

	//进一步筛选轮廓
	int area = 0;
	vector<Point>approx;
	vector<int>contourIdx;
	for (size_t i = 0; i < contours2.size(); i++)
	{
		approxPolyDP(Mat(contours2[i]), approx, arcLength(Mat(contours2[i]), true) * 0.02, true);

		if (approx.size() >= 4 && fabs(contourArea(Mat(approx))) < 2000 && isContourConvex(Mat(approx)));
		{
			area = contourArea(contours2[i]);
			RotatedRect rectPoint = minAreaRect(contours2[i]);
			Point2f fourPoint2f[4];
			rectPoint.points(fourPoint2f);
			float reatanarea = pow(fabs(fourPoint2f[1].x - fourPoint2f[0].x), 2) + pow(fabs(fourPoint2f[1].y - fourPoint2f[0].y), 2);
			if ((fabs(reatanarea - area) / area) < 0.4)
			{
				contours3.push_back(contours2[i]);
				drawContours(picture_5, contours3, -1, CV_RGB(255, 255, 255), 1, 8);
			}
		}
	}
	//imshow("7", picture_5);


	Scalar color(1, 1, 225);
	Point point[10], structure_point[10],four;
	RotatedRect rectPoint2;
	Point2f fourPoint2f2[4];

	//寻找定位角中心
	for (int j = 0; j <= contours3.size() - 1; j++)
	{
		RotatedRect rectPoint2 = minAreaRect(contours3[j]);
		rectPoint2.points(fourPoint2f2);

		point[j].x = (fourPoint2f2[0].x + fourPoint2f2[2].x) / 2;
		point[j].y = (fourPoint2f2[0].y + fourPoint2f2[2].y) / 2;

	}
	area = contourArea(contours3[1]);
	
	//勾勒大概轮廓
	int area_side = cvRound(sqrt(double(area)));
	int distance1, distance0=0,m;
	for (int i = 0; i < contours3.size(); i++)
	{
		line(drawing1, point[i % contours3.size()], point[(i + 1) % contours3.size()], color, area_side , 8);
		distance1 = distance(point[i % contours3.size()], point[(i + 1) % contours3.size()]);
		if (distance1 > distance0)
		{
			distance0 = distance1;
			m = i;
		}
	}

	//寻找二维码剩余一角位置,并完善轮廓勾勒
	four = point[m % contours3.size()] + point[(m + 1) % contours3.size()] - point[(m + 2) % contours3.size()];
	line(drawing1, point[0], four, color, area_side , 8);
	
	//根据大概轮廓框选二维码
	vector <vector<Point>> contours_all, structure_contours, contours4;
	vector<Vec4i> hierarchy_all;
	cvtColor(drawing1, drawing2, COLOR_BGR2GRAY);
	threshold(drawing2, drawing3, 45, 255, THRESH_BINARY);
	//imshow("threshold", drawing3);
	findContours(drawing3, contours_all, hierarchy_all, RETR_EXTERNAL, CHAIN_APPROX_TC89_KCOS, Point(0, 0));
	Point2f fourPoint2f3[4];
	RotatedRect rectPoint = minAreaRect(contours_all[0]);
	rectPoint.points(fourPoint2f3);
	for (int i = 0; i < 4; i++)
	{
		line(picture_1, fourPoint2f3[i % 4], fourPoint2f3[(i + 1) % 4], Scalar(20, 21, 237), 3);

		line(drawing6, fourPoint2f3[i % 4], fourPoint2f3[(i + 1) % 4], Scalar(20, 21, 237), 3);
		
	}
	//imshow("8", drawing6);
	imshow("result", picture_1);
	
	//二维码在图片中四点坐标
	cout << "二维码四个顶点坐标" << endl;
	for (int i = 0; i <= 3; i++)
	{
		cout << fourPoint2f3[i].x << " " << fourPoint2f3[i].y << endl;
	}
	waitKey();
	return 0;
}