#ifndef CAMERA_H
#define CAMERA_H

#include <Windows.h>	// 这个头文件很重要
#include "../Camera/CameraApi.h"
//#include "../Camera/CameraApiLoad.h"
#include "../Camera/CameraDefine.h"
#include "../Camera/CameraStatus.h"
#pragma comment(lib,"../lib/MVCAMSDK_X64.lib")//预处理

#include<qobject.h>
// opencv
#include <opencv2\opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <qlabel.h>
#include <qthread.h>

typedef unsigned char BYTE;
using namespace cv;
using namespace std;

class Camera : public QObject
{
	Q_OBJECT

public:
	explicit  Camera(QObject * parent=0);
	//Camera();
	//~Camera();

	// 工业相机初始化
	Mat ImageCapture();			    // 输入相机HMV，获取一张图像
	void myCreate(int AEC, int Gain);// 根据HMV初始化和loc信息初始化相机
	void CamInit(tSdkCameraDevInfo sCameraList); // 根据idx,range初始化相机
	void WriteCapturedImage();
	void SetTracking(bool );
	void GetChessBoard(vector<Point2f> &);
	//void CapturePicture(bool);

	Mat CapturedImage;
	vector<Point2f> CorresponedChessBoard;
	bool detectChessBoard;
	Size board_size;
	std::string imageSavePath;
	int IMGCOUNT;
	std::string Name;
	Mat cameraMatrix;
	Mat distCoeffs;

	bool ifundistort;

	bool wheatherCapturePicture;
	bool wheatherCapturePicture2;

	bool iftracking;
	bool iftracking2;


	bool ifreconstruction;
	bool ifreconstruction2;
signals:
	void wheatherSuccessCapturePicture(int);


public slots:
	void RenderPicture();



private:
	CameraHandle hmv;
	QLabel * Label;





};
#endif
