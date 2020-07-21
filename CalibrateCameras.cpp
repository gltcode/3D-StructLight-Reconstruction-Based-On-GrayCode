#include"CalibrateCameras.h"

CalibrateCameras::CalibrateCameras()
{

	WorkSpace = "";


	IMGCOUNT = 0;
	LeftIMGCOUNT = 0;
	RightIMGCOUNT = 0;
	LeftCameraMatrix = Mat::eye(3, 3, CV_64FC1);
	RightCameraMatrix = Mat::eye(3, 3, CV_64FC1);
	LeftCameradistCoeffs = (Mat_<double>(5, 1) << 0, 0, 0, 0, 0);
	RightCameradistCoeffs = (Mat_<double>(5, 1) << 0, 0, 0, 0, 0);

	CalibrateThred= new CalibrateCamerasThred;
	connect(CalibrateThred, SIGNAL(FinishedCalibrate(double,double)), this, SLOT(AfterCalibrate(double,double)));

	SteroCalibrateThred = new SteroCalibrateCamerasThred;
	connect(SteroCalibrateThred, SIGNAL(FinishedSteroCalibration(double)), this, SLOT(AfterSteroCalibration(double)));

}

CalibrateCameras::~CalibrateCameras()
{

	delete CalibrateThred;
	delete SteroCalibrateThred;
}

std::time_t getTimeStamp()
{
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
	std::time_t timestamp = tmp.count();
	//std::time_t timestamp = std::chrono::system_clock::to_time_t(tp);
	return timestamp;
}
int last_time = 0;
std::tm* gettm(std::time_t timestamp)
{
	std::time_t milli = timestamp/*+ (std::time_t)8*60*60*1000*/;//此处转化为东八区北京时间，如果是其它时区需要按需求修改
	auto mTime = std::chrono::milliseconds(milli);
	auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(mTime);
	auto tt = std::chrono::system_clock::to_time_t(tp);
	std::tm* now = std::gmtime(&tt);
	//printf("%4d年%02d月%02d日 %02d:%02d:%02d.%d\n", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, milli % 1000);

	std::cout << milli % 1000 - last_time << endl;
	last_time = milli % 1000;
	return now;
}

