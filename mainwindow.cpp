#include "mainwindow.h"
#include <opencv2\opencv.hpp>
#include <Windows.h>
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	cap.open(0);
	if (!cap.isOpened())
	{
		cout << "could not open the VideoCapture!" << endl;
		system("PAUSE");

	}
	InitCameras();
	connect(ui->directoryBrowsePushButton, SIGNAL(clicked()), this, SLOT(directoryBrowsePushButton_clicked()));
	connect(ui->toolBox, SIGNAL(currentChanged(int)), this, SLOT(ToolBoxChanged(int)));
	connect(ui->openCameraPushButton, SIGNAL(clicked()), this, SLOT(OpenCamera()));
	connect(ui->closeCameraPushButton, SIGNAL(clicked()), this, SLOT(CloseCamera()));
	connect(ui->detectChessBoardCheckBox, SIGNAL(clicked(bool)), this, SLOT(detectChessBoardCheckBoxChanged(bool)));
	connect(ui->CaCa_CaptureImagePushButton, SIGNAL(clicked()), this, SLOT(CaptureImage()));
	connect(ui->CaCa_ClearAll, SIGNAL(clicked()), this, SLOT(ClearAll()));
	connect(ui->rectifyDeformationCheckBox, SIGNAL(clicked(bool)), this, SLOT(rectifyDeformationCheckBoxChanged(bool)));
	connect(ui->SteroRectifiyCheckBox, SIGNAL(clicked(bool)), this, SLOT(SteroRectifiyCheckBoxChanged(bool)));

	ui->toolBox->setCurrentIndex(0);
	this->IMGCOUNT = 0;
	LeftIMGCOUNT=0;
	RightIMGCOUNT=0;
	this->board_size= Size(4, 6);
	this->square_size= Size(36.5, 36.5);
	this->detectChessBoard = false;
	this->RectifyCameraDeformation = false;
	this->SteroRectifiy = false;
	this->findcorner_left=false;
	this->findcorner_right=false;
	this->stop = true;
	this->captureOneFrame=false;
	this->WorkSpace = "";
	LeftCameraMatrix = Mat::eye(3, 3, CV_64FC1);
	RightCameraMatrix = Mat::eye(3, 3, CV_64FC1);
	LeftCameradistCoeffs = (Mat_<double>(5, 1) << 0, 0, 0, 0, 0);
	RightCameradistCoeffs = (Mat_<double>(5, 1) << 0, 0, 0, 0, 0);

	DetectChessBoard_left_thred = new DetectChessBoard;

	DetectChessBoard_right_thred = new DetectChessBoard;
	DetectChessBoard_left_thred->board_size = board_size;
	std::cout << "left_borad_size" << DetectChessBoard_left_thred->board_size << std::endl;
	DetectChessBoard_right_thred->board_size = board_size;
	std::cout << "right_borad_size" << DetectChessBoard_right_thred->board_size << std::endl;
	m_timer = new QTimer(this);
	//设置定时器是否为单次触发。默认为 false 多次触发 
	m_timer->setSingleShot(false);
	step = 30;
	//定时器触发信号槽  
	connect(m_timer, SIGNAL(timeout()), this, SLOT(RenderAfreame()));

	CalibrateCameras_object = new CalibrateCameras;
	CalibrateCameras_object->textBrowser = ui->textBrowser;
	CalibrateCameras_object->board_size = board_size;
	CalibrateCameras_object->square_size = square_size;
	CalibrateCameras_object->qvtkOpenGLWidget_Big=ui->qvtkOpenGLWidget_Big;
	CalibrateCameras_object->CaCa_label_count=ui->CaCa_label_count;
	connect(ui->LeftCameraRadioButton, SIGNAL(clicked(bool)), this, SLOT(LeftCameraRadioButtonChanged(bool)));
	connect(ui->RightCameraRadioButton, SIGNAL(clicked(bool)), this, SLOT(RightCameraRadioButtonChanged(bool)));
	connect(ui->SteroCalibrationRadioButton, SIGNAL(clicked(bool)), this, SLOT(SteroCalibrationRadioButtonChanged(bool)));
	ui->LeftCameraRadioButton->setChecked(false);
	ui->RightCameraRadioButton->setChecked(false);
	ui->SteroCalibrationRadioButton->setChecked(false);
	CalibrateLeftCamera = false;
	CalibrateRightCamera = false;
	SteroCalibrate = false;
	connect(ui->CaCa_CalibritionPushButton, SIGNAL(clicked()), CalibrateCameras_object, SLOT(Calibrate()));
	connect(ui->CaCa_steroCalibritionPushButton, SIGNAL(clicked()), CalibrateCameras_object, SLOT(SteroCalibrate()));
	connect(ui->test_pushButton, SIGNAL(clicked()), CalibrateCameras_object, SLOT(test()));


	iftracking = false;

	Tracking_object = new Tracking;
	Tracking_object->textBrowser = ui->textBrowser;
	Tracking_object->qvtkOpenGLWidget_Big = ui->qvtkOpenGLWidget_Big;

	connect(ui->Track_LoadParameters, SIGNAL(clicked()), Tracking_object, SLOT(LoadParameters()));
	connect(Tracking_object, SIGNAL(AfterLoadParameters(bool)), this, SLOT(Track_AfterLoadParameters(bool)));
	connect(ui->Track_trackChessBoard, SIGNAL(clicked()), this, SLOT(StartTracking()));
	connect(ui->Track_StopTracking, SIGNAL(clicked()), this, SLOT(StopTracking()));
	ui->Track_trackChessBoard->setEnabled(false);
	ui->Track_StopTracking->setEnabled(false);
	tem_count = 0;

	proj_delay = 150;//ms
	ifCollectImages = false;
	Reconstruction_object = new ThreeDimensionReconstruction;
	Reconstruction_object->textBrowser = ui->textBrowser;
	Reconstruction_object->qvtkOpenGLWidget_Big = ui->qvtkOpenGLWidget_Big;
	Reconstruction_object->Rec_ImageCount_label = ui->Rec_ImageCount_label;
	Reconstruction_object->Depth_Label = ui->Rec_Depth_Label;
	connect(ui->Rec_LoadParametersPushButton, SIGNAL(clicked()), Reconstruction_object, SLOT(LoadParameters()));
	connect(Reconstruction_object, SIGNAL(AfterLoadParameters(bool)), this, SLOT(Rec_AfterLoadParameters(bool)));
	connect(ui->Rec_CollectImagesPushButton, SIGNAL(clicked()), this, SLOT(CollectGrayCodesImages()));
	connect(ui->Rec_LoadImagesPushButton, SIGNAL(clicked()), Reconstruction_object, SLOT(LoadImages()));
	connect(ui->Rec_reconstructionPushButton, SIGNAL(clicked()), Reconstruction_object, SLOT(Reconstruction()));
	connect(Reconstruction_object, SIGNAL(AfterReconstruction(bool)), this, SLOT(Rec_AfterReconstruction(bool)));
	connect(ui->Rec_TextureMapping, SIGNAL(clicked()), Reconstruction_object, SLOT(TextureMapping()));
	connect(ui->Rec_PixeltoPoint, SIGNAL(clicked()), Reconstruction_object, SLOT(PixeltoPoint()));
	connect(ui->Rec_Depth_HorizontalSlider, SIGNAL(valueChanged(int)), Reconstruction_object, SLOT(DepthChanged(int)));
	connect(ui->Rec_ProjectionDelaySpinBox, SIGNAL(valueChanged(int)), this, SLOT(ProjectionDelaChanged(int)));
	connect(ui->SpaceCodingRadioButton, SIGNAL(clicked(bool)), Reconstruction_object, SLOT(UseSpaceCoding(bool)));
	ui->Rec_CollectImagesPushButton->setEnabled(false);
	ui->Rec_LoadImagesPushButton->setEnabled(false);
	ui->Rec_reconstructionPushButton->setEnabled(false);
	ui->Rec_TextureMapping->setEnabled(false);
	ui->Rec_PixeltoPoint->setEnabled(false);
	ui->Rec_Depth_HorizontalSlider->setEnabled(false);
	Reconstruction_object->Rec_Depth_HorizontalSlider = ui->Rec_Depth_HorizontalSlider;
	proj_w = 1280;
	proj_h = 720;

	Reconstruction_object->CameraImgsLeft.clear();
	Reconstruction_object->CameraImgsRight.clear();

	Reconstruction_object->proj_w = proj_w;
	Reconstruction_object->proj_h = proj_h;
	Reconstruction_object->DownSample = 2;
	Reconstruction_object->generateGrayCodes(proj_w, proj_h, true, true);
	Reconstruction_object->Th = 100;
	BrightParam = 1;
	gray_codes = Reconstruction_object->gray_codes;

}

