#ifndef CalibrateCameras_H
#define CalibrateCameras_H 

#include"Camera.h"
#include<opencv2\opencv.hpp>
#include<qobject.h>
#include <qtimer.h>
#include <io.h>
#include<DetectChessBoard.h>
#include<CalibrateCamerasThred.h>
#include<SteroCalibrateCamerasThred.h>

#include <QtWidgets/QTextBrowser>
#include "qvtkopenglnativewidget.h"

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
class CalibrateCameras:public QObject
{
	Q_OBJECT
public:
	CalibrateCameras();
	~CalibrateCameras();

	bool ReadImagesAndChessBoardCoordinates();
	void split(const std::string& s, std::vector<std::string>& sv, const char flag = ' ');
	void TransformTV2TM(Mat rvecsMatOfCamera, Mat tvecsMatOfCamera, vtkSmartPointer<vtkMatrix4x4> RTM);
	bool ReadCalibrationParameters(std::string CalibritionFilePath, int camera);
	void RenderPlaneAndCamera();
	void RenderBothCamerasAndPlanes();
	bool ReadLeftImagesAndChessBoardCoordinates();
	bool ReadRightImagesAndChessBoardCoordinates();
	QTextBrowser *textBrowser;
	QVTKOpenGLNativeWidget* qvtkOpenGLWidget_Big;
	QLabel* CaCa_label_count;

	//QTimer * m_timer;

	std::string WorkSpace;
	Size board_size;
	Size image_size;
	Size square_size;

public slots:

	void Calibrate();
	void AfterCalibrate(double leftAverageError, double rightAverageError);
	void SteroCalibrate();
	void AfterSteroCalibration(double reprojectionerror);

	void test();
private:
	int IMGCOUNT;
	int LeftIMGCOUNT;
	int RightIMGCOUNT;
	vector<vector<Point2f>> Corner_buf_Left, Corner_buf_Right;
	vector<Mat> SourceImagesOfLeftCamera, SourceImagesOfRightCamera;

	CalibrateCamerasThred *CalibrateThred;

	vector<vector<Point3f>> Left_object_points;
	vector<vector<Point3f>> Right_object_points;
	vector<vector<Point3f>> object_points;
	vector<Mat> rvecsMatOfLeftCamera;
	vector<Mat> tvecsMatOfLeftCamera;
	Mat LeftCameraMatrix;
	Mat LeftCameradistCoeffs;

	vector<Mat> rvecsMatOfRightCamera;
	vector<Mat> tvecsMatOfRightCamera;
	Mat RightCameraMatrix;
	Mat RightCameradistCoeffs;


	SteroCalibrateCamerasThred * SteroCalibrateThred;
	Mat R, T, E, F, R1, R2, P1, P2, Q;  //stero calibrition Result


	
};
# endif