bool CalibrateCameras::ReadImagesAndChessBoardCoordinates()
{
	std::string String = "Read Image and chess board....\n";
	QString WriteQString = QString::fromLocal8Bit(String.c_str());
	textBrowser->append(WriteQString);
	textBrowser->moveCursor(textBrowser->textCursor().End);

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
				if (FileName.find("SteroRightCameraChessBoard") != string::npos && FileName.find("txt") != string::npos)
				{
					std::vector<std::string> SubStrings;
					split(FileName, SubStrings, '.');
					std::string RightImagePath = WorkSpace + "\\" + FileName;
					std::string ImageNum = SubStrings.at(0).substr(26, SubStrings.at(0).length() - 1);
					std::string LeftImagePath = "SteroLeftCameraChessBoard" + ImageNum;
					LeftImagePath += ".txt";
					std::cout << FileName << "," << LeftImagePath << std::endl;
					LeftImagePath = WorkSpace + "/" + LeftImagePath;

					fstream _file;
					_file.open(LeftImagePath, ios::in);
					if (_file)
					{
						IMGCOUNT += 1;
						char s1[100], s2[100];
						std::sprintf(s1, (WorkSpace + "\\" + "SteroLeftCameraImage%d.png").c_str(), atoi(ImageNum.c_str()));
						std::sprintf(s2, (WorkSpace + "\\" + "SteroRightCameraImage%d.png").c_str(), atoi(ImageNum.c_str()));
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


	CaCa_label_count->setText(QString(std::to_string(IMGCOUNT).c_str()));
	if (IMGCOUNT == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}
bool CalibrateCameras::ReadLeftImagesAndChessBoardCoordinates()
{
	std::string String = "Read Left Image and chess board....\n";
	QString WriteQString = QString::fromLocal8Bit(String.c_str());
	textBrowser->append(WriteQString);
	textBrowser->moveCursor(textBrowser->textCursor().End);

	Corner_buf_Left.clear(); //缓存每幅图像上检测到的角点
							  //								   /*提取角点*/
	SourceImagesOfLeftCamera.clear();

	LeftIMGCOUNT = 0;
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
				if (FileName.find("SingleLeftCamera") != string::npos && FileName.find("txt") != string::npos)
				{
					std::vector<std::string> SubStrings;
					split(FileName, SubStrings, '.');
					std::string LeftImagePath = WorkSpace + "\\" + FileName;
					std::string ImageNum = SubStrings.at(0).substr(20, SubStrings.at(0).length() - 1);
					std::cout << FileName << std::endl;
					LeftIMGCOUNT += 1;
					char s1[100];
					std::sprintf(s1, (WorkSpace + "\\" + "SingleLeftCameraImage%d.png").c_str(), atoi(ImageNum.c_str()));
					Mat inputImage1 = imread(s1, 1);
					SourceImagesOfLeftCamera.push_back(inputImage1);
					image_size.width = inputImage1.cols;
					image_size.height = inputImage1.rows;
					ifstream InPut1(LeftImagePath.c_str());

					vector<Point2f> tem_Corner_buf_1;
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
					
					Corner_buf_Left.push_back(tem_Corner_buf_1);
					
				}

			}
		} while (!_findnext(hFile, &fileinfo));
		_findclose(hFile);
	}


	CaCa_label_count->setText(QString(std::to_string(LeftIMGCOUNT).c_str()));
	return true;
}
bool CalibrateCameras::ReadRightImagesAndChessBoardCoordinates()
{
	std::string String = "Read Right Image and chess board....\n";
	QString WriteQString = QString::fromLocal8Bit(String.c_str());
	textBrowser->append(WriteQString);
	textBrowser->moveCursor(textBrowser->textCursor().End);

	Corner_buf_Right.clear(); //缓存每幅图像上检测到的角点
							  //								   /*提取角点*/
	SourceImagesOfRightCamera.clear();

	RightIMGCOUNT = 0;
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
				if (FileName.find("SingleRightCamera") != string::npos && FileName.find("txt") != string::npos)
				{
					std::vector<std::string> SubStrings;
					split(FileName, SubStrings, '.');
					std::string RightImagePath = WorkSpace + "\\" + FileName;
					std::string ImageNum = SubStrings.at(0).substr(21, SubStrings.at(0).length() - 1);
					std::cout << FileName << std::endl;
					RightIMGCOUNT += 1;
					char s1[100];
					std::sprintf(s1, (WorkSpace + "\\" + "SingleRightCameraImage%d.png").c_str(), atoi(ImageNum.c_str()));
					Mat inputImage1 = imread(s1, 1);
					SourceImagesOfRightCamera.push_back(inputImage1);
					image_size.width = inputImage1.cols;
					image_size.height = inputImage1.rows;
					ifstream InPut1(RightImagePath.c_str());

					vector<Point2f> tem_Corner_buf_1;
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

					Corner_buf_Right.push_back(tem_Corner_buf_1);

				}

			}
		} while (!_findnext(hFile, &fileinfo));
		_findclose(hFile);
	}


	CaCa_label_count->setText(QString(std::to_string(RightIMGCOUNT).c_str()));
	
	return true;
}

void CalibrateCameras::split(const std::string& s, std::vector<std::string>& sv, const char flag)
{
	sv.clear();
	std::istringstream iss(s);
	std::string temp;

	while (getline(iss, temp, flag)) {
		sv.push_back(temp);
	}
	return;
}