MainWindow::~MainWindow()
{
	delete DetectChessBoard_left_thred;
	delete DetectChessBoard_right_thred;
	delete m_timer;
	delete CalibrateCameras_object;
	delete Tracking_object;
	delete Reconstruction_object;

}
void MainWindow::InitCameras()
{
	Camera_Two = new Camera;

	Camera_Two->Name = "TwoCamera";
	//Camera_Two->board_size = board_size;


	tSdkCameraDevInfo  sCameraList[2];
	int iCameraNums = 1;

	//获得设备信息
	if (CameraEnumerateDevice(sCameraList, &iCameraNums) != CAMERA_STATUS_SUCCESS)
	{
		ui->textBrowser->setText("No camera was found!\n");
		std::cout << "No camera was found!\n" << std::endl;
	}
	else
	{

		Camera_Two->CamInit(sCameraList[0]);

	}

}
void MainWindow::ToolBoxChanged(int status)
{
	std::cout << status << endl;
	/*switch (status)
	{
	case 0: {
		if (Tracking_object)
		{
			Tracking_object->CloseCamera();
			delete Tracking_object;
		}
		CalibrateCameras_object = new CalibrateCameras;
		CalibrateCameras_object->LeftLabel = ui->label_Left;
		CalibrateCameras_object->RightLabel = ui->label_Right;
		CalibrateCameras_object->textBrowser = ui->textBrowser;
		CalibrateCameras_object->board_size = Size(4, 6);
		CalibrateCameras_object->square_size = Size(21.25, 21.25);
		CalibrateCameras_object->qvtkOpenGLWidget_Big = ui->qvtkOpenGLWidget_Big;
		ui->CaCa_label_count->setText("0");
		CalibrateCameras_object->CaCa_label_count = ui->CaCa_label_count;
		connect(ui->CaCa_openCameraPushButton, SIGNAL(clicked()), CalibrateCameras_object, SLOT(OpenCamera()));
		connect(ui->CaCa_closeCameraPushButton, SIGNAL(clicked()), CalibrateCameras_object, SLOT(CloseCamera()));
		connect(ui->CaCa_detectChessBoardCheckBox, SIGNAL(clicked(bool)), CalibrateCameras_object, SLOT(detectChessBoardCheckBoxChanged(bool)));
		connect(ui->CaCa_CaptureImagePushButton, SIGNAL(clicked()), CalibrateCameras_object, SLOT(CaptureImage()));
		connect(ui->CaCa_ClearAll, SIGNAL(clicked()), CalibrateCameras_object, SLOT(ClearAll()));
		connect(ui->CaCa_CalibritionPushButton, SIGNAL(clicked()), CalibrateCameras_object, SLOT(Calibrate()));
		connect(ui->CaCa_steroCalibritionPushButton, SIGNAL(clicked()), CalibrateCameras_object, SLOT(SteroCalibrate()));

	}
	case 1:
	{
		if (CalibrateCameras_object)
		{
			CalibrateCameras_object->CloseCamera();
			delete CalibrateCameras_object;
		}
		Tracking_object = new Tracking;
		Tracking_object->LeftLabel = ui->label_Left;
		Tracking_object->RightLabel = ui->label_Right;
		Tracking_object->textBrowser = ui->textBrowser;
		Tracking_object->board_size = Size(4, 6);
		Tracking_object->square_size = Size(21.25, 21.25);
		Tracking_object->qvtkOpenGLWidget_Big = ui->qvtkOpenGLWidget_Big;
		connect(ui->Track_openCameraPushButton, SIGNAL(clicked()), Tracking_object, SLOT(OpenCamera()));
		connect(ui->Track_closeCameraPushButton, SIGNAL(clicked()), Tracking_object, SLOT(CloseCamera()));
		connect(ui->Track_detectChessBoardCheckBox, SIGNAL(clicked(bool)), Tracking_object, SLOT(detectChessBoardCheckBoxChanged(bool)));
	}
	}*/
}
void MainWindow::directoryBrowsePushButton_clicked()
{
	QString fileName = QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "../WorkSpace", QFileDialog::ShowDirsOnly);
	ui->directoryBrowserTextBrowser->setText(fileName);
	this->WorkSpace = fileName.toLocal8Bit();

	CalibrateCameras_object->WorkSpace = this->WorkSpace;
	Tracking_object->WorkSpace = this->WorkSpace;
	Reconstruction_object->WorkSpace = this->WorkSpace;

}
void MainWindow::OpenCamera()
{
	////DetectChessBoard_left_thred->start();
	////DetectChessBoard_right_thred->start();
	//启动或重启定时器, 并设置定时器时间：毫秒  
	m_timer->start(step);
	stop = false;


}
void MainWindow::CloseCamera()
{
	//DetectChessBoard_left_thred->m_stop=false;
	//DetectChessBoard_right_thred->m_stop=false;
	//停止渲染
	m_timer->stop();
	stop = true;

}
void MainWindow::detectChessBoardCheckBoxChanged(bool flag)
{
	this->detectChessBoard = flag;
	ifCollectImages = false;
}
inline bool exists_test1(const std::string& name) {
	if (FILE *file = fopen(name.c_str(), "r")) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}
