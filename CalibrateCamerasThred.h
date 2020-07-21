#ifndef CalibrateCameraThred_H
#define CalibrateCameraThred_H
#include<qthread.h>
#include <iostream>
#include<fstream>
#include<opencv2\opencv.hpp>
//#include "qvtkopenglnativewidget.h"
//
//#include<vtkSmartPointer.h>
//#include<vtkActor.h>
//#include<vtkRenderer.h>
//#include<vtkRenderWindow.h>
//#include<vtkGenericOpenGLRenderWindow.h>
//#include<vtkRenderWindowInteractor.h>
//#include<vtkPlaneSource.h>
//#include <vtkCylinderSource.h>
//#include <vtkCubeSource.h>
//#include<vtkPolyDataMapper.h>
//#include<vtkAxesActor.h>
//#include <vtkMatrix4x4.h>
//#include <vtkTransform.h>
//#include <vtkDelaunay2D.h>
//#include <vtkTransformPolyDataFilter.h>
//#include <vtkSphereSource.h>
//#include<vtkProperty.h>
//#include<vtkAppendPolyData.h>
//#include<vtkPLYWriter.h>
//#include <vtkPLYReader.h>
//#include<random>

using namespace std;
using namespace cv;
class CalibrateCamerasThred :public QThread
{
	Q_OBJECT
public:
	CalibrateCamerasThred();
	~CalibrateCamerasThred();

	void run()Q_DECL_OVERRIDE;


	cv::Size image_size;
	Size board_size;
	std::vector<std::vector<cv::Point3f>> *Left_object_points;
	std::vector<std::vector<cv::Point3f>> *Right_object_points;

	std::vector<cv::Mat> * Left_rvecsMat, *Left_tvecsMat;

	std::vector<std::vector<cv::Point2f>> *Left_Corner_buf;
	cv::Mat * Left_CameraMatrix, *Left_CameradistCoeffs;
	double  Left_averageerror;


	std::vector<cv::Mat> * Right_rvecsMat, *Right_tvecsMat;
	std::vector<std::vector<cv::Point2f>> *Right_Corner_buf;
	cv::Mat * Right_CameraMatrix, *Right_CameradistCoeffs;
	double  Right_averageerror;

	//QVTKOpenGLNativeWidget* qvtkOpenGLWidget_Big;
	string WorkSpace;

signals:
	void FinishedCalibrate(double, double);
};

#endif