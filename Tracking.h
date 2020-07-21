#ifndef Tracking_H
#define Tracking_H
#include<qobject.h>
#include<opencv2\opencv.hpp>
#include<DetectChessBoard.h>

#include <QtWidgets/QTextBrowser>
#include "qvtkopenglnativewidget.h"
#include<QtWidgets\qlabel.h>
#include<vtkSmartPointer.h>
#include<vtkActor.h>
#include<vtkRenderer.h>
#include<vtkRenderWindow.h>
#include<vtkGenericOpenGLRenderWindow.h>
#include<vtkRenderWindowInteractor.h>
#include<vtkPlaneSource.h>
#include <vtkCylinderSource.h>
#include <vtkCubeSource.h>
#include<vtkPolyDataMapper.h>
#include<vtkAxesActor.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkDelaunay2D.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkSphereSource.h>
#include<vtkProperty.h>
#include<vtkAppendPolyData.h>
#include<vtkPLYWriter.h>
#include <vtkPLYReader.h>
#include<random>
using namespace std;
using namespace cv;
class Tracking:public QObject
{
	Q_OBJECT
public:
	Tracking();
	~Tracking();
	void split(const std::string& s, std::vector<std::string>& sv, const char flag = ' ');
	bool ReadCalibrationParameters(std::string CalibritionFilePath, int camera);
	bool ReadSteroCalibrationParameters(std::string CalibritionFilePath);
	void SetRenderWindow();
	void RenderInitialization();
	void TrackingAndRendering();
	void triangulation(vector< Point3d >& points);
	Point2d pixel2cam(const Point2f& p, const Mat& K);
	std::string WorkSpace;

	vector<Mat> rvecsMatOfLeftCamera;
	vector<Mat> tvecsMatOfLeftCamera;

	Mat LeftCameraMatrix;
	Mat LeftCameradistCoeffs;

	vector<Mat> rvecsMatOfRightCamera;
	vector<Mat> tvecsMatOfRightCamera;
	Mat RightCameraMatrix;
	Mat RightCameradistCoeffs;
	Mat R, T, T1, T2, E, F, R1, R2, P1, P2, Q;  //stero calibrition Result


	vector<Point2f> *image_points_buf_left, *image_points_buf_right;//缓存每幅图像上检测到的角点


	vtkSmartPointer<vtkActor> LastActor;
	vtkSmartPointer<vtkRenderer> Renderer;
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> RenderWindow;
	QTextBrowser *textBrowser;
	QVTKOpenGLNativeWidget* qvtkOpenGLWidget_Big;
public slots:
	void LoadParameters();
signals:
	void AfterLoadParameters(bool);
private:

	Mat leftImage;
	Mat rightImage;

};


#endif // !Tracking_H