void split(const std::string& s, std::vector<std::string>& sv, const char flag)
{
	sv.clear();
	std::istringstream iss(s);
	std::string temp;

	while (getline(iss, temp, flag)) {
		sv.push_back(temp);
	}
	return;
}
bool MainWindow::ReadCalibrationParameters(std::string CalibritionFilePath, int camera)
{

	if (!exists_test1(CalibritionFilePath))
	{
		std::cout << "calibrition file doesn't exix." << std::endl;
		return false;
	}
	ifstream fin(CalibritionFilePath.c_str());//读取保存标定结果的文件，以供矫正

	while (!fin.eof())
	{
		// every line
		std::string lineString;
		std::getline(fin, lineString);
		//std::cout << lineString << std::endl;
		if (lineString == "Camera Matrix: ")
		{
			for (int i = 0; i < 3; i++)
			{
				std::vector<std::string> lines;

				std::getline(fin, lineString);
				lineString = lineString.substr(1, lineString.length() - 2);
				//std::cout << lineString << std::endl;
				split(lineString, lines, ',');

				for (int j = 0; j < 3; j++)
				{
					if (camera == 0)//left
					{
						LeftCameraMatrix.at<double>(i, j) = atof(lines.at(j).c_str());
					}
					else if (camera == 1)//right
					{
						RightCameraMatrix.at<double>(i, j) = atof(lines.at(j).c_str());
					}
				}
			}

		}
		else if (lineString == "distortion Coeffs: ")
		{
			//std::vector<std::string> lines;
			//std::getline(fin, lineString);
			//lineString = lineString.substr(1, lineString.length() - 2);
			//std::cout << lineString << std::endl;
			//split(lineString, lines, ',');
			/*		for (int j = 0; j < lines.size(); j++)
			{
			printf( lines.at(j));
			}*/
			for (int j = 0; j < 5; j++)
			{
				std::getline(fin, lineString);
				if (camera == 0)//left
				{
					LeftCameradistCoeffs.at<double>(j) = atof(lineString.substr(1, lineString.length() - 2).c_str());
				}
				else if (camera == 1)//right
				{
					RightCameradistCoeffs.at<double>(j) = atof(lineString.substr(1, lineString.length() - 2).c_str());
				}

			}
		}
		else if (lineString == "rvecsMatOfCamera: ")
		{
			std::vector<std::string> lines;
			std::getline(fin, lineString);
			int count = stoi(lineString);
			//std::cout << lineString << std::endl<< count<<endl;
			for (int j = 0; j < count; j++)
			{
				Mat rvecsMatOfCamera = (Mat_<double>(3, 1) << 0, 0, 0);
				for (int z = 0; z < 3; z++)
				{
					std::getline(fin, lineString);
					//std::cout << lineString.substr(1, lineString.length() - 2)<< endl;
					rvecsMatOfCamera.at<double>(z) = atof(lineString.substr(1, lineString.length() - 2).c_str());


				}
				//std::cout << "rvecsMatOfCamera" << rvecsMatOfCamera << std::endl;
				if (camera == 0)//left
				{
					rvecsMatOfLeftCamera.push_back(rvecsMatOfCamera);
				}
				else if (camera == 1)//right
				{
					rvecsMatOfRightCamera.push_back(rvecsMatOfCamera);
				}
			}

		}
		else if (lineString == "tvecsMatOfCamera: ")
		{
			std::vector<std::string> lines;
			std::getline(fin, lineString);
			int count = stoi(lineString);
			for (int j = 0; j < count; j++)
			{
				Mat tvecsMatOfCamera = (Mat_<double>(3, 1) << 0, 0, 0);
				for (int z = 0; z < 3; z++)
				{
					std::getline(fin, lineString);
					tvecsMatOfCamera.at<double>(z) = atof(lineString.substr(1, lineString.length() - 2).c_str());


				}
				if (camera == 0)//left
				{
					tvecsMatOfLeftCamera.push_back(tvecsMatOfCamera);
				}
				else if (camera == 1)//right
				{
					tvecsMatOfRightCamera.push_back(tvecsMatOfCamera);
				}
			}

		}
	}
	return true;
}
bool MainWindow::ReadSteroCalibrationParameters(std::string CalibritionFilePath)
{
	if (!exists_test1(CalibritionFilePath))
	{
		std::cout << "sterocalibrition file doesn't exix." << std::endl;
		return false;
	}
	//R, T, E, F, R1, R2, P1, P2, Q;
	R = Mat(3, 3, CV_64FC1, Scalar::all(0));
	T = Mat(1, 3, CV_64FC1, Scalar::all(0));
	E = Mat(3, 3, CV_64FC1, Scalar::all(0));
	F = Mat(3, 3, CV_64FC1, Scalar::all(0));
	R1 = Mat(3, 3, CV_64FC1, Scalar::all(0));
	R2 = Mat(3, 3, CV_64FC1, Scalar::all(0));
	P1 = Mat(3, 4, CV_64FC1, Scalar::all(0));
	P2 = Mat(3, 4, CV_64FC1, Scalar::all(0));
	Q = Mat(4, 4, CV_64FC1, Scalar::all(0));

	ifstream fin(CalibritionFilePath.c_str());//读取保存标定结果的文件，以供矫正

	while (!fin.eof())
	{
		// every line
		std::string lineString;
		std::getline(fin, lineString);
		std::cout << lineString << std::endl;
		if (lineString == "R")
		{
			for (int i = 0; i < 3; i++)
			{
				std::vector<std::string> lines;

				std::getline(fin, lineString);
				lineString = lineString.substr(1, lineString.length() - 2);
				//std::cout << lineString << std::endl;
				split(lineString, lines, ',');

				for (int j = 0; j < 3; j++)
				{
					R.at<double>(i, j) = atof(lines.at(j).c_str());
				}
			}

		}
		else if (lineString == "T")
		{
			for (int j = 0; j < 3; j++)
			{
				std::vector<std::string> lines;
				std::getline(fin, lineString);
				lineString = lineString.substr(1, lineString.length() - 2);
				//std::cout << lineString << std::endl;
				//split(lineString, lines, ',');

				T.at<double>(j) = atof(lineString.c_str());
			}
		}
		else if (lineString == "E")
		{
			for (int i = 0; i < 3; i++)
			{
				std::vector<std::string> lines;

				std::getline(fin, lineString);
				lineString = lineString.substr(1, lineString.length() - 2);
				//std::cout << lineString << std::endl;
				split(lineString, lines, ',');

				for (int j = 0; j < 3; j++)
				{
					E.at<double>(i, j) = atof(lines.at(j).c_str());
				}
			}

		}
		else if (lineString == "F")
		{
			for (int i = 0; i < 3; i++)
			{
				std::vector<std::string> lines;

				std::getline(fin, lineString);
				lineString = lineString.substr(1, lineString.length() - 2);
				//std::cout << lineString << std::endl;
				split(lineString, lines, ',');

				for (int j = 0; j < 3; j++)
				{
					F.at<double>(i, j) = atof(lines.at(j).c_str());
				}
			}

		}
		else if (lineString == "R1")
		{
			for (int i = 0; i < 3; i++)
			{
				std::vector<std::string> lines;

				std::getline(fin, lineString);
				lineString = lineString.substr(1, lineString.length() - 2);
				//std::cout << lineString << std::endl;
				split(lineString, lines, ',');

				for (int j = 0; j < 3; j++)
				{
					R1.at<double>(i, j) = atof(lines.at(j).c_str());
				}
			}

		}
		else if (lineString == "R2")
		{
			for (int i = 0; i < 3; i++)
			{
				std::vector<std::string> lines;

				std::getline(fin, lineString);
				lineString = lineString.substr(1, lineString.length() - 2);
				//std::cout << lineString << std::endl;
				split(lineString, lines, ',');

				for (int j = 0; j < 3; j++)
				{
					R2.at<double>(i, j) = atof(lines.at(j).c_str());
				}
			}

		}
		else if (lineString == "P1")
		{
			for (int i = 0; i < 3; i++)
			{
				std::vector<std::string> lines;

				std::getline(fin, lineString);
				lineString = lineString.substr(1, lineString.length() - 2);
				//std::cout << lineString << std::endl;
				split(lineString, lines, ',');

				for (int j = 0; j < 4; j++)
				{
					P1.at<double>(i, j) = atof(lines.at(j).c_str());
				}
			}

		}
		else if (lineString == "P2")
		{
			for (int i = 0; i < 3; i++)
			{
				std::vector<std::string> lines;

				std::getline(fin, lineString);
				lineString = lineString.substr(1, lineString.length() - 2);
				//std::cout << lineString << std::endl;
				split(lineString, lines, ',');

				for (int j = 0; j < 4; j++)
				{
					P2.at<double>(i, j) = atof(lines.at(j).c_str());
				}
			}

		}
		else if (lineString == "Q")
		{
			for (int i = 0; i < 4; i++)
			{
				std::vector<std::string> lines;

				std::getline(fin, lineString);
				lineString = lineString.substr(1, lineString.length() - 2);
				//std::cout << lineString << std::endl;
				split(lineString, lines, ',');

				for (int j = 0; j < 4; j++)
				{
					Q.at<double>(i, j) = atof(lines.at(j).c_str());
				}
			}

		}


	}

	return true;
}
void MainWindow::rectifyDeformationCheckBoxChanged(bool flag)
{	
	if (flag)
	{
		if (WorkSpace == "")
		{
			ui->textBrowser->append(QString::fromLocal8Bit("Please specify the work space!\n"));
			ui->textBrowser->moveCursor(ui->textBrowser->textCursor().End);
			this->RectifyCameraDeformation = false;
			ui->rectifyDeformationCheckBox->setChecked(false);
			return;
		}
		bool leftflag = ReadCalibrationParameters(WorkSpace + "\\calibrationParametersOfLeftCamera.txt", 0);
		bool rightflag = ReadCalibrationParameters(WorkSpace + "\\calibrationParametersOfRightCamera.txt", 1);
		bool steroflag = ReadSteroCalibrationParameters(WorkSpace + "\\extrinsicParametersOfTwoCameras.txt");
		if (leftflag&&rightflag&&steroflag)
		{
			this->RectifyCameraDeformation = true;
			this->SteroRectifiy = false;
			ui->SteroRectifiyCheckBox->setChecked(false);
		}
		else
		{
			this->RectifyCameraDeformation = false;
			ui->rectifyDeformationCheckBox->setChecked(false);
		}
	}
	else
	{
		this->RectifyCameraDeformation = false;
	}


}