void CalibrateCameras::Calibrate()
{

	if (WorkSpace == "")
	{
		textBrowser->append(QString::fromLocal8Bit("Please specify the work space!\n"));
		textBrowser->moveCursor(textBrowser->textCursor().End);
		return;
	}
	std::string String = "Start to calibrate camera....\n";
	QString WriteQString = QString::fromLocal8Bit(String.c_str());
	textBrowser->append(WriteQString);
	textBrowser->moveCursor(textBrowser->textCursor().End);
	bool leftreadsucess = ReadLeftImagesAndChessBoardCoordinates();
	bool rightreadsucess = ReadRightImagesAndChessBoardCoordinates();
	if (!leftreadsucess|| !rightreadsucess)
	{
		if (!leftreadsucess)
		{
			textBrowser->append(QString::fromLocal8Bit("Please Capture Enough Images For Calibrating Left Cameras!\n"));
		}
		if (!rightreadsucess)
		{
			textBrowser->append(QString::fromLocal8Bit("Please Capture Enough Images For Calibrating Right Cameras!\n"));
		}
		textBrowser->moveCursor(textBrowser->textCursor().End);
		return;
	}
	Left_object_points.clear(); //保存标定板上角点的三维坐标,为标定函数的第一个参数
						   //实际测量得到的标定板上每个棋盘格的大小，这里其实没测，就假定了一个值，感觉影响不是太大，后面再研究下

	for (int t = 0; t < LeftIMGCOUNT; t++)
	{
		vector<Point3f> tempPointSet;
		for (int i = 0; i < board_size.height; i++)
		{
			for (int j = 0; j < board_size.width; j++)
			{
				Point3f realPoint;
				//假设标定板放在世界坐标系中z=0的平面上
				realPoint.x = i*square_size.height;
				realPoint.y = j*square_size.width;
				realPoint.z = 0;
				tempPointSet.push_back(realPoint);
			}
		}
		Left_object_points.push_back(tempPointSet);
	}
	Right_object_points.clear(); //保存标定板上角点的三维坐标,为标定函数的第一个参数
	for (int t = 0; t < RightIMGCOUNT; t++)
	{
		vector<Point3f> tempPointSet;
		for (int i = 0; i < board_size.height; i++)
		{
			for (int j = 0; j < board_size.width; j++)
			{
				Point3f realPoint;
				//假设标定板放在世界坐标系中z=0的平面上
				realPoint.x = i*square_size.height;
				realPoint.y = j*square_size.width;
				realPoint.z = 0;
				tempPointSet.push_back(realPoint);
			}
		}
		Right_object_points.push_back(tempPointSet);
	}

	rvecsMatOfLeftCamera.clear();//每幅图像的旋转向量
	tvecsMatOfLeftCamera.clear();//每幅图像的平移向量

	rvecsMatOfRightCamera.clear();//每幅图像的旋转向量
	tvecsMatOfRightCamera.clear();//每幅图像的平移向量

	CalibrateThred->WorkSpace = WorkSpace;

	CalibrateThred->image_size = image_size;
	CalibrateThred->board_size = board_size;
	CalibrateThred->Left_object_points = &Left_object_points;
	CalibrateThred->Right_object_points = &Right_object_points;

	CalibrateThred->Left_rvecsMat = &rvecsMatOfLeftCamera;
	CalibrateThred->Left_tvecsMat = &tvecsMatOfLeftCamera;
	CalibrateThred->Left_Corner_buf = &Corner_buf_Left;
	CalibrateThred->Left_CameraMatrix = &LeftCameraMatrix;
	CalibrateThred->Left_CameradistCoeffs = &LeftCameradistCoeffs;

	CalibrateThred->Right_rvecsMat = &rvecsMatOfRightCamera;
	CalibrateThred->Right_tvecsMat = &tvecsMatOfRightCamera;
	CalibrateThred->Right_Corner_buf = &Corner_buf_Right;
	CalibrateThred->Right_CameraMatrix = &RightCameraMatrix;
	CalibrateThred->Right_CameradistCoeffs = &RightCameradistCoeffs;
	CalibrateThred->start();
	
}
void CalibrateCameras::AfterCalibrate(double leftAverageError,double rightAverageError)
{
	std::string String = "(Left) Average calibration error is " + std::to_string(leftAverageError) + " pixels.\n";
	textBrowser->append(QString::fromLocal8Bit(String.c_str()));
	textBrowser->moveCursor(textBrowser->textCursor().End);

	
	String = "(Right) Average calibration error is " + std::to_string(rightAverageError) + " pixels.\n";
	textBrowser->append(QString::fromLocal8Bit(String.c_str()));
	textBrowser->moveCursor(textBrowser->textCursor().End);
	CalibrateThred->wait();
	//emit RenderSingleCameraCalibritionResult();
	RenderPlaneAndCamera();
	String = "Calibrition Finished.\n";
	textBrowser->append(QString::fromLocal8Bit(String.c_str()));
	textBrowser->moveCursor(textBrowser->textCursor().End);
	std::cout << "Calibrition Finished" << endl;
}
void CalibrateCameras::RenderPlaneAndCamera()
{
	std::default_random_engine random(time(NULL));
	std::uniform_real_distribution<double> dis2(0.0, 1.0);
	double leftCamera[4] = { 0.0, 0.0, 0.5, 1.0 };
	double rightCamera[4] = { 0.5, 0.0, 1.0, 1.0 };
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> RenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

	for (int i = 0; i < 2; i++)
	{
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

		vtkSmartPointer<vtkPolyDataMapper> Mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
		Mapper1->SetInputData(TransformFilter->GetOutput());


		vtkSmartPointer<vtkPolyDataMapper> Mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
		Mapper2->SetInputData(Cube->GetOutput());

		vtkSmartPointer<vtkActor> Actor1 = vtkSmartPointer<vtkActor>::New();
		Actor1->SetMapper(Mapper1);

		vtkSmartPointer<vtkActor> Actor2 = vtkSmartPointer<vtkActor>::New();
		Actor2->SetMapper(Mapper2);
		vtkSmartPointer<vtkRenderer> Renderer = vtkSmartPointer<vtkRenderer>::New();
		Renderer->AddActor(Actor1);
		Renderer->AddActor(Actor2);
		if (i == 0)
		{
			Renderer->SetViewport(leftCamera);
		}
		else if (i == 1)
		{
			Renderer->SetViewport(rightCamera);
		}
		int NumberOfImage = 0;
		if (i == 0)
		{
			NumberOfImage = rvecsMatOfLeftCamera.size();
		}
		else if (i == 1)
		{
			NumberOfImage = rvecsMatOfRightCamera.size();
		}
		for (int index = 0; index <NumberOfImage; index++)
		{

			vtkSmartPointer<vtkPlaneSource> Plane = vtkSmartPointer<vtkPlaneSource>::New();
			Plane->SetOrigin(100, -100, 0);
			Plane->SetPoint1(100, 100, 0);
			Plane->SetPoint2(-100, -100, 0);
			Plane->SetResolution(100, 100);
			Plane->Update();
			vtkSmartPointer<vtkMatrix4x4> MatrixRT1 = vtkSmartPointer<vtkMatrix4x4>::New();
			if (i == 0)
			{
				TransformTV2TM(rvecsMatOfLeftCamera.at(index), tvecsMatOfLeftCamera.at(index), MatrixRT1);
			}
			else if (i == 1)
			{
				TransformTV2TM(rvecsMatOfRightCamera.at(index), tvecsMatOfRightCamera.at(index), MatrixRT1);
			}
			double randomcolor[3];
			randomcolor[0] = dis2(random);
			randomcolor[1] = dis2(random);
			randomcolor[2] = dis2(random);
			vtkSmartPointer<vtkTransform> Transform1 = vtkSmartPointer<vtkTransform>::New();
			Transform1->SetMatrix(MatrixRT1);
			//if (1)
			//{
			//	for (int num_sphere = 0; num_sphere < object_points.at(index).size(); num_sphere++)
			//	{
			//		vtkSmartPointer<vtkSphereSource>Sphere = vtkSmartPointer<vtkSphereSource>::New();
			//		Sphere->SetRadius(1);
			//		Sphere->SetCenter(object_points.at(index).at(num_sphere).x, object_points.at(index).at(num_sphere).y, object_points.at(index).at(num_sphere).z);
			//		Sphere->Update();

			//		vtkSmartPointer<vtkTransformPolyDataFilter> TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			//		TransformFilter->SetTransform(Transform1);
			//		TransformFilter->SetInputData(Sphere->GetOutput());
			//		TransformFilter->Update();

			//		vtkSmartPointer<vtkPolyDataMapper> Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
			//		Mapper->SetInputData(TransformFilter->GetOutput());
			//		vtkSmartPointer<vtkActor> Actor = vtkSmartPointer<vtkActor>::New();
			//		Actor->SetMapper(Mapper);
			//		Actor->GetProperty()->SetColor(randomcolor);
			//		Renderer->AddActor(Actor);
			//	}
			//}
			vtkSmartPointer<vtkTransformPolyDataFilter> TransformFilter1 = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			TransformFilter1->SetTransform(Transform1);
			TransformFilter1->SetInputData(Plane->GetOutput());
			TransformFilter1->Update();

			vtkSmartPointer<vtkPolyDataMapper> Mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
			Mapper1->SetInputData(TransformFilter1->GetOutput());



			vtkSmartPointer<vtkActor> Actor1 = vtkSmartPointer<vtkActor>::New();
			Actor1->SetMapper(Mapper1);
			Actor1->GetProperty()->SetColor(randomcolor);

			Renderer->AddActor(Actor1);
		}
		double AXIS_LEN = 50;
		vtkSmartPointer<vtkAxesActor> AxesActor = vtkSmartPointer<vtkAxesActor>::New();
		AxesActor->SetPosition(0, 0, 0);
		AxesActor->SetTotalLength(AXIS_LEN, AXIS_LEN, AXIS_LEN);
		AxesActor->SetShaftType(0);
		AxesActor->SetAxisLabels(1);
		AxesActor->SetCylinderRadius(0.02);

		Renderer->AddActor(AxesActor);

		RenderWindow->AddRenderer(Renderer);
	}

	qvtkOpenGLWidget_Big->SetRenderWindow(RenderWindow);
	qvtkOpenGLWidget_Big->show();
	qvtkOpenGLWidget_Big->update();
}
void CalibrateCameras::TransformTV2TM(Mat rvecsMatOfCamera, Mat tvecsMatOfCamera, vtkSmartPointer<vtkMatrix4x4> RTM)
{

	Mat outputArray;
	Rodrigues(rvecsMatOfCamera, outputArray);

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			RTM->SetElement(i, j, outputArray.at<double>(i, j));
		}
	}

	for (int i = 0; i < 3; i++)
	{
		RTM->SetElement(i, 3, tvecsMatOfCamera.at<double>(i));
		RTM->SetElement(3, i, 0);
	}

	RTM->SetElement(3, 3, 1);

}

