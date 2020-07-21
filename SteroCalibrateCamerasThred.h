#ifndef STEC_H
#define STEC_H
#include<qthread.h>
#include<opencv2\opencv.hpp>
#include <iostream>
#include<fstream>
//#include<QVTKOpenGLNativeWidget.h>
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
class SteroCalibrateCamerasThred: public QThread
{
	Q_OBJECT
public:
	SteroCalibrateCamerasThred();
	~SteroCalibrateCamerasThred();

	void run()Q_DECL_OVERRIDE;
	Point2d pixel2cam(const Point2d& p, const Mat& K);
	//void RenderBothCamerasAndPlanes();
	//void TransformTV2TM(Mat rvecsMatOfCamera, Mat tvecsMatOfCamera, vtkSmartPointer<vtkMatrix4x4> RTM);
	cv::Size image_size;
	Size board_size;
	std::vector<std::vector<cv::Point3f>> *object_points;

	std::vector<std::vector<cv::Point2f>> *Corner_buf_Left,*Corner_buf_Right;

	//std::vector<cv::Mat> * Left_rvecsMat, *Left_tvecsMat;
	//std::vector<cv::Mat> * Right_rvecsMat, *Right_tvecsMat;

	cv::Mat * LeftCameraMatrix, *LeftCameradistCoeffs,*RightCameraMatrix,*RightCameradistCoeffs;

	Mat *R, *T;

	string WorkSpace;

	//QVTKOpenGLNativeWidget* qvtkOpenGLWidget_Big;

signals:
	void FinishedSteroCalibration(double);

};


#endif // !STEC_H