void MainWindow::SteroRectifiyCheckBoxChanged(bool flag)
{
	if (flag)
	{
		if (WorkSpace == "")
		{
			ui->textBrowser->append(QString::fromLocal8Bit("Please specify the work space!\n"));
			ui->textBrowser->moveCursor(ui->textBrowser->textCursor().End);
			this->SteroRectifiy = false;
			ui->SteroRectifiyCheckBox->setChecked(false);
			return;
		}
		bool leftflag = ReadCalibrationParameters(WorkSpace + "\\calibrationParametersOfLeftCamera.txt", 0);
		bool rightflag = ReadCalibrationParameters(WorkSpace + "\\calibrationParametersOfRightCamera.txt", 1);
		bool steroflag = ReadSteroCalibrationParameters(WorkSpace + "\\extrinsicParametersOfTwoCameras.txt");
		if (leftflag&&rightflag&&steroflag)
		{
			this->SteroRectifiy = true;

			this->RectifyCameraDeformation = false;
			ui->rectifyDeformationCheckBox->setChecked(false);
		}
		else
		{
			this->SteroRectifiy = false;
			ui->SteroRectifiyCheckBox->setChecked(false);
		}
	}
	else
	{
		this->SteroRectifiy = false;
	}
}
Mat MainWindow::rotateImage1(Mat img, int degree)
{
	degree = -degree;
	double angle = degree  * CV_PI / 180.; // 弧度  
	double a = sin(angle), b = cos(angle);
	int width = img.cols;
	int height = img.rows;
	int width_rotate = int(height * fabs(a) + width * fabs(b));
	int height_rotate = int(width * fabs(a) + height * fabs(b));
	//旋转数组map
	// [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]
	// [ m3  m4  m5 ] ===>  [ A21  A22   b2 ]
	float map[6];
	Mat map_matrix = Mat(2, 3, CV_32F, map);
	// 旋转中心
	Point2f center = Point2f(width / 2, height / 2);
	map_matrix=getRotationMatrix2D(center, degree, 1.0);
	map[2] += (width_rotate - width) / 2;
	map[5] += (height_rotate - height) / 2;
	Mat img_rotate;
	//对图像做仿射变换
	//CV_WARP_FILL_OUTLIERS - 填充所有输出图像的象素。
	//如果部分象素落在输入图像的边界外，那么它们的值设定为 fillval.
	//CV_WARP_INVERSE_MAP - 指定 map_matrix 是输出图像到输入图像的反变换，
	warpAffine(img, img_rotate, map_matrix, Size(width_rotate, height_rotate), 1, 0, 0);
	return img_rotate;
}

