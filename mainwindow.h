#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QVTKWidget.h"

#include <QVTKOpenGLWidget.h>
#include <QFileDialog>
#include <qthread.h>

#include "Camera.h"
//#include "Tracking.h"
//#include "StructureLight.h"
#include "ui_mainwindow.h"
#include"DetectChessBoard.h"
#include "CalibrateCameras.h"
#include "Tracking.h"
#include"ThreeDimensionReconstruction.h"
#include <opencv2\opencv.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>

#include<vtkSmartPointer.h>
#include<vtkPlaneSource.h>
#include<vtkPolyDataMapper.h>
#include<vtkActor.h>
#include<vtkRenderer.h>
#include<vtkRenderWindow.h>
#include<vtkGenericOpenGLRenderWindow.h>
#include<vtkRenderWindowInteractor.h>
#include<vtkAxesActor.h>
#include<random>
#include<time.h>
#include <io.h>
#include <direct.h>
#include <vtkSmartPointer.h>
#include <vtkPlaneSource.h>
#include <vtkCylinderSource.h>
#include <vtkCubeSource.h>
#include <vtkAxesActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkDelaunay2D.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>
#include<vtkProperty.h>
#include<vtkAppendPolyData.h>
#include<vtkPLYWriter.h>
#include <vtkPLYReader.h>

#include<stack>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();



	VideoCapture cap;
	void InitCameras();
	Mat rotateImage1(Mat img, int degree);
	bool ReadCalibrationParameters(std::string CalibritionFilePath, int camera);
	bool ReadSteroCalibrationParameters(std::string CalibritionFilePath);
	Size square_size;
	Size board_size;
	Size image_size;
	bool stop;

	bool detectChessBoard;
	bool RectifyCameraDeformation;
	bool SteroRectifiy;
	bool CalibrateLeftCamera;
	bool CalibrateRightCamera;
	bool SteroCalibrate;
	bool findcorner_left;
	bool findcorner_right;
	DetectChessBoard * DetectChessBoard_left_thred;
	DetectChessBoard * DetectChessBoard_right_thred;

	bool captureOneFrame;
	int IMGCOUNT;
	int LeftIMGCOUNT;
	int RightIMGCOUNT;
	void WriteCapturedImage(Mat& LeftCamera, vector<Point2f> &image_points_buf, String FileHeadname, int num);

	Mat leftImage;
	Mat rightImage;
	vector<Point2f> image_points_buf_left, image_points_buf_right;//缓存每幅图像上检测到的角点



	bool iftracking;

	vector<vector<Point2f>> Corner_buf_Left, Corner_buf_Right;
	vector<Mat> SourceImagesOfLeftCamera, SourceImagesOfRightCamera;
	void ReadImagesAndChessBoardCoordinates();
	int tem_count;

	bool ifCollectImages;

signals:
	void RenderLeftCamera();
	void RenderRightCamera();
	void RenderSingleCameraCalibritionResult();
	void RenderSteroCalibritionResult();
	void WorkSpaceChange();
	void StartReconstruction();

	
public slots://槽函数声明标志

	void ToolBoxChanged( int status);
	void OpenCamera();
	void CloseCamera();
	void detectChessBoardCheckBoxChanged(bool);
	void rectifyDeformationCheckBoxChanged(bool);
	void SteroRectifiyCheckBoxChanged(bool);
	void RenderAfreame();
	void CaptureImage();
	void ClearAll();
	void test();
	void directoryBrowsePushButton_clicked();

	void Track_AfterLoadParameters(bool);
	void StartTracking();
	void StopTracking();

	void CollectGrayCodesImages();
	void Rec_AfterLoadParameters(bool);
	void Rec_AfterReconstruction(bool);
	void LeftCameraRadioButtonChanged(bool);
	void RightCameraRadioButtonChanged(bool);
	void SteroCalibrationRadioButtonChanged(bool);
	void ProjectionDelaChanged(int);
private:
    Ui::MainWindow *ui;
	std::string WorkSpace;
	QString WorkSpace1;
	Camera * Camera_Left;
	Camera * Camera_Right;
	Camera * Camera_Two;
	QThread * m_childThread_Left;
	QThread * m_childThread_Right;

	vector<Mat> rvecsMatOfLeftCamera;
	vector<Mat> tvecsMatOfLeftCamera;
	Mat LeftCameraMatrix;
	Mat LeftCameradistCoeffs;

	vector<Mat> rvecsMatOfRightCamera;
	vector<Mat> tvecsMatOfRightCamera;
	Mat RightCameraMatrix;
	Mat RightCameradistCoeffs;


	Mat R, T, E, F, R1, R2, P1, P2, Q;  //stero calibrition Result


	QThread * TrackingThread;
	//TrackingObject * Tracking_object;
	CalibrateCameras *CalibrateCameras_object;
	Tracking* Tracking_object;
	ThreeDimensionReconstruction* Reconstruction_object;

	QTimer * m_timer;
	int step;
	//reconstruction
	vector<Mat> gray_codes;					// 格雷码图像
	int proj_w;
	int proj_h;

	const char* projWindName = "ProjWindow";

	const char* cameraLeftWindName = "LeftWindName";
	const char* cameraRightWindName = "RightWindName";

	double BrightParam;
	int CountNumberOfGrayCodes;
	int proj_delay;
	

private slots:

	//void TimerTimeOut();
};

#endif // MAINWINDOW_H