void CalibrateCameras::test()
{

	if (WorkSpace == "")
	{
		textBrowser->append(QString::fromLocal8Bit("Please specify the work space!\n"));
		textBrowser->moveCursor(textBrowser->textCursor().End);
		return;
	}
	bool leftflag = ReadCalibrationParameters(WorkSpace + "\\calibrationParametersOfLeftCamera.txt", 0);
	bool rightflag = ReadCalibrationParameters(WorkSpace + "\\calibrationParametersOfRightCamera.txt", 1);
	ReadImagesAndChessBoardCoordinates();
	Mat undistortedImage;
	for (int i = 0; i < SourceImagesOfLeftCamera.size(); i++)
	{
		undistort(SourceImagesOfRightCamera.at(i), undistortedImage, LeftCameraMatrix, LeftCameradistCoeffs);

		imshow("a", SourceImagesOfRightCamera.at(i));
		imshow("b", undistortedImage);
		waitKey();
	}
	Mat rvecsMatOfCamera=(Mat_<double>(3,1)<< 2.005615813467458,2.263300567467571,-0.33225708381235);
	Mat tvecsMatOfCamera= (Mat_<double>(3, 1) << 80.44241528902096,-36.7213736136511,478.4597011816459);
	vtkSmartPointer<vtkMatrix4x4> MatrixRT1 = vtkSmartPointer<vtkMatrix4x4>::New();
	TransformTV2TM(rvecsMatOfCamera, tvecsMatOfCamera, MatrixRT1);
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
bool CalibrateCameras::ReadCalibrationParameters(std::string CalibritionFilePath, int camera)
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
					else if (camera==1)//right
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
void CalibrateCameras::SteroCalibrate()
{
	if (WorkSpace == "")
	{
		textBrowser->append(QString::fromLocal8Bit("Please specify the work space!\n"));
		textBrowser->moveCursor(textBrowser->textCursor().End);
		return;
	}
	std::string String = "Start stero-calibration....\n";
	QString WriteQString = QString::fromLocal8Bit(String.c_str());
	textBrowser->append(WriteQString);
	textBrowser->moveCursor(textBrowser->textCursor().End);
	if (!ReadImagesAndChessBoardCoordinates())
	{
		textBrowser->append(QString::fromLocal8Bit("Please Capture Enough Images For Stero-Calibration!\n"));
		textBrowser->moveCursor(textBrowser->textCursor().End);
		return;
	}
	LeftCameraMatrix = Mat::eye(3, 3, CV_64FC1);
	RightCameraMatrix = Mat::eye(3, 3, CV_64FC1);
	LeftCameradistCoeffs = (Mat_<double>(5, 1) << 0, 0, 0, 0, 0);
	RightCameradistCoeffs = (Mat_<double>(5, 1) << 0, 0, 0, 0, 0);
	bool leftflag =ReadCalibrationParameters(WorkSpace + "\\calibrationParametersOfLeftCamera.txt",0);
	bool rightflag=ReadCalibrationParameters(WorkSpace + "\\calibrationParametersOfRightCamera.txt",1);
	if (leftflag&&rightflag)
	{
		std::cout << "LeftCamera" << std::endl;
		std::cout << LeftCameraMatrix << std::endl;
		std::cout << LeftCameradistCoeffs << std::endl;

		std::cout << "RightCamera" << std::endl;
		std::cout << RightCameraMatrix << std::endl;
		std::cout << RightCameradistCoeffs << std::endl;

	}
	else
	{
		std::string String = "Please Calibrate Left and Right Cameras!\n";
		textBrowser->append(QString::fromLocal8Bit(String.c_str()));
		textBrowser->moveCursor(textBrowser->textCursor().End);
		return;
	}
	object_points.clear(); //保存标定板上角点的三维坐标,为标定函数的第一个参数
						   //实际测量得到的标定板上每个棋盘格的大小，这里其实没测，就假定了一个值，感觉影响不是太大，后面再研究下
	for (int t = 0; t < IMGCOUNT; t++)
	{
		vector<Point3f> tempPointSet;
		for (int i = 0; i < board_size.height; i++)
		{
			for (int j = 0; j < board_size.width; j++)
			{
				Point3f realPoint;
				//假设标定板放在世界坐标系中z=0的平面上
				realPoint.x = i*square_size.height;
				realPoint.y = j*square_size.width;
				realPoint.z = 0;
				tempPointSet.push_back(realPoint);
			}
		}
		object_points.push_back(tempPointSet);
	}

	SteroCalibrateThred->image_size = image_size;
	SteroCalibrateThred->board_size = board_size;
	SteroCalibrateThred->object_points = &object_points;

	SteroCalibrateThred->Corner_buf_Left = &Corner_buf_Left;
	SteroCalibrateThred->Corner_buf_Right = &Corner_buf_Right;
	SteroCalibrateThred->LeftCameraMatrix = &LeftCameraMatrix;
	SteroCalibrateThred->LeftCameradistCoeffs = &LeftCameradistCoeffs;
	SteroCalibrateThred->RightCameraMatrix = &RightCameraMatrix;
	SteroCalibrateThred->RightCameradistCoeffs = &RightCameradistCoeffs;
	SteroCalibrateThred->R = &R;
	SteroCalibrateThred->T = &T;
	SteroCalibrateThred->WorkSpace = WorkSpace;
	SteroCalibrateThred->start();


}
void CalibrateCameras::AfterSteroCalibration(double reprojectionerror)
{
	std::string String = "The reprojection error =" + std::to_string(reprojectionerror) + " pixels.\n";
	QString WriteQString = QString::fromLocal8Bit(String.c_str());
	textBrowser->append(WriteQString);
	textBrowser->moveCursor(textBrowser->textCursor().End);
	SteroCalibrateThred->wait();

	RenderBothCamerasAndPlanes();

	String = "Stero-Calibrition Finished.\n";
	textBrowser->append(QString::fromLocal8Bit(String.c_str()));
	textBrowser->moveCursor(textBrowser->textCursor().End);
	std::cout << "Stero-Calibrition Finished." << endl;
}
void CalibrateCameras::RenderBothCamerasAndPlanes()
{
	std::default_random_engine random(time(NULL));
	std::uniform_real_distribution<double> dis2(0.0, 1.0);

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

	vtkSmartPointer<vtkPolyData>Camera2 = vtkSmartPointer<vtkPolyData>::New();
	Camera2->DeepCopy(AppendPolyDataFilter->GetOutput());

	vtkSmartPointer<vtkMatrix4x4> RTMatrixBetweenTwoCamera = vtkSmartPointer<vtkMatrix4x4>::New();
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			RTMatrixBetweenTwoCamera->SetElement(i, j, R.at<double>(i, j));
			//std::cout << R.at<double>(i, j) << std::endl;
		}
	}
	for (int i = 0; i < 3; i++)
	{
		RTMatrixBetweenTwoCamera->SetElement(i, 3, T.at<double>(i));
		//std::cout << T.at<double>(i) << std::endl;
		RTMatrixBetweenTwoCamera->SetElement(3, i, 0);
	}
	RTMatrixBetweenTwoCamera->SetElement(3, 3, 1); 
	std::cout << *RTMatrixBetweenTwoCamera << std::endl;
	RTMatrixBetweenTwoCamera->Invert();
	std::cout << *RTMatrixBetweenTwoCamera << std::endl;
	vtkSmartPointer<vtkTransform> TransformBetweenTwoCamera = vtkSmartPointer<vtkTransform>::New();
	TransformBetweenTwoCamera->SetMatrix(RTMatrixBetweenTwoCamera);

	vtkSmartPointer<vtkTransformPolyDataFilter> BetweenTwoCameraTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	BetweenTwoCameraTransformFilter->SetTransform(TransformBetweenTwoCamera);
	BetweenTwoCameraTransformFilter->SetInputData(Camera2);
	BetweenTwoCameraTransformFilter->Update();

	vtkSmartPointer<vtkPolyDataMapper> MapperOfCamera1 = vtkSmartPointer<vtkPolyDataMapper>::New();
	MapperOfCamera1->SetInputData(Camera1);

	vtkSmartPointer<vtkActor> ActorCamera1 = vtkSmartPointer<vtkActor>::New();
	ActorCamera1->SetMapper(MapperOfCamera1);
	ActorCamera1->GetProperty()->SetColor(1, 0, 0);

	vtkSmartPointer<vtkPolyDataMapper> MapperOfCamera2 = vtkSmartPointer<vtkPolyDataMapper>::New();
	MapperOfCamera2->SetInputData(BetweenTwoCameraTransformFilter->GetOutput());

	vtkSmartPointer<vtkActor> ActorCamera2 = vtkSmartPointer<vtkActor>::New();
	ActorCamera2->SetMapper(MapperOfCamera2);
	ActorCamera2->GetProperty()->SetColor(0, 0, 1);

	vtkSmartPointer<vtkRenderer> Renderer = vtkSmartPointer<vtkRenderer>::New();
	Renderer->AddActor(ActorCamera1);
	Renderer->AddActor(ActorCamera2);

	double AXIS_LEN = 250;
	vtkSmartPointer<vtkAxesActor> AxesActor = vtkSmartPointer<vtkAxesActor>::New();
	AxesActor->SetPosition(0, 0, 0);
	AxesActor->SetTotalLength(AXIS_LEN, AXIS_LEN, AXIS_LEN);
	AxesActor->SetShaftType(0);
	AxesActor->SetAxisLabels(1);
	AxesActor->SetCylinderRadius(0.02);

	Renderer->AddActor(AxesActor);

	vtkSmartPointer<vtkGenericOpenGLRenderWindow> RenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
	RenderWindow->AddRenderer(Renderer);
	std::cout << "index" << std::endl;
	vtkSmartPointer<vtkActor> LastActor1 = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> LastActor2 = vtkSmartPointer<vtkActor>::New();
	int numIndex = rvecsMatOfLeftCamera.size() < rvecsMatOfRightCamera.size() ? rvecsMatOfLeftCamera.size() : rvecsMatOfRightCamera.size();
	for (int index = 0; index < numIndex; index++)
	{
		std::cout <<index << std::endl;
		if (index == 40)
		{
			int a = 1;
		}
		vtkSmartPointer<vtkPlaneSource> Plane1 = vtkSmartPointer<vtkPlaneSource>::New();
		Plane1->SetOrigin(100, -100, 0);
		Plane1->SetPoint1(100, 100, 0);
		Plane1->SetPoint2(-100, -100, 0);
		Plane1->SetResolution(100, 100);
		Plane1->Update();

		vtkSmartPointer<vtkPlaneSource> Plane2 = vtkSmartPointer<vtkPlaneSource>::New();
		Plane2->SetOrigin(100, -100, 0);
		Plane2->SetPoint1(100, 100, 0);
		Plane2->SetPoint2(-100, -100, 0);
		Plane2->SetResolution(100, 100);
		Plane2->Update();

		vtkSmartPointer<vtkMatrix4x4> MatrixRT1 = vtkSmartPointer<vtkMatrix4x4>::New();
		TransformTV2TM(rvecsMatOfLeftCamera.at(index), tvecsMatOfLeftCamera.at(index), MatrixRT1);
		//MatrixRT1->Invert();
		vtkSmartPointer<vtkTransform> Transform1 = vtkSmartPointer<vtkTransform>::New();
		Transform1->SetMatrix(MatrixRT1);

		vtkSmartPointer<vtkMatrix4x4> MatrixRT2 = vtkSmartPointer<vtkMatrix4x4>::New();
		TransformTV2TM(rvecsMatOfRightCamera.at(index), tvecsMatOfRightCamera.at(index), MatrixRT2);
		//MatrixRT2->Invert();
		vtkSmartPointer<vtkTransform> Transform2 = vtkSmartPointer<vtkTransform>::New();
		Transform2->SetMatrix(MatrixRT2);

		vtkSmartPointer<vtkTransformPolyDataFilter> TransformFilter1 = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
		TransformFilter1->SetTransform(Transform1);
		TransformFilter1->SetInputData(Plane1->GetOutput());
		TransformFilter1->Update();

		vtkSmartPointer<vtkTransformPolyDataFilter> TransformFilter2 = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
		TransformFilter2->SetTransform(Transform2);
		TransformFilter2->SetInputData(Plane2->GetOutput());
		TransformFilter2->Update();

		vtkSmartPointer<vtkTransformPolyDataFilter> BetweenTwoCameraTransformFilter_temporary = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
		BetweenTwoCameraTransformFilter_temporary->SetTransform(TransformBetweenTwoCamera);
		BetweenTwoCameraTransformFilter_temporary->SetInputData(TransformFilter2->GetOutput());
		BetweenTwoCameraTransformFilter_temporary->Update();

		vtkSmartPointer<vtkPolyDataMapper> Mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
		Mapper1->SetInputData(TransformFilter1->GetOutput());

		vtkSmartPointer<vtkPolyDataMapper> Mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
		Mapper2->SetInputData(BetweenTwoCameraTransformFilter_temporary->GetOutput());


		double randomcolor[3];
		randomcolor[0] = dis2(random);
		randomcolor[1] = dis2(random);
		randomcolor[2] = dis2(random);
		vtkSmartPointer<vtkActor> Actor1 = vtkSmartPointer<vtkActor>::New();
		Actor1->SetMapper(Mapper1);
		Actor1->GetProperty()->SetColor(randomcolor);

		vtkSmartPointer<vtkActor> Actor2 = vtkSmartPointer<vtkActor>::New();
		Actor2->SetMapper(Mapper2);
		Actor2->GetProperty()->SetColor(randomcolor);

		Renderer->AddActor(Actor1);
		Renderer->AddActor(Actor2);

		if (0)
		{
			vtkSmartPointer<vtkPolyData> ChessBoard1 = vtkSmartPointer<vtkPolyData>::New();
			vtkSmartPointer<vtkPolyData> ChessBoard2 = vtkSmartPointer<vtkPolyData>::New();
			for (int num_sphere = 0; num_sphere < object_points.at(index).size(); num_sphere++)
			{
				vtkSmartPointer<vtkSphereSource>Sphere1 = vtkSmartPointer<vtkSphereSource>::New();
				Sphere1->SetRadius(5);
				Sphere1->SetCenter(object_points.at(index).at(num_sphere).x, object_points.at(index).at(num_sphere).y, object_points.at(index).at(num_sphere).z);
				Sphere1->Update();
				vtkSmartPointer<vtkAppendPolyData> AppendPolydata = vtkSmartPointer<vtkAppendPolyData>::New();
				AppendPolydata->AddInputData(ChessBoard1);
				AppendPolydata->AddInputData(Sphere1->GetOutput());
				AppendPolydata->Update();

				ChessBoard1->DeepCopy(AppendPolydata->GetOutput());
			}
			ChessBoard2->DeepCopy(ChessBoard1);

			vtkSmartPointer<vtkTransformPolyDataFilter> TransformFilter1 = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			TransformFilter1->SetTransform(Transform1);
			TransformFilter1->SetInputData(ChessBoard1);
			TransformFilter1->Update();

			vtkSmartPointer<vtkTransformPolyDataFilter> TransformFilter2 = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			TransformFilter2->SetTransform(Transform2);
			TransformFilter2->SetInputData(ChessBoard2);
			TransformFilter2->Update();

			vtkSmartPointer<vtkTransformPolyDataFilter> BetweenTwoCameraTransformFilter_temporary = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			BetweenTwoCameraTransformFilter_temporary->SetTransform(TransformBetweenTwoCamera);
			BetweenTwoCameraTransformFilter_temporary->SetInputData(TransformFilter2->GetOutput());
			BetweenTwoCameraTransformFilter_temporary->Update();

			double AverageCoordination1[3] = { 0,0,0 };
			double AverageCoordination2[3] = { 0,0,0 };
			for (int num_point = 0; num_point < ChessBoard1->GetNumberOfPoints(); num_point++)
			{
				double point1[3];
				double point2[3];
				TransformFilter1->GetOutput()->GetPoint(num_point, point1);
				BetweenTwoCameraTransformFilter_temporary->GetOutput()->GetPoint(num_point, point2);
				for (int i = 0; i < 3; i++)
				{
					AverageCoordination1[i] += point1[i];
					AverageCoordination2[i] += point2[i];
				}
			}

			for (int i = 0; i < 3; i++)
			{
				AverageCoordination1[i] /= ChessBoard1->GetNumberOfPoints();
				AverageCoordination2[i] /= ChessBoard2->GetNumberOfPoints();
			}
			std::cout << "The difference is " << AverageCoordination1[0] - AverageCoordination2[0] << "," << AverageCoordination1[1] - AverageCoordination2[1] << ","
				<< AverageCoordination1[2] - AverageCoordination2[2] << std::endl;
			vtkSmartPointer<vtkPolyDataMapper> Mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
			Mapper1->SetInputData(TransformFilter1->GetOutput());
			vtkSmartPointer<vtkActor> Actor1 = vtkSmartPointer<vtkActor>::New();
			Actor1->SetMapper(Mapper1);
			Actor1->GetProperty()->SetColor(randomcolor);

			vtkSmartPointer<vtkPolyDataMapper> Mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
			Mapper2->SetInputData(BetweenTwoCameraTransformFilter_temporary->GetOutput());
			vtkSmartPointer<vtkActor> Actor2 = vtkSmartPointer<vtkActor>::New();
			Actor2->SetMapper(Mapper2);
			Actor2->GetProperty()->SetColor(randomcolor);


			Renderer->AddActor(Actor1);
			Renderer->AddActor(Actor2);

		}

	}
	qvtkOpenGLWidget_Big->SetRenderWindow(RenderWindow);
	qvtkOpenGLWidget_Big->show();
	qvtkOpenGLWidget_Big->update();

}