void MainWindow::RenderAfreame()
{

	//gettm(getTimeStamp());

	//int threshold1 = 0;
	//int threshold2 = 30;
	//int sizeOfSober = 3;
	if (this->ifCollectImages)
	{

		if (CountNumberOfGrayCodes <this->gray_codes.size())
		{
			cv::imshow(projWindName, this->gray_codes.at(CountNumberOfGrayCodes) * BrightParam);
		}
		else
		{
			Mat Texture=Mat(640, 360, CV_8UC1);
			Texture.setTo(255);
			cv::imshow(projWindName, Texture * BrightParam);
		}
		if (CountNumberOfGrayCodes == 0)
		{
			waitKey(500);
		}
		else
		{
			waitKey(proj_delay);
		}

	}

	Mat frame;
	frame = Camera_Two->ImageCapture();

	//std::cout << frame.channels() << std::endl;
	//bool ok = cap.read(frame);
	//if (!ok)
	//{
	//	return;
	//}
	frame = rotateImage1(frame, 180);
	flip(frame, frame, 1);
	Mat leftBGRImage =frame(Range(0,frame.rows),Range(0,frame.cols/2));
	Mat RightBGRImage = frame(Range(0, frame.rows), Range(frame.cols / 2, frame.cols));
	
	if (RectifyCameraDeformation)
	{
		Mat UndistortedRightImage;
		Mat UndistortedLeftImage;
		undistort(leftBGRImage, UndistortedLeftImage, LeftCameraMatrix, LeftCameradistCoeffs);
		undistort(RightBGRImage, UndistortedRightImage, RightCameraMatrix, RightCameradistCoeffs);

		//Mat LeftMapx, LeftMapy;
		//cv::initUndistortRectifyMap(LeftCameraMatrix, LeftCameradistCoeffs, R1, P1(cv::Rect(0, 0, 3, 3)), leftBGRImage.size(), CV_32FC1, LeftMapx, LeftMapy);
		//cv::remap(leftBGRImage, UndistortedLeftImage, LeftMapx, LeftMapy, INTER_NEAREST);

		//imshow("LeftMapx", LeftMapx);
		//imshow("LeftMapy", LeftMapy);

		//Mat RightMapx, RightMapy;
		//cv::initUndistortRectifyMap(RightCameraMatrix, RightCameradistCoeffs, R2, P2(cv::Rect(0, 0, 3, 3)), RightBGRImage.size(), CV_32FC1, RightMapx, RightMapy);
		//cv::remap(RightBGRImage, UndistortedRightImage, RightMapx, RightMapy, INTER_NEAREST);
		//imshow("RightMapx", RightMapx);
		//imshow("RightMapy", RightMapy);

		//imshow("UndistortedLeftImage", UndistortedLeftImage);
		//imshow("UndistortedRightImage", UndistortedRightImage);
		//waitKey(10);

		leftBGRImage = UndistortedLeftImage;
		RightBGRImage = UndistortedRightImage;
	}
	else if (SteroRectifiy)
	{
		Mat UndistortedRightImage;
		Mat UndistortedLeftImage;

		Mat LeftMapx, LeftMapy;
		cv::initUndistortRectifyMap(LeftCameraMatrix, LeftCameradistCoeffs, R1, P1(cv::Rect(0, 0, 3, 3)), leftBGRImage.size(), CV_32FC1, LeftMapx, LeftMapy);
		cv::remap(leftBGRImage, UndistortedLeftImage, LeftMapx, LeftMapy, INTER_NEAREST);

		//imshow("LeftMapx", LeftMapx);
		//imshow("LeftMapy", LeftMapy);

		Mat RightMapx, RightMapy;
		cv::initUndistortRectifyMap(RightCameraMatrix, RightCameradistCoeffs, R2, P2(cv::Rect(0, 0, 3, 3)), RightBGRImage.size(), CV_32FC1, RightMapx, RightMapy);
		cv::remap(RightBGRImage, UndistortedRightImage, RightMapx, RightMapy, INTER_NEAREST);
		//imshow("RightMapx", RightMapx);
		//imshow("RightMapy", RightMapy);

		//imshow("UndistortedLeftImage", UndistortedLeftImage);
		//imshow("UndistortedRightImage", UndistortedRightImage);
		//waitKey(10);

		leftBGRImage = UndistortedLeftImage;
		RightBGRImage = UndistortedRightImage;
	}
	//std::cout << leftBGRImage.cols << " " << leftBGRImage.rows << std::endl;
	//std::cout << RightBGRImage.cols << " " << RightBGRImage.rows << std::endl;
	//cv::resize(RightBGRImage, RightBGRImage, Size(RightBGRImage.cols / 2, RightBGRImage.rows / 2));
	//cv::resize(leftBGRImage, leftBGRImage, Size(leftBGRImage.cols / 2, leftBGRImage.rows / 2));

	//Mat leftBGRImage = frame(Range(0, frame.rows), Range(0, frame.cols / 2));
	//Mat RightBGRImage = frame(Range(0, frame.rows), Range(frame.cols / 2, frame.cols));
	//Mat leftBGRImage = frame;
	//Mat RightBGRImage = frame;
	//left camera
	cvtColor(leftBGRImage, leftImage, COLOR_BGR2GRAY);
	//right camera
	cvtColor(RightBGRImage, rightImage, COLOR_BGR2GRAY);



	if (this->detectChessBoard)
	{

		//Mat img1;


		//bool findcorner_left = findChessboardCorners(leftImage, board_size, image_points_buf_left);

		//bool findcorner_right = findChessboardCorners(rightImage, board_size, image_points_buf_right);
		//leftfinishedflag = false;
		//leftfinishedflag = false;
		findcorner_left = false;
		findcorner_right = false;
		DetectChessBoard_left_thred->flag = &findcorner_left;
		DetectChessBoard_left_thred->image_points_buf = &image_points_buf_left;
		DetectChessBoard_left_thred->image = &leftImage;
		//DetectChessBoard_left_thred->detect = true;

		DetectChessBoard_right_thred->flag = &findcorner_right;
		DetectChessBoard_right_thred->image_points_buf = &image_points_buf_right;
		DetectChessBoard_right_thred->image = &rightImage;
		//DetectChessBoard_right_thred->detect = true;
		DetectChessBoard_left_thred->start();
		DetectChessBoard_right_thred->start();
		DetectChessBoard_left_thred->wait();
		DetectChessBoard_right_thred->wait();


		//cout << "can not find chessboard corners in cmaera !\n"; //找不到角点  
		//flag = 2; //未成功检测棋盘格态												   /*return;*/
		//}
		//else
		//{
		/*亚像素精确化*/
		//cornerSubPix(leftImage, image_points_buf_left, Size(5, 5), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

		//find4QuadCornerSubpix(leftImage, image_points_buf_left, Size(5, 5)); //对粗提取的角点进行精确化  
		//find4QuadCornerSubpix(rightImage, image_points_buf_right, Size(5, 5)); //对粗提取的角点进行精确化  
		//flag = 1;//成功检测棋盘格态
		if (captureOneFrame&&SteroCalibrate&&findcorner_left&&findcorner_right)
		{
			//for (int index = 0; index < image_points_buf_left.size(); index++)
			//{
			//	//std::cout << "Write" << std::endl;
			//	CorresponedChessBoard_left.push_back(image_points_buf_left[index]);
			//	CorresponedChessBoard_right.push_back(image_points_buf_right[index]);
			//}
			std::cout << findcorner_left << std::endl;
			std::cout << findcorner_right << std::endl;
			WriteCapturedImage(leftImage,image_points_buf_left, string("SteroLeft"), IMGCOUNT);
			WriteCapturedImage(rightImage,image_points_buf_right, string("SteroRight"), IMGCOUNT);
			IMGCOUNT++;
			ui->CaCa_label_count->setText(QString(std::to_string(IMGCOUNT).c_str()));
		}
		else if (captureOneFrame&&CalibrateLeftCamera&&findcorner_left)
		{
			//for (int index = 0; index < image_points_buf_left.size(); index++)
			//{
			//	//std::cout << "Write" << std::endl;
			//	CorresponedChessBoard_left.push_back(image_points_buf_left[index]);
			//	CorresponedChessBoard_right.push_back(image_points_buf_right[index]);
			//}
			std::cout << findcorner_left << std::endl;
			std::cout << findcorner_right << std::endl;
			WriteCapturedImage(leftImage, image_points_buf_left, string("SingleLeft"), LeftIMGCOUNT);
			LeftIMGCOUNT++;
			ui->CaCa_label_count->setText(QString(std::to_string(LeftIMGCOUNT).c_str()));
		}
		else if (captureOneFrame&&CalibrateRightCamera&&findcorner_right)
		{
			//for (int index = 0; index < image_points_buf_left.size(); index++)
			//{
			//	//std::cout << "Write" << std::endl;
			//	CorresponedChessBoard_left.push_back(image_points_buf_left[index]);
			//	CorresponedChessBoard_right.push_back(image_points_buf_right[index]);
			//}
			std::cout << findcorner_left << std::endl;
			std::cout << findcorner_right << std::endl;
			WriteCapturedImage(rightImage,image_points_buf_right, string("SingleRight"), RightIMGCOUNT);
			RightIMGCOUNT++;
			ui->CaCa_label_count->setText(QString(std::to_string(RightIMGCOUNT).c_str()));
		}
		else if (iftracking&&findcorner_left&&findcorner_right)
		{
			//Tracking_object->image_points_buf_left = &Corner_buf_Left[tem_count];
			//Tracking_object->image_points_buf_right= &Corner_buf_Right[tem_count];
			//tem_count++;
			//tem_count = tem_count%IMGCOUNT;
				Tracking_object->image_points_buf_left = &image_points_buf_left;
				Tracking_object->image_points_buf_right = &image_points_buf_right;
				Tracking_object->TrackingAndRendering();
				
		}
		//cvtColor(leftImage, leftImage, COLOR_GRAY2BGR);
		//cvtColor(rightImage, rightImage, COLOR_GRAY2BGR);
		if (findcorner_left)
		{
			drawChessboardCorners(leftImage, board_size, image_points_buf_left, true);
		}
		if (findcorner_right)
		{
			drawChessboardCorners(rightImage, board_size, image_points_buf_right, true);
		}
		


	}
	//else
	//{
	//	RenderAfreame2();
	//}
	captureOneFrame = false;

	if (this->ifCollectImages)
	{
		std::string LeftImageName;
		std::string RightImageName;
		if (CountNumberOfGrayCodes <this->gray_codes.size())
		{
			LeftImageName = WorkSpace + "\\Reconstruction\\Leftimage" + std::to_string(CountNumberOfGrayCodes+1) + ".png";
			RightImageName = WorkSpace + "\\Reconstruction\\Rightimage" + std::to_string(CountNumberOfGrayCodes+1) + ".png";
			cv::imwrite(LeftImageName.c_str(), leftImage);
			cv::imwrite(RightImageName.c_str(), rightImage);
		}
		else
		{
			LeftImageName = WorkSpace + "\\Reconstruction\\LeftTextureImage.png";
			RightImageName = WorkSpace + "\\Reconstruction\\RightTextureImage.png";
			cv::imwrite(LeftImageName.c_str(), leftBGRImage);
			cv::imwrite(RightImageName.c_str(), RightBGRImage);
		}

		CountNumberOfGrayCodes++;
		if (CountNumberOfGrayCodes == this->gray_codes.size()+1)
		{
			ifCollectImages = false;
			cv::destroyWindow(projWindName);
			ui->detectChessBoardCheckBox->setEnabled(true);
			
		}

	}

	QImage disImage1 = QImage((const unsigned char*)(leftImage.data), leftImage.cols, leftImage.rows, QImage::Format_Grayscale8).rgbSwapped();
	ui->label_Left->setPixmap(QPixmap::fromImage(disImage1.scaled(ui->label_Left->size(), Qt::KeepAspectRatio)));

	QImage disImage2 = QImage((const unsigned char*)(rightImage.data), rightImage.cols, rightImage.rows, QImage::Format_Grayscale8).rgbSwapped();
	ui->label_Right->setPixmap(QPixmap::fromImage(disImage2.scaled(ui->label_Right->size(), Qt::KeepAspectRatio)));


}
void MainWindow::CaptureImage()
{
	captureOneFrame = true;
}
void MainWindow::WriteCapturedImage(Mat& LeftCamera,vector<Point2f> &image_points_buf,String FileHeadname,int num)
{
	if (WorkSpace == "")
	{
		ui->textBrowser->append(QString::fromLocal8Bit("Please specify the work space!\n"));
		ui->textBrowser->moveCursor(ui->textBrowser->textCursor().End);
		return;
	}
	char s1[100], sc1[100];

	//leftCamera
	std::sprintf(s1, (WorkSpace + "\\" + FileHeadname +"CameraImage%d.png").c_str(), num);

	cv::imwrite(s1, LeftCamera);

	std::sprintf(sc1, (WorkSpace + "\\" + FileHeadname+ "CameraChessBoard%d.txt").c_str(), num);
	ofstream OutPut1(sc1);

	std::cout << image_points_buf.size() << std::endl;

	for (int index = 0; index < image_points_buf.size(); index++)
	{
		OutPut1 << image_points_buf.at(index).x << "," << image_points_buf.at(index).y << std::endl;
		//std::cout << "WRITE " << Name << std::endl;
	}
	OutPut1.close();
	image_points_buf.clear();

}
void MainWindow::ClearAll()
{
	if (SteroCalibrate)
	{
		IMGCOUNT = 0;
	}
	if (CalibrateLeftCamera)
	{
		LeftIMGCOUNT = 0;
	}
	if (CalibrateRightCamera)
	{
		RightIMGCOUNT = 0;
	}
	ui->CaCa_label_count->setText(QString(std::to_string(0).c_str()));
}
void MainWindow::LeftCameraRadioButtonChanged(bool flag)
{
	ui->RightCameraRadioButton->setChecked(false);
	ui->LeftCameraRadioButton->setChecked(flag);
	ui->RightCameraRadioButton->setEnabled(true);
	ui->LeftCameraRadioButton->setEnabled(false);
	CalibrateLeftCamera = true;
	CalibrateRightCamera = false;

	ui->SteroCalibrationRadioButton->setChecked(false);
	ui->SteroCalibrationRadioButton->setEnabled(true);
	SteroCalibrate = false;
	ui->CaCa_label_count->setText(QString(std::to_string(LeftIMGCOUNT).c_str()));

}
void MainWindow::RightCameraRadioButtonChanged(bool flag)
{
	ui->RightCameraRadioButton->setChecked(flag);
	ui->LeftCameraRadioButton->setChecked(false);
	ui->RightCameraRadioButton->setEnabled(false);
	ui->LeftCameraRadioButton->setEnabled(true);

	CalibrateLeftCamera = false;
	CalibrateRightCamera = true;

	ui->SteroCalibrationRadioButton->setChecked(false);
	ui->SteroCalibrationRadioButton->setEnabled(true);
	
	SteroCalibrate = false;
	ui->CaCa_label_count->setText(QString(std::to_string(RightIMGCOUNT).c_str()));
}
void MainWindow::SteroCalibrationRadioButtonChanged(bool flag)
{

	ui->LeftCameraRadioButton->setChecked(false);
	ui->LeftCameraRadioButton->setEnabled(true);

	ui->RightCameraRadioButton->setChecked(false);
	ui->RightCameraRadioButton->setEnabled(true);

	CalibrateLeftCamera = false;
	CalibrateRightCamera = false;

	ui->SteroCalibrationRadioButton->setChecked(flag);
	ui->SteroCalibrationRadioButton->setEnabled(false);

	SteroCalibrate = true;

	ui->CaCa_label_count->setText(QString(std::to_string(IMGCOUNT).c_str()));
}
void MainWindow::test()
{	
	vtkSmartPointer<vtkRenderer> Renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> RenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
	RenderWindow->AddRenderer(Renderer);
	RenderWindow->SetSize(1080, 960);


	vtkSmartPointer<vtkPLYReader>PLYReader = vtkSmartPointer<vtkPLYReader>::New();
	PLYReader->SetFileName("D:\\projects\\3DReconstruction\\MVCamera\\image\\image7\\Reconstruction\\result.ply");
	PLYReader->Update();
	vtkSmartPointer<vtkPolyData>ReconstructionResult = vtkSmartPointer<vtkPolyData>::New();
	ReconstructionResult = PLYReader->GetOutput();
	vtkSmartPointer<vtkCylinderSource> Cylinder = vtkSmartPointer<vtkCylinderSource>::New();
	Cylinder->SetHeight(30);
	Cylinder->SetRadius(15);
	Cylinder->SetCenter(0, 0, 0);
	Cylinder->Update();

	vtkSmartPointer<vtkTransform> TransformForCylinder = vtkSmartPointer<vtkTransform>::New();
	TransformForCylinder->RotateX(90);

	vtkSmartPointer<vtkTransformPolyDataFilter> TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	TransformFilter->SetTransform(TransformForCylinder);
	TransformFilter->SetInputData(Cylinder->GetOutput());
	TransformFilter->Update();


	vtkSmartPointer<vtkCubeSource> Cube = vtkSmartPointer<vtkCubeSource>::New();
	Cube->SetCenter(0, 0, -30);
	Cube->SetXLength(30);
	Cube->SetYLength(30);
	Cube->SetZLength(30);
	Cube->Update();

	vtkSmartPointer<vtkAppendPolyData> AppendPolyDataFilter = vtkSmartPointer<vtkAppendPolyData>::New();
	AppendPolyDataFilter->AddInputData(TransformFilter->GetOutput());
	AppendPolyDataFilter->AddInputData(Cube->GetOutput());
	AppendPolyDataFilter->Update();

	vtkSmartPointer<vtkPolyData>Camera1 = vtkSmartPointer<vtkPolyData>::New();
	Camera1->DeepCopy(AppendPolyDataFilter->GetOutput());


	vtkSmartPointer<vtkPolyDataMapper> MapperOfCamera1 = vtkSmartPointer<vtkPolyDataMapper>::New();
	MapperOfCamera1->SetInputData(Camera1);

	vtkSmartPointer<vtkActor> ActorCamera1 = vtkSmartPointer<vtkActor>::New();
	ActorCamera1->SetMapper(MapperOfCamera1);
	ActorCamera1->GetProperty()->SetColor(1, 0, 0);
	Renderer->AddActor(ActorCamera1);
	vtkSmartPointer<vtkDelaunay2D> Delaunay2D = vtkSmartPointer<vtkDelaunay2D>::New();
	Delaunay2D->SetInputData(ReconstructionResult);
	Delaunay2D->Update();

	vtkSmartPointer<vtkPolyDataMapper> Mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
	Mapper1->SetInputData(Delaunay2D->GetOutput());

	vtkSmartPointer<vtkActor> Actor = vtkSmartPointer<vtkActor>::New();
	Actor->SetMapper(Mapper1);
	Renderer->AddActor(Actor);

	ui->qvtkOpenGLWidget_Big->SetRenderWindow(RenderWindow);
	ui->qvtkOpenGLWidget_Big->show();
	ui->qvtkOpenGLWidget_Big->update();
}

