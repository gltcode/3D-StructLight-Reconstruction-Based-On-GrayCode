#include<Tracking.h>
Tracking::Tracking()
{

	WorkSpace = "";

	LeftCameraMatrix = Mat::eye(3, 3, CV_64FC1);
	RightCameraMatrix = Mat::eye(3, 3, CV_64FC1);
	LeftCameradistCoeffs = (Mat_<double>(5, 1) << 0, 0, 0, 0, 0);
	RightCameradistCoeffs = (Mat_<double>(5, 1) << 0, 0, 0, 0, 0);
	RenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
	RenderWindow->SetSize(1080, 960);
	Renderer = vtkSmartPointer<vtkRenderer>::New();
	LastActor = vtkSmartPointer<vtkActor>::New();
	Renderer->AddActor(LastActor);
	RenderWindow->AddRenderer(Renderer);
}

Tracking::~Tracking()
{
}

void Tracking::LoadParameters()
{
	if (WorkSpace == "")
	{
		textBrowser->append(QString::fromLocal8Bit("Please specify the work space!\n"));
		textBrowser->moveCursor(textBrowser->textCursor().End);
		emit AfterLoadParameters(false);
		return;
	}
	bool leftflag = ReadCalibrationParameters(WorkSpace + "\\calibrationParametersOfLeftCamera.txt", 0);
	bool rightflag = ReadCalibrationParameters(WorkSpace + "\\calibrationParametersOfRightCamera.txt", 1);
	bool steroflag=ReadSteroCalibrationParameters(WorkSpace + "\\extrinsicParametersOfTwoCameras.txt");
	if (!(leftflag&&rightflag&&steroflag))
	{
		std::string String = "Please Check the Calibration Files!\n";
		textBrowser->append(QString::fromLocal8Bit(String.c_str()));
		textBrowser->moveCursor(textBrowser->textCursor().End);
		emit AfterLoadParameters(false);
	}
	else
	{
		std::string String = "Sucessfuly load the calibration parameters!\n";
		textBrowser->append(QString::fromLocal8Bit(String.c_str()));
		textBrowser->moveCursor(textBrowser->textCursor().End);
		emit AfterLoadParameters(true);
	}
	return;
}
void Tracking::SetRenderWindow()
{
	this->qvtkOpenGLWidget_Big->SetRenderWindow(RenderWindow);
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
bool Tracking::ReadCalibrationParameters(std::string CalibritionFilePath, int camera)
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
bool Tracking::ReadSteroCalibrationParameters(std::string CalibritionFilePath)
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
	T1 = (Mat_<float>(3, 4) <<
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0);
	T2 = (Mat_<float>(3, 4) <<
		R.at<double>(0, 0), R.at<double>(0, 1), R.at<double>(0, 2), T.at<double>(0, 0),
		R.at<double>(1, 0), R.at<double>(1, 1), R.at<double>(1, 2), T.at<double>(0, 1),
		R.at<double>(2, 0), R.at<double>(2, 1), R.at<double>(2, 2), T.at<double>(0, 2)
		);
	return true;
}
void Tracking::split(const std::string& s, std::vector<std::string>& sv, const char flag)
{
	sv.clear();
	std::istringstream iss(s);
	std::string temp;

	while (getline(iss, temp, flag)) {
		sv.push_back(temp);
	}
	return;
}
void Tracking::TrackingAndRendering()
{
	vector<Point3d> OutPutPoints;

	//std::cout << "One Frame" << std::endl;
	triangulation(OutPutPoints);

	vtkSmartPointer<vtkPolyData>ChessBoard = vtkSmartPointer<vtkPolyData>::New();

	for (int j = 0; j < OutPutPoints.size(); j++)
	{
		vtkSmartPointer<vtkSphereSource> Sphere = vtkSmartPointer<vtkSphereSource>::New();
		Sphere->SetRadius(1);
		Sphere->SetCenter(OutPutPoints[j].x, OutPutPoints[j].y, OutPutPoints[j].z);
		Sphere->Update();

		//std::cout << OutPutPoints[j].x << " " << OutPutPoints[j].y << " " << OutPutPoints[j].z << endl;
		vtkSmartPointer<vtkAppendPolyData>AppendPolydata = vtkSmartPointer<vtkAppendPolyData>::New();
		AppendPolydata->AddInputData(ChessBoard);
		AppendPolydata->AddInputData(Sphere->GetOutput());
		AppendPolydata->Update();

		ChessBoard->DeepCopy(AppendPolydata->GetOutput());
	}


	vtkSmartPointer<vtkPolyDataMapper> Mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
	Mapper1->SetInputData(ChessBoard);

	vtkSmartPointer<vtkActor> Actor = vtkSmartPointer<vtkActor>::New();
	Actor->SetMapper(Mapper1);

	Renderer->RemoveActor(LastActor);
	LastActor = Actor;

	Renderer->AddActor(Actor);

	this->qvtkOpenGLWidget_Big->GetRenderWindow()->Render();
	//this->qvtkOpenGLWidget_Big->SetRenderWindow(RenderWindow);
	this->qvtkOpenGLWidget_Big->show();
	this->qvtkOpenGLWidget_Big->update();
}
void Tracking::triangulation(vector< Point3d >& points)
{
	vector<Point2d> pts_1, pts_2;
	for (int i = 0; i<image_points_buf_left->size(); i++)
	{
		// 将像素坐标转换至相机坐标
		pts_1.push_back(pixel2cam(image_points_buf_left->at(i), LeftCameraMatrix));
		pts_2.push_back(pixel2cam(image_points_buf_right->at(i), RightCameraMatrix));
	}

	Mat pts_4d;
	cv::triangulatePoints(T1, T2, pts_1, pts_2, pts_4d);
	//std::cout << pts_4d.size() << std::endl;
	//std::cout << pts_4d.type() << std::endl;
	// 转换成非齐次坐标
	for (int i = 0; i<pts_4d.cols; i++)
	{
		Mat x = pts_4d.col(i);
		x /= x.at<double>(3, 0); // 归一化
		Point3d p(
			x.at<double>(0, 0),
			x.at<double>(1, 0),
			x.at<double>(2, 0)
		);
		points.push_back(p);
	}
}
Point2d Tracking::pixel2cam(const Point2f& p, const Mat& K)
{
	return Point2d
	(
		(p.x - K.at<double>(0, 2)) / K.at<double>(0, 0),
		(p.y - K.at<double>(1, 2)) / K.at<double>(1, 1)
	);
}
void Tracking::RenderInitialization()
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
	RTMatrixBetweenTwoCamera->Invert();

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
	this->qvtkOpenGLWidget_Big->SetRenderWindow(RenderWindow);
}