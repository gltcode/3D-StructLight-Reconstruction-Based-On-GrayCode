#ifndef ThreeDimensionReconstrction_H
#define ThreeDimensionReconstrction_H
#include<opencv2\opencv.hpp>
#include<qobject.h>
#include <iostream>
#include<fstream>
#include <io.h>
#include <QtWidgets/QTextBrowser>
#include "qvtkopenglnativewidget.h"
#include<QtWidgets\qlabel.h>
#include <QtWidgets/QSlider>
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
#include <vtkOBJReader.h>
#include <vtkOBJWriter.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include<random>
#include<vtkAppendPolyData.h>
using namespace std;
using namespace cv;

class ThreeDimensionReconstruction : public QObject
{
	Q_OBJECT
public:
	ThreeDimensionReconstruction();
	~ThreeDimensionReconstruction();
	void split(const std::string& s, std::vector<std::string>& sv, const char flag = ' ');
	bool ReadCalibrationParameters(std::string CalibritionFilePath, int camera);
	bool ReadSteroCalibrationParameters(std::string CalibritionFilePath);
	Mat rotateImage1(Mat img, int degree, int mode);
	bool generateGrayCodes(int width, int height,	// 图像的宽和高
		bool sl_scan_cols, bool sl_scan_rows);		// 是否打行列图
	bool decodeGrayCodes(int max_width, int max_height, vector<Mat>& gray_codes,
		Mat& decoded_cols, Mat& decoded_rows, Mat& mask,
		int sl_thresh);
	void RenderInitializa();
	void RenderReconstructionResult(vtkSmartPointer<vtkPolyData>ReconstructionResult);
	Point2d pixel2cam(const Point2d& p, const Mat& K);
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> RenderWindow;
	vtkSmartPointer<vtkRenderer> Renderer;
	vtkSmartPointer<vtkActor> RcReWoCActor;
	vtkSmartPointer<vtkActor> RcReWCActor;
	string WorkSpace;

	Size image_size;

	vector<Mat> rvecsMatOfLeftCamera;
	vector<Mat> tvecsMatOfLeftCamera;

	Mat LeftCameraMatrix;
	Mat LeftCameradistCoeffs;

	vector<Mat> rvecsMatOfRightCamera;
	vector<Mat> tvecsMatOfRightCamera;
	Mat RightCameraMatrix;
	Mat RightCameradistCoeffs;

	Mat R, T, E, F, R1, R2, P1, P2, Q;  //stero calibrition Result


	QTextBrowser *textBrowser;
	QVTKOpenGLNativeWidget* qvtkOpenGLWidget_Big;
	QLabel* Rec_ImageCount_label;
	int IMGCOUNT;

	int proj_w;
	int proj_h;
	int cam_w;//sysParams.camL.imgROI.width;
	int cam_h;//sysParams.camL.imgROI.height;


	int n_cols, n_rows;			    		// n_cols:列有多少组图像
	int col_shift, row_shift;				// col_shift:列平移值
	int DownSample;                         //舍弃格雷码低位的个数
	vector<Mat> gray_codes;					// 格雷码图像

	bool whetherUseSpaceCoding;
	int Th;

	vector<Mat> CameraImgsLeft;	 // left 拍照得到的图片
	vector<Mat> CameraImgsRight; // right拍照得到的图片

	Mat LeftBRGImage;
	Mat RightBGRImage;

	vector<Point2f> ptsInLeft, ptsInRight; //左右匹配点对

	vector<Point3f> PointCloud;

	//pixeltopoints

	int depth ;
	QLabel * Depth_Label;
	QSlider * Rec_Depth_HorizontalSlider;
	vtkSmartPointer<vtkActor> WordPolydataActor;
	vtkSmartPointer<vtkPolyData> WordPolydata;
public slots:
	void LoadParameters();
	void LoadImages();
	void Reconstruction();
	void TextureMapping();
	void PixeltoPoint();

	void DepthChanged(int);

	void UseSpaceCoding(bool);
signals:
	void AfterLoadParameters(bool);
	void AfterReconstruction(bool);

};
class Point2fCmp
{
public:
	bool operator()(const Point2f& lhs, const Point2f& rhs) const
	{
		if (lhs.x<rhs.x)
			return true;
		if (lhs.x == rhs.x && lhs.y<rhs.y)
			return true;
		return false;
	}
};
// 匹配点对
struct TwoVec
{
	vector<Point2f> lpts;
	vector<Point2f> rpts;
};
#endif // !ThreeDimensionReconstrction_H