void MainWindow::Track_AfterLoadParameters(bool flag)
{
	if (flag)
	{
		ui->Track_trackChessBoard->setEnabled(true);
		ui->Track_StopTracking->setEnabled(false);
	}
}
void MainWindow::StartTracking()
{
	if (stop)
	{
		ui->textBrowser->append(QString::fromLocal8Bit("Please Open the Camera!\n"));
		ui->textBrowser->moveCursor(ui->textBrowser->textCursor().End);
		return;
	}
	if (!detectChessBoard)
	{
		ui->textBrowser->append(QString::fromLocal8Bit("Please Click the \"Detec ChessBoard\" Check Box!\n"));
		ui->textBrowser->moveCursor(ui->textBrowser->textCursor().End);
		return;
	}
	//ReadImagesAndChessBoardCoordinates();
	Tracking_object->RenderInitialization();
	//Tracking_object->SetRenderWindow();

	ui->Track_trackChessBoard->setEnabled(false);
	ui->Track_StopTracking->setEnabled(true);
	ui->detectChessBoardCheckBox->setEnabled(false);
	iftracking = true;
}
void MainWindow::StopTracking()
{
	iftracking = false;
	ui->Track_trackChessBoard->setEnabled(true);
	ui->Track_StopTracking->setEnabled(false);
	ui->detectChessBoardCheckBox->setEnabled(true);
}

void MainWindow::ReadImagesAndChessBoardCoordinates()
{
	std::string String = "Read Image and chess board....\n";
	QString WriteQString = QString::fromLocal8Bit(String.c_str());
	ui->textBrowser->append(WriteQString);
	ui->textBrowser->moveCursor(ui->textBrowser->textCursor().End);

	Corner_buf_Left.clear(); //缓存每幅图像上检测到的角点
	Corner_buf_Right.clear(); //缓存每幅图像上检测到的角点
							  //								   /*提取角点*/
	SourceImagesOfLeftCamera.clear();
	SourceImagesOfRightCamera.clear();

	IMGCOUNT = 0;
	//文件句柄  
	intptr_t  hFile = 0;
	//文件信息，_finddata_t需要io.h头文件  
	struct _finddata_t fileinfo;
	std::string p;

	if ((hFile = _findfirst(p.assign(WorkSpace).append("\\*.*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				continue;
			}
			else
			{
				std::string FileName = fileinfo.name;
				if (FileName.find("RightCamera") != string::npos && FileName.find("txt") != string::npos)
				{
					std::vector<std::string> SubStrings;
					split(FileName, SubStrings, '.');
					std::string RightImagePath = WorkSpace + "\\" + FileName;
					std::string ImageNum = SubStrings.at(0).substr(21, SubStrings.at(0).length() - 1);
					std::string LeftImagePath = "LeftCameraChessBoard" + ImageNum;
					LeftImagePath += ".txt";
					std::cout << FileName << "," << LeftImagePath << std::endl;
					LeftImagePath = WorkSpace + "\\" + LeftImagePath;

					fstream _file;
					_file.open(LeftImagePath, ios::in);
					if (_file)
					{
						IMGCOUNT += 1;
						char s1[100], s2[100];
						std::sprintf(s1, (WorkSpace + "\\" + "LeftCameraImage%d.png").c_str(), atoi(ImageNum.c_str()));
						std::sprintf(s2, (WorkSpace + "\\" + "RightCameraImage%d.png").c_str(), atoi(ImageNum.c_str()));
						Mat inputImage1 = imread(s1, 1);
						Mat inputImage2 = imread(s2, 1);
						SourceImagesOfLeftCamera.push_back(inputImage1);
						SourceImagesOfRightCamera.push_back(inputImage2);
						image_size.width = inputImage1.cols;
						image_size.height = inputImage1.rows;
						ifstream InPut1(LeftImagePath.c_str());
						ifstream InPut2(RightImagePath.c_str());

						vector<Point2f> tem_Corner_buf_1;
						vector<Point2f> tem_Corner_buf_2;
						while (!InPut1.eof())
						{
							// every line
							std::string lineString;
							std::getline(InPut1, lineString);
							std::vector<std::string> lines;
							split(lineString, lines, ',');
							if (lines.size() == 2)
							{
								Point2f Corner;
								Corner.x = atof(lines.at(0).c_str());
								Corner.y = atof(lines.at(1).c_str());
								tem_Corner_buf_1.push_back(Corner);
							}

						}
						while (!InPut2.eof())
						{
							// every line
							std::string lineString;
							std::getline(InPut2, lineString);
							std::vector<std::string> lines;
							split(lineString, lines, ',');
							if (lines.size() == 2)
							{
								Point2f Corner;
								Corner.x = atof(lines.at(0).c_str());
								Corner.y = atof(lines.at(1).c_str());
								tem_Corner_buf_2.push_back(Corner);
							}
						}

						Corner_buf_Left.push_back(tem_Corner_buf_1);
						Corner_buf_Right.push_back(tem_Corner_buf_2);
					}
				}

			}
		} while (!_findnext(hFile, &fileinfo));
		_findclose(hFile);
	}


	ui->CaCa_label_count->setText(QString(std::to_string(IMGCOUNT).c_str()));
}
void MainWindow::Rec_AfterLoadParameters(bool flag)
{
	ui->Rec_CollectImagesPushButton->setEnabled(flag);
	ui->Rec_LoadImagesPushButton->setEnabled(flag);
	ui->Rec_reconstructionPushButton->setEnabled(flag);
}
void MainWindow::Rec_AfterReconstruction(bool flag)
{
	ui->Rec_TextureMapping->setEnabled(true);
	ui->Rec_PixeltoPoint->setEnabled(true);
}

void isDirExist(std::string path)
{

	//const char* dir = "D:\\ASDF\\123.txt";   //_access也可用来判断文件是否存在

	if (_access(path.c_str(), 0) == -1)
	{
		_mkdir(path.c_str());
	}
}
void MainWindow::CollectGrayCodesImages()
{
	if (stop)
	{
		ui->textBrowser->append(QString::fromLocal8Bit("Please Open the Camera!\n"));
		ui->textBrowser->moveCursor(ui->textBrowser->textCursor().End);
		return;
	}

	isDirExist(WorkSpace + "\\Reconstruction");

	ui->detectChessBoardCheckBox->setChecked(false);
	detectChessBoardCheckBoxChanged(false);
	ui->detectChessBoardCheckBox->setEnabled(false);


	// 投影区域的控制
	namedWindow(projWindName, WINDOW_NORMAL);
	resizeWindow(projWindName, proj_w, proj_h);
	moveWindow(projWindName, 1920, 0);

	gray_codes = Reconstruction_object->gray_codes;
	CountNumberOfGrayCodes = 0;

	//cv::imshow(projWindName, gray_codes.at(0)*BrightParam);
	//waitKey(300);

	//CloseCamera();
	//step = 500;
	//OpenCamera();
	ifCollectImages = true;
}
void MainWindow::ProjectionDelaChanged(int value)
{
	proj_delay = value;
}