#include<ThreeDimensionReconstruction.h>
ThreeDimensionReconstruction::ThreeDimensionReconstruction()
{
	WorkSpace = "";
	IMGCOUNT = 0;
	LeftCameraMatrix = Mat::eye(3, 3, CV_64FC1);
	RightCameraMatrix = Mat::eye(3, 3, CV_64FC1);
	LeftCameradistCoeffs = (Mat_<double>(5, 1) << 0, 0, 0, 0, 0);
	RightCameradistCoeffs = (Mat_<double>(5, 1) << 0, 0, 0, 0, 0);
	Th = 150;
	DownSample = 2;
	proj_w =1280;
	proj_h = 720;
	WordPolydataActor = NULL;
	RcReWoCActor=NULL;
	RcReWCActor=NULL;
	depth = 50;
	this->whetherUseSpaceCoding = false;
}

ThreeDimensionReconstruction::~ThreeDimensionReconstruction()
{
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
void ThreeDimensionReconstruction::LoadParameters()
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
	bool steroflag = ReadSteroCalibrationParameters(WorkSpace + "\\extrinsicParametersOfTwoCameras.txt");
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
bool ThreeDimensionReconstruction::ReadCalibrationParameters(std::string CalibritionFilePath, int camera)
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
bool ThreeDimensionReconstruction::ReadSteroCalibrationParameters(std::string CalibritionFilePath)
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

void ThreeDimensionReconstruction::UseSpaceCoding(bool flag)
{
	this->whetherUseSpaceCoding = flag;
	generateGrayCodes(proj_w, proj_h, true, true);
}
void ThreeDimensionReconstruction::split(const std::string& s, std::vector<std::string>& sv, const char flag)
{
	sv.clear();
	std::istringstream iss(s);
	std::string temp;

	while (getline(iss, temp, flag)) {
		sv.push_back(temp);
	}
	return;
}
void ThreeDimensionReconstruction::LoadImages()
{
	if (WorkSpace == "")
	{
		textBrowser->append(QString::fromLocal8Bit("Please specify the work space!\n"));
		textBrowser->moveCursor(textBrowser->textCursor().End);
		return;
	}
	std::string String = "Read Image and chess board....\n";
	QString WriteQString = QString::fromLocal8Bit(String.c_str());
	textBrowser->append(WriteQString);
	textBrowser->moveCursor(textBrowser->textCursor().End);

	CameraImgsLeft.clear();
	CameraImgsRight.clear();

	IMGCOUNT = 0;
	//文件句柄  
	for (int i = 0; i < this->gray_codes.size(); i++)
	{
		string LeftImagePath = "Leftimage" + std::to_string(i+1) + ".png";
		string RightImagePath = "Rightimage" + std::to_string(i + 1) + ".png";
		std::cout << RightImagePath << "," << LeftImagePath << std::endl;

		RightImagePath = WorkSpace + "\\Reconstruction\\" + RightImagePath;
		LeftImagePath = WorkSpace + "\\Reconstruction\\" + LeftImagePath;


		Mat inputImage1 = imread(LeftImagePath, IMREAD_GRAYSCALE);
		Mat inputImage2 = imread(RightImagePath, IMREAD_GRAYSCALE);
		CameraImgsLeft.push_back(inputImage1);
		CameraImgsRight.push_back(inputImage2);
		image_size.width = inputImage1.cols;
		image_size.height = inputImage1.rows;
		IMGCOUNT++;
		

	}
	string LeftImagePath = WorkSpace + "\\Reconstruction\\"+"LeftTextureImage.png";
	string RightImagePath = WorkSpace + "\\Reconstruction\\" + "RightTextureImage.png";
	LeftBRGImage = imread(LeftImagePath, IMREAD_COLOR);
	RightBGRImage= imread(RightImagePath, IMREAD_COLOR);
	Rec_ImageCount_label->setText(QString(std::to_string(IMGCOUNT).c_str()));
}
Mat ThreeDimensionReconstruction::rotateImage1(Mat img, int degree,int mode=1)
{
	degree = -degree;
	double angle = degree  * CV_PI / 180.; // 弧度  
	double a = sin(angle), b = cos(angle);
	int width = img.cols;
	int height = img.rows;
	int width_rotate;
	int height_rotate;
	if (mode == 0)
	{
		width_rotate = int(pow(2, 0.5)*(width / 2 + height / 2) / 2);
		height_rotate = int(pow(2, 0.5)*(width / 2 + height / 2) / 2);
	}
	else if (mode == 1 || mode == 2)
	{
		width_rotate = int(height * fabs(a) + width * fabs(b));
		height_rotate = int(width * fabs(a) + height * fabs(b));
	}

	//旋转数组map
	// [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]
	// [ m3  m4  m5 ] ===>  [ A21  A22   b2 ]
	float map[6];
	Mat map_matrix = Mat(2, 3, CV_32F, map);
	// 旋转中心
	Point2f center = Point2f(width / 2, height / 2);
	map_matrix = getRotationMatrix2D(center, degree, 1.0);
	std::cout << (width_rotate - width) / 2 << " " << (height_rotate - height) / 2 << std::endl;
	std::cout << map_matrix << std::endl;
	map_matrix.at<double>(0, 2) += (width_rotate - width) / 2;
	map_matrix.at<double>(1, 2) += (height_rotate - height) / 2;
	std::cout << map_matrix << std::endl;
	Mat img_rotate;
	//对图像做仿射变换
	//CV_WARP_FILL_OUTLIERS - 填充所有输出图像的象素。
	//如果部分象素落在输入图像的边界外，那么它们的值设定为 fillval.
	//CV_WARP_INVERSE_MAP - 指定 map_matrix 是输出图像到输入图像的反变换，
	warpAffine(img, img_rotate, map_matrix, Size(width_rotate, height_rotate), INTER_LINEAR, BORDER_CONSTANT, 0);
	if (mode == 2)
	{
		Range Range_rows = Range(int(width*fabs(a)), height_rotate - int(width*fabs(a)));
		Range Rangey_cols = Range(int(height*fabs(a)), width_rotate - int(height*fabs(a)));

		img_rotate = img_rotate(Range_rows, Rangey_cols);
	}
	return img_rotate;
}
bool ThreeDimensionReconstruction::generateGrayCodes(int width, int height,	// 图像的宽和高
	bool sl_scan_cols, bool sl_scan_rows)		// 是否打行列图
{


	if (this->whetherUseSpaceCoding)
	{
		int cubelength = 5;
		int RowSideCount = 16;
		int ColsSideCount = 32;
		Mat CodeImage(2 * RowSideCount*cubelength, ColsSideCount*cubelength, CV_8UC1, Scalar(0));
		//imshow("CodeImage1", CodeImage);


		for (int i = 0; i < RowSideCount; i++)
		{
			for (int rowOfCude = 0; rowOfCude < cubelength; rowOfCude++)
			{
				uchar * dataOfEachRow = CodeImage.ptr<uchar>(2 * i*cubelength + rowOfCude);
				for (int j = 0; j < ColsSideCount; j++)
				{
					uchar value;
					if (j % 2 == 1)
					{
						value = 128;
					}
					else
					{
						value = 0;
					}
					for (int k = 0; k < cubelength; k++)
					{
						dataOfEachRow[j*cubelength + k] = value;
					}
				}
			}
			for (int rowOfCude = 0; rowOfCude < cubelength; rowOfCude++)
			{
				uchar * dataOfEachRow = CodeImage.ptr<uchar>((2 * i + 1)*cubelength + rowOfCude);
				for (int j = 0; j < ColsSideCount; j++)
				{
					uchar value;
					if (j % 2 == 1)
					{
						value = 255;
					}
					else
					{
						value = 125;
					}
					for (int k = 0; k < cubelength; k++)
					{
						dataOfEachRow[j*cubelength + k] = value;
					}
				}
			}
			//for(int i)
		}
		RotatedRect rRect = RotatedRect(Point2f(CodeImage.rows / 2, CodeImage.cols / 2), Size2f(pow(2, 0.5) * 2 * RowSideCount*cubelength / 2, pow(2, 0.5)*ColsSideCount*cubelength / 2), 45); //定义一bai个旋转矩形
		Point2f vertices[4];
		rRect.points(vertices);//提取旋转矩形的四个角点du
		for (int i = 0; i<4; i++)
		{
			line(CodeImage, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0));//四个角点连成线，最终形成旋转的矩形。
		}

		vector<bool> firstRow = { 1,1,0,0,1,0,0,1,1,0,1,0,0,0,1,1 };
		vector<bool> secondRow = { 1,1,0,0,1,1,1,1,0,0,0,1,0,1,0,0 };

		for (int i = 0; i < firstRow.size(); i++)
		{
			if (i % 2 == 0)
			{
				bool tem_value = firstRow[i];
				firstRow[i] = secondRow[i];
				secondRow[i] = tem_value;
			}
		}

		for (int i = RowSideCount - 1; i < 2 * RowSideCount; i++) //i-th row
		{
			int inedx_colum = i - (RowSideCount - 1);  //the index of the row colored now
			int count = 0;   //the number of colored cube in each line
			int start_row = i;
			int start_col = i - (RowSideCount - 1);

			while (count<16)
			{
				int tem_value;

				if (inedx_colum % 2 == 0)
				{
					if (firstRow[count])
					{
						tem_value = -75;
					}
					else
					{
						tem_value = 75;
					}

				}
				else
				{
					if (secondRow[count])
					{
						tem_value = -75;
					}
					else
					{
						tem_value = 75;
					}
				}

				for (int k = 0; k < cubelength; k++)
				{
					uchar * DataOfEachRow = CodeImage.ptr<uchar>(start_row*cubelength + k);
					for (int num = start_col*cubelength; num < start_col*cubelength + cubelength; num++)
					{
						DataOfEachRow[num] += tem_value;
					}
				}

				start_row--;
				start_col++;
				count++;
			}
		}

		//imshow("CodeImage2", CodeImage);

		//GaussianBlur(CodeImage, CodeImage, Size(5, 5), 10, 10);
		blur(CodeImage, CodeImage, Size(5, 5));
		//imshow("CodeImage3", CodeImage);
		CodeImage = rotateImage1(CodeImage, 45,0);
		//imshow("CodeImage4", CodeImage);
		int Threshold = 150;
		for (int i = 0; i < CodeImage.rows; i++)
		{
			uchar * DataOfEachRow = CodeImage.ptr<uchar>(i);
			for (int j = 0; j < CodeImage.cols; j++)
			{
				if (DataOfEachRow[j] > Threshold)
				{
					DataOfEachRow[j] = 255;
				}
				else
				{
					DataOfEachRow[j] = 0;
				}
			}
		}
		//imshow("CodeImage5", CodeImage);
		int repeatrow = 12;
		int repeatcol = 19;
		Mat RepeatCodeImage(CodeImage.rows * repeatrow, CodeImage.cols * repeatcol, CV_8UC1, Scalar(0));
		for (int i = 0; i < repeatrow; i++)
		{
			for (int j = 0; j < repeatcol; j++)
			{
				for (int index_row = 0; index_row < CodeImage.rows; index_row++)
				{
					for (int index_col = 0; index_col < CodeImage.cols; index_col++)
					{
						RepeatCodeImage.at<uchar>(i*CodeImage.rows + index_row, j*CodeImage.cols + index_col)
							= CodeImage.at<uchar>(index_row, index_col);
					}
				}
			}
		}
		//imshow("CodeImage6", RepeatCodeImage);
		////RepeatCodeImage = rotateImage1(RepeatCodeImage, 10, 1);

		//imshow("CodeImage7", rotateImage1(RepeatCodeImage, 10, 1));

		RepeatCodeImage = rotateImage1(RepeatCodeImage, 10, 2);
		imshow("CodeImage8", RepeatCodeImage);
		gray_codes.clear();
		Mat emptyCode(RepeatCodeImage.rows, RepeatCodeImage.cols,CV_8UC1, Scalar(0));
		gray_codes.push_back(emptyCode);
		gray_codes.push_back(RepeatCodeImage);
		Mat inverseCode = 255 - RepeatCodeImage.clone();
		imshow("inverseCode", inverseCode);
		gray_codes.push_back(inverseCode);
	}
	else
	{
		vector<Mat> TemgGrayCodes;
		// Determine number of required codes and row/column offsets.
		if (sl_scan_cols) {
			n_cols = (int)ceil(log2(width));
			col_shift = (int)floor((pow(2.0, n_cols) - width) / 2);
		}
		else {
			n_cols = 0;
			col_shift = 0;
		}
		if (sl_scan_rows) {
			n_rows = (int)ceil(log2(height));
			row_shift = (int)floor((pow(2.0, n_rows) - height) / 2);
		}
		else {
			n_rows = 0;
			row_shift = 0;
		}

		// Allocate Gray codes.
		for (int i = 0; i<n_cols + n_rows + 1 - 2 * DownSample; i++)	// n_cols和n_rows发生变化
			TemgGrayCodes.push_back(Mat(height, width, CV_8UC1));
		int step1 = TemgGrayCodes[0].step[0];
		int step2 = TemgGrayCodes[0].step[1];

		// Define first code as a white image.
		TemgGrayCodes[0].setTo(255);

		// Define Gray codes for projector columns.列条纹
		for (int c = 0; c<width; c++) {
			for (int i = 0; i<n_cols - DownSample; i++) {
				uchar* data = (uchar*)TemgGrayCodes[i + 1].data;
				if (i>0)
					(uchar)*(data + c*step2) = (((c + col_shift) >> (n_cols - i - 1)) & 1) ^ (((c + col_shift) >> (n_cols - i)) & 1);
				else
					(uchar)*(data + c*step2) = (((c + col_shift) >> (n_cols - i - 1)) & 1);
				(uchar)*(data + c*step2) *= 255;
				for (int r = 1; r<height; r++)
					(uchar)*(data + r*step1 + c*step2) = (uchar)*(data + c*step2);
			}
		}

		// Define Gray codes for projector rows. 行条纹
		for (int r = 0; r<height; r++) {
			for (int i = 0; i<n_rows - DownSample; i++) {
				uchar* data = (uchar*)TemgGrayCodes[i + n_cols - DownSample + 1].data;
				if (i>0)
					(uchar)*(data + r*step1) = (((r + row_shift) >> (n_rows - i - 1)) & 1) ^ (((r + row_shift) >> (n_rows - i)) & 1);
				else
					(uchar)*(data + r*step1) = (((r + row_shift) >> (n_rows - i - 1)) & 1);
				(uchar)*(data + r*step1) *= 255;
				for (int c = 1; c<width; c++)
					(uchar)*(data + r*step1 + c*step2) = (uchar)*(data + r*step1);
			}
		}
		gray_codes.clear();
		for (int i = 0; i < TemgGrayCodes.size(); i++)
		{
			gray_codes.push_back(TemgGrayCodes[i]);
			//imshow("a", TemgGrayCodes[i]);
			//waitKey();
			Mat inverseCode = 255 - TemgGrayCodes[i].clone();
			gray_codes.push_back(inverseCode);
		}
	}

	//std::cout << gray_codes.size() << std::endl;
	// Return without errors.
	return 0;
}
bool ThreeDimensionReconstruction::decodeGrayCodes(int max_width, int max_height, vector<Mat>& GrayImage_codes,
	Mat& decoded_cols, Mat& decoded_rows, Mat& mask,
	int sl_thresh)
{
	int cam_width = GrayImage_codes[0].cols;
	int cam_height = GrayImage_codes[0].rows;

	// Allocate temporary variables.
	Mat gray_1(cam_height, cam_width, CV_8UC1);
	Mat gray_2(cam_height, cam_width, CV_8UC1);
	Mat bit_plane_1(cam_height, cam_width, CV_8UC1);
	Mat bit_plane_2(cam_height, cam_width, CV_8UC1);
	Mat temp(cam_height, cam_width, CV_8UC1);


	// Initialize image mask (indicates reconstructed pixels).
	mask.setTo(0);

	// Decode Gray codes for projector columns.
	decoded_cols.setTo(0);
	for (int i = 0; i<n_cols - DownSample; i++) {		// 
														// Decode bit-plane and update mask.
		if (GrayImage_codes[2 * (i + 1)].channels() == 3) {
			cvtColor(GrayImage_codes[2 * (i + 1)], gray_1, COLOR_RGB2GRAY);
			cvtColor(GrayImage_codes[2 * (i + 1) + 1], gray_2, COLOR_RGB2GRAY);
		}
		else {
			gray_1 = GrayImage_codes[2 * (i + 1)];
			gray_2 = GrayImage_codes[2 * (i + 1) + 1];
		}
		temp = abs(gray_1 - gray_2);
		//imshow("gray_1", gray_1);
		//imshow("gray_2", gray_2);
		//imshow("temp1", temp);
		temp = (temp >= sl_thresh);
		//imshow("temp2", temp);
		

		//cout<<"Right"<<endl<<temp.rows<<temp.cols<<mask.rows<<mask.cols<<endl;
		bitwise_or(temp, mask, mask);		//灰度变化的，即全投影区域
		bit_plane_2 = (gray_1 >= gray_2);
		//imshow("mask", mask);
		//waitKey();
		// Convert from gray code to decimal value. 验证完毕
		if (i>0)
			bitwise_xor(bit_plane_1, bit_plane_2, bit_plane_1);
		else
			bit_plane_2.copyTo(bit_plane_1);
		int tmp = pow(2.0, n_cols - i - 1);
		add(decoded_cols, Scalar(tmp), decoded_cols, bit_plane_1);
	}
	int bias = 0;
	if (DownSample != 0)
		bias = pow(2.0, DownSample - 1);
	decoded_cols += -col_shift + bias;

	// Decode Gray codes for projector rows.
	decoded_rows.setTo(0);
	for (int i = 0; i<n_rows - DownSample; i++) {	 // 从n_col
													 // Decode bit-plane and update mask.
		if (GrayImage_codes[2 * (i + n_cols + 1 - DownSample)].channels() == 3) {
			cvtColor(GrayImage_codes[2 * (i + n_cols - DownSample + 1)], gray_1, COLOR_RGB2GRAY);
			cvtColor(GrayImage_codes[2 * (i + n_cols - DownSample + 1) + 1], gray_2, COLOR_RGB2GRAY);
		}
		else {
			gray_1 = GrayImage_codes[2 * (i + n_cols - DownSample + 1)];
			gray_2 = GrayImage_codes[2 * (i + n_cols - DownSample + 1) + 1];
		}

		temp = abs(gray_1 - gray_2);
		temp = (temp >= sl_thresh);
		bitwise_or(temp, mask, mask);		//灰度变化的，即全投影区域
		bit_plane_2 = (gray_1 >= gray_2);

		// Convert from gray code to decimal value.
		if (i>0)
			bitwise_xor(bit_plane_1, bit_plane_2, bit_plane_1);
		else
			bit_plane_2.copyTo(bit_plane_1);

		int tmp = pow(2.0, n_rows - i - 1);
		add(decoded_rows, Scalar(tmp), decoded_rows, bit_plane_1);
	}
	decoded_rows += -row_shift + bias;

	// Eliminate invalid column/row estimates.
	// Note: This will exclude pixels if either the column or row is missing or erroneous.
	// 0<=decoded_cols<=max_width-1
	// 0<=decoded_rows<=max_height-1
	temp = (decoded_cols <= max_width - 1);
	bitwise_and(temp, mask, mask);

	temp = (decoded_cols >= 0);
	bitwise_and(temp, mask, mask);

	temp = (decoded_rows <= max_height - 1);
	bitwise_and(temp, mask, mask);

	temp = (decoded_rows >= 0);
	bitwise_and(temp, mask, mask);


	// 其余的点赋值为NULL
	bitwise_not(mask, temp);
	decoded_cols.setTo(Scalar(0), temp);
	decoded_rows.setTo(Scalar(0), temp);

	// Return without errors.
	return 1;
}
void ThreeDimensionReconstruction::Reconstruction()
{
	if (CameraImgsLeft.size() == 0 && CameraImgsRight.size() == 0)
	{

		textBrowser->append(QString::fromLocal8Bit("Please Collect or Load Images!\n"));
		textBrowser->moveCursor(textBrowser->textCursor().End);
		return;

	}
	if (CameraImgsLeft.size() != gray_codes.size() || CameraImgsRight.size() != gray_codes.size())
	{

		textBrowser->append(QString::fromLocal8Bit("Please Collect Images!\n"));
		textBrowser->moveCursor(textBrowser->textCursor().End);
		return;
		
	}
	if (this->whetherUseSpaceCoding)
	{
		imshow("OriginalLeftImage1", CameraImgsLeft[1]);
		imshow("OriginalLeftImage2",CameraImgsLeft[2]);
		imshow("LeftImage1", abs(CameraImgsLeft[1] - CameraImgsLeft[0]));
		imshow("LeftImage2", abs(CameraImgsLeft[2] - CameraImgsLeft[0]));
		imshow("LeftImageDifference1", (CameraImgsLeft[2] - CameraImgsLeft[1]));
		imshow("LeftImageDifference2", (CameraImgsLeft[2] - CameraImgsLeft[1]) / (CameraImgsLeft[2] + CameraImgsLeft[1]) * 255);
		imshow("LeftImageDifference3", (CameraImgsLeft[2] - CameraImgsLeft[1]) >100);
		imshow("LeftImageDifference4", (CameraImgsLeft[2] - CameraImgsLeft[1]) / (CameraImgsLeft[2] + CameraImgsLeft[1]) * 255>100);
		imshow("LeftImageadd", CameraImgsLeft[2] + CameraImgsLeft[1]);
		imshow("LeftImageDifferencemask", abs(CameraImgsLeft[2] - CameraImgsLeft[1])/(CameraImgsLeft[2] + CameraImgsLeft[1])*255);

		waitKey();
		imshow("RightImage", abs(CameraImgsRight[0] - CameraImgsRight[1]));
	}
	else
	{
		cam_h = image_size.height;
		cam_w = image_size.width;

		std::cout << cam_h << " " << cam_w << std::endl;

		// Step 3: 编码图像的解码,包括left解码, right解码, 和点对的匹配
		cout << "[Step 3] Decode Gray Codes." << endl;
		Mat decoded_cols_left(cam_h, cam_w, CV_32SC1, Scalar(0));
		Mat decoded_rows_left(cam_h, cam_w, CV_32SC1, Scalar(0));
		Mat gray_mask_left(cam_h, cam_w, CV_8UC1, Scalar(0));
		Mat decoded_cols_right(cam_h, cam_w, CV_32SC1, Scalar(0));
		Mat decoded_rows_right(cam_h, cam_w, CV_32SC1, Scalar(0));
		Mat gray_mask_right(cam_h, cam_w, CV_8UC1, Scalar(0));
		// left解码
		decodeGrayCodes(proj_w, proj_h, CameraImgsLeft,
			decoded_cols_left, decoded_rows_left, gray_mask_left,
			Th);
		// right解码
		decodeGrayCodes(proj_w, proj_h, CameraImgsRight,
			decoded_cols_right, decoded_rows_right, gray_mask_right,
			Th);

		// Step4: 得到匹配点对
		cout << "[Step 4] Get Matched Points." << endl;
		ptsInLeft.clear();
		ptsInRight.clear();
		map<size_t, TwoVec> ptsMap;
		for (int i = 0; i < cam_h; i++)
		{
			const int* Lcol_i = decoded_cols_left.ptr<int>(i);
			const int* Lrow_i = decoded_rows_left.ptr<int>(i);
			const uchar* maskL_i = gray_mask_left.ptr<uchar>(i);
			const int* Rcol_i = decoded_cols_right.ptr<int>(i);
			const int* Rrow_i = decoded_rows_right.ptr<int>(i);
			const uchar* maskR_i = gray_mask_right.ptr<uchar>(i);
			for (int j = 0; j < cam_w; j++)
			{
				if (maskL_i[j] != 0)
				{
					size_t hashVal = Lcol_i[j] + 1024 * Lrow_i[j];
					ptsMap[hashVal].lpts.push_back(Point2f(j, i));
				}
				if (maskR_i[j] != 0)
				{
					size_t hashVal = Rcol_i[j] + 1024 * Rrow_i[j];
					ptsMap[hashVal].rpts.push_back(Point2f(j, i));
				}
			}
		}
		cout << "MapSize:" << ptsMap.size() << endl;
		typedef map<size_t, TwoVec>::iterator MapIter;
		for (MapIter it = ptsMap.begin(); it != ptsMap.end(); it++)
		{
			if (it->second.rpts.empty() || it->second.lpts.empty())	// 左或右为空
				continue;
			float lx = 0, ly = 0, rx = 0, ry = 0;
			for (int i = 0; i < it->second.lpts.size(); i++)
			{
				lx += it->second.lpts[i].x;
				ly += it->second.lpts[i].y;
			}
			lx /= it->second.lpts.size();
			ly /= it->second.lpts.size();
			ptsInLeft.push_back(Point2f(lx, ly));

			for (int i = 0; i < it->second.rpts.size(); i++)
			{
				rx += it->second.rpts[i].x;
				ry += it->second.rpts[i].y;
			}
			rx /= it->second.rpts.size();
			ry /= it->second.rpts.size();
			ptsInRight.push_back(Point2f(rx, ry));
		}
		cout << "MapSize:" << ptsInLeft.size() << endl;

		// Step 5:重建表面
		cout << "[Step 5] Start Reconstruction." << endl;
		if (ptsInLeft.empty() || ptsInRight.empty())
		{
			emit AfterReconstruction(false);
			return;
		}
		Mat Pts3D;
		Mat T1 = (Mat_<float>(3, 4) <<
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0);
		Mat T2 = (Mat_<float>(3, 4) <<
			R.at<double>(0, 0), R.at<double>(0, 1), R.at<double>(0, 2), T.at<double>(0, 0),
			R.at<double>(1, 0), R.at<double>(1, 1), R.at<double>(1, 2), T.at<double>(0, 1),
			R.at<double>(2, 0), R.at<double>(2, 1), R.at<double>(2, 2), T.at<double>(0, 2)
			);

		vector<Point2d> pts_1, pts_2;
		for (int i = 0; i < ptsInLeft.size(); i++)
		{
			// 将像素坐标转换至相机坐标
			pts_1.push_back(pixel2cam(ptsInLeft[i], LeftCameraMatrix));
			pts_2.push_back(pixel2cam(ptsInRight[i], RightCameraMatrix));
		}

		cv::triangulatePoints(T1, T2, pts_1, pts_2, Pts3D);
		std::cout << Pts3D.size() << std::endl;
		std::cout << Pts3D.type() << std::endl;
		PointCloud.clear();
		for (int i = 0; i < Pts3D.cols; i++)	// pts3D: 4*N matrix
		{
			for (int j = 0; j < 3; j++)
				Pts3D.at<double>(j, i) = Pts3D.at<double>(j, i) / Pts3D.at<double>(3, i);
			Pts3D.at<double>(3, i) = 1.0;
			PointCloud.push_back(Point3f(Pts3D.at<double>(0, i), Pts3D.at<double>(1, i), Pts3D.at<double>(2, i)));
		}
		//triangulatePoints(P1, P2, ptsInLeft, ptsInRight, Pts3D);
		//std::cout << Pts3D.size() << std::endl;
		//std::cout << Pts3D.type() << std::endl;
		//for (int i = 0; i<Pts3D.cols; i++)	// pts3D: 4*N matrix
		//{
		//	for (int j = 0; j<3; j++)
		//		Pts3D.at<float>(j, i) = Pts3D.at<float>(j, i) / Pts3D.at<float>(3, i);
		//	Pts3D.at<float>(3, i) = 1.0;
		//	PointCloud.push_back(Point3f(Pts3D.at<float>(0, i), Pts3D.at<float>(1, i), Pts3D.at<float>(2, i)));
		//}

		vtkSmartPointer<vtkPoints>ReconstructionPointsCloud = vtkSmartPointer<vtkPoints>::New();
		for (int i = 0; i < PointCloud.size(); i++)
		{
			ReconstructionPointsCloud->InsertNextPoint(PointCloud[i].x, PointCloud[i].y, PointCloud[i].z);
		}

		vtkSmartPointer<vtkPolyData>ReconstructionResult = vtkSmartPointer<vtkPolyData>::New();
		ReconstructionResult->SetPoints(ReconstructionPointsCloud);
		vtkSmartPointer<vtkDelaunay2D> Delaunay2D = vtkSmartPointer<vtkDelaunay2D>::New();
		Delaunay2D->SetInputData(ReconstructionResult);
		Delaunay2D->Update();
		ReconstructionResult = Delaunay2D->GetOutput();
		vtkSmartPointer<vtkPLYWriter>PLYWriter = vtkSmartPointer<vtkPLYWriter>::New();
		PLYWriter->SetInputData(ReconstructionResult);
		PLYWriter->SetFileName((WorkSpace + "\\Reconstruction\\result.ply").c_str());
		PLYWriter->Update();
		RenderInitializa();

		vtkSmartPointer<vtkPolyDataMapper> Mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
		Mapper1->SetInputData(ReconstructionResult);
		if (RcReWoCActor)
		{
			Renderer->RemoveActor(RcReWoCActor);
		}
		if (RcReWCActor)
		{
			Renderer->RemoveActor(RcReWCActor);
			RcReWCActor = NULL;
		}
		RcReWoCActor = vtkSmartPointer<vtkActor>::New();
		RcReWoCActor->SetMapper(Mapper1);

		Renderer->AddActor(RcReWoCActor);

		qvtkOpenGLWidget_Big->show();
		qvtkOpenGLWidget_Big->update();

		emit AfterReconstruction(true);
	}
	return;
}

Point2d ThreeDimensionReconstruction::pixel2cam(const Point2d& p, const Mat& K)
{
	return Point2d
	(
		(p.x - K.at<double>(0, 2)) / K.at<double>(0, 0),
		(p.y - K.at<double>(1, 2)) / K.at<double>(1, 1)
	);
}
void ThreeDimensionReconstruction::RenderInitializa()
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

	Renderer = vtkSmartPointer<vtkRenderer>::New();
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


	RenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
	RenderWindow->AddRenderer(Renderer);
	RenderWindow->SetSize(1080, 960);

	qvtkOpenGLWidget_Big->SetRenderWindow(RenderWindow);
}
void ThreeDimensionReconstruction::RenderReconstructionResult(vtkSmartPointer<vtkPolyData>ReconstructionResult)
{



}

void ThreeDimensionReconstruction::TextureMapping()
{
	vtkSmartPointer<vtkPolyData>ReconstructionResult = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPLYReader>PLYReader = vtkSmartPointer<vtkPLYReader>::New();
	PLYReader->SetFileName((WorkSpace + "\\Reconstruction\\result.ply").c_str());
	PLYReader->Update();
	ReconstructionResult = PLYReader->GetOutput();
	std::cout << "Left" << endl;
	float min_x = 10000;
	float max_x = -1e5;
	float min_y = 10000;
	float max_y = -1e5;
	for (int i = 0; i < ptsInLeft.size(); i++)
	{
		if (ptsInLeft[i].x > max_x)
		{
			max_x = ptsInLeft[i].x;
		}
		if (ptsInLeft[i].x < min_x)
		{
			min_x = ptsInLeft[i].x;
		}
		if (ptsInLeft[i].y > max_y)
		{
			max_y = ptsInLeft[i].y;
		}
		if (ptsInLeft[i].y < min_y)
		{
			min_y = ptsInLeft[i].y;
		}

	}

	std::cout << "min_x" << min_x << " " << "max_x" << max_x << std::endl;
	std::cout << "min_y" << min_y << " " << "max_y" << max_y << std::endl;
	 min_x = 10000;
	 max_x = -1e5;
	 min_y = 10000;
	 max_y = -1e5;
	std::cout << "Right" << endl;
	for (int i = 0; i < ptsInRight.size(); i++)
	{
		if (ptsInRight[i].x > max_x)
		{
			max_x = ptsInRight[i].x;
		}
		if (ptsInRight[i].x < min_x)
		{
			min_x = ptsInRight[i].x;
		}
		if (ptsInRight[i].y > max_y)
		{
			max_y = ptsInRight[i].y;
		}
		if (ptsInRight[i].y < min_y)
		{
			min_y = ptsInRight[i].y;
		}
		//std::cout << ptsInRight[i] << endl;
	}
	std::cout << "min_x" << min_x << " " << "max_x" << max_x << std::endl;
	std::cout << "min_y" << min_y << " " << "max_y" << max_y << std::endl;
	vtkSmartPointer<vtkFloatArray> T_Coords = vtkSmartPointer<vtkFloatArray>::New();
	T_Coords->SetNumberOfComponents(2);
	for (int i = 0; i < PointCloud.size(); i++)
	{
		T_Coords->InsertNextTuple2(ptsInLeft[i].x, ptsInLeft[i].y);
		//Vec3b Pixel = LeftBRGImage.at<Vec3b>(int(ptsInLeft[i].y), int(ptsInLeft[i].x));
	}
	ReconstructionResult->GetPointData()->SetTCoords(T_Coords);
	vtkSmartPointer<vtkUnsignedCharArray>	pointscolor = vtkSmartPointer<vtkUnsignedCharArray>::New();
	pointscolor->SetName("Colors");
	pointscolor->SetNumberOfComponents(3);
	std::cout << LeftBRGImage.size() << std::endl;
	std::cout << LeftBRGImage.cols<< std::endl;	
	std::cout << LeftBRGImage.rows<< std::endl;
	std::cout << LeftBRGImage.channels() << std::endl;
	std::cout << LeftBRGImage.type() << std::endl;
	for (int i = 0; i < PointCloud.size(); i++)
	{
		Vec3b Pixel = LeftBRGImage.at<Vec3b>(int(ptsInLeft[i].y), int(ptsInLeft[i].x));
		pointscolor->InsertNextTuple3(Pixel[2], Pixel[1], Pixel[0]);
	}
	ReconstructionResult->GetPointData()->SetScalars(pointscolor);
	ReconstructionResult->GetPointData()->AddArray(pointscolor);

	ReconstructionResult->GetPointData()->GetArray("Colors")->Print(std::cout);

	vtkSmartPointer<vtkPLYWriter>PLYWriter = vtkSmartPointer<vtkPLYWriter>::New();
	PLYWriter->SetInputData(ReconstructionResult);
	PLYWriter->SetArrayName("Colors");
	PLYWriter->SetFileName((WorkSpace + "\\Reconstruction\\result.ply").c_str());
	PLYWriter->Update();

	vtkSmartPointer<vtkPolyDataMapper> Mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
	Mapper1->SetInputData(ReconstructionResult);
	if (RcReWoCActor)
	{
		Renderer->RemoveActor(RcReWoCActor);
		RcReWoCActor = NULL;
	}
	if (RcReWCActor)
	{
		Renderer->RemoveActor(RcReWCActor);

	}
	RcReWCActor = vtkSmartPointer<vtkActor>::New();
	RcReWCActor->SetMapper(Mapper1);

	Renderer->RemoveActor(RcReWoCActor);
	Renderer->AddActor(RcReWCActor);
	qvtkOpenGLWidget_Big->GetRenderWindow()->Render();
	qvtkOpenGLWidget_Big->show();
	qvtkOpenGLWidget_Big->update();
}

void ThreeDimensionReconstruction::DepthChanged(int depth)
{
	this->depth = depth;
	Depth_Label->setText(QString(std::to_string(depth).c_str()));
	vtkSmartPointer<vtkPolyData> tem_WordPolydata = vtkSmartPointer<vtkPolyData>::New();

	tem_WordPolydata->DeepCopy(WordPolydata);
	vtkSmartPointer<vtkPoints> WordPoints = vtkSmartPointer<vtkPoints>::New();
	WordPoints = tem_WordPolydata->GetPoints();
	for (int i = 0; i < WordPoints->GetNumberOfPoints(); i++)
	{
		double point[3] = { 0,0,0 };
		WordPoints->GetPoint(i, point);
		for (int j = 0; j < 3; j++)
		{
			point[j] *= this->depth;
		}
		WordPoints->SetPoint(i, point);
	}
	tem_WordPolydata->SetPoints(WordPoints);

	vtkSmartPointer<vtkPolyDataMapper> Mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
	Mapper1->SetInputData(tem_WordPolydata);
	if (WordPolydataActor)
	{
		Renderer->RemoveActor(WordPolydataActor);
	}
	WordPolydataActor = vtkSmartPointer<vtkActor>::New();
	WordPolydataActor->SetMapper(Mapper1);

	Renderer->AddActor(WordPolydataActor);
	qvtkOpenGLWidget_Big->GetRenderWindow()->Render();
	qvtkOpenGLWidget_Big->update();
	qvtkOpenGLWidget_Big->show();
}
void ThreeDimensionReconstruction::PixeltoPoint()
{


	vtkSmartPointer<vtkMatrix4x4> LeftCameraMatrix4x4 = vtkSmartPointer<vtkMatrix4x4>::New();
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			LeftCameraMatrix4x4->SetElement(i, j, LeftCameraMatrix.at<double>(i, j));
			//std::cout << R.at<double>(i, j) << std::endl;
		}
	}
	for (int i = 0; i < 3; i++)
	{
		LeftCameraMatrix4x4->SetElement(i, 3, 0);
		//std::cout << T.at<double>(i) << std::endl;
		LeftCameraMatrix4x4->SetElement(3, i, 0);
	}
	LeftCameraMatrix4x4->SetElement(3, 3, 1);
	LeftCameraMatrix4x4->Invert();


	vtkSmartPointer<vtkPoints> ImagePixel = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkUnsignedCharArray>	pointscolor = vtkSmartPointer<vtkUnsignedCharArray>::New();
	pointscolor->SetName("Colors");
	pointscolor->SetNumberOfComponents(3);

	for (int i = 0; i < LeftBRGImage.cols; i+=5)
	{
		for (int j = 0; j < LeftBRGImage.rows; j+=5)
		{
			//std::cout << "i " << i<<" j " << j << std::endl;
			ImagePixel->InsertNextPoint(i, j, 1);
			Vec3b Pixel = LeftBRGImage.at<Vec3b>(j, i);
			pointscolor->InsertNextTuple3(Pixel[2], Pixel[1], Pixel[0]);
		}
	}

	//ImagePixel->InsertNextPoint(LeftBRGImage.cols, 0, 1);
	//ImagePixel->InsertNextPoint(0, LeftBRGImage.rows, 1);
	//ImagePixel->InsertNextPoint(LeftBRGImage.cols, LeftBRGImage.rows, 1);

	vtkSmartPointer<vtkPolyData> ImagePixelPolydata = vtkSmartPointer<vtkPolyData>::New();
	ImagePixelPolydata->SetPoints(ImagePixel);
	std::cout << ImagePixelPolydata->GetNumberOfPoints() << std::endl;
	vtkSmartPointer<vtkTransform> TransformLeftCameraMatrix4x4 = vtkSmartPointer<vtkTransform>::New();
	TransformLeftCameraMatrix4x4->SetMatrix(LeftCameraMatrix4x4);

	vtkSmartPointer<vtkTransformPolyDataFilter> PixeltoPointsTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	PixeltoPointsTransformFilter->SetTransform(TransformLeftCameraMatrix4x4);
	PixeltoPointsTransformFilter->SetInputData(ImagePixelPolydata);
	PixeltoPointsTransformFilter->Update();

	WordPolydata = vtkSmartPointer<vtkPolyData>::New();
	WordPolydata = PixeltoPointsTransformFilter->GetOutput();
	vtkSmartPointer<vtkPoints> WordPoints = vtkSmartPointer<vtkPoints>::New();
	WordPoints = WordPolydata->GetPoints();
	for (int i = 0; i < WordPoints->GetNumberOfPoints(); i++)
	{
		double point[3] = { 0,0,0 };
		WordPoints->GetPoint(i, point);
		for (int j = 0; j < 3; j++)
		{
			point[j] /= point[2];
		}
		WordPoints->SetPoint(i, point);
	}
	WordPolydata->SetPoints(WordPoints);

	vtkSmartPointer<vtkDelaunay2D> Delaunay2D = vtkSmartPointer<vtkDelaunay2D>::New();
	Delaunay2D->SetInputData(WordPolydata);
	Delaunay2D->Update();
	WordPolydata = Delaunay2D->GetOutput();




	WordPolydata->GetPointData()->SetScalars(pointscolor);


	vtkSmartPointer<vtkPolyData> tem_WordPolydata = vtkSmartPointer<vtkPolyData>::New();

	tem_WordPolydata->DeepCopy(WordPolydata);
	WordPoints = tem_WordPolydata->GetPoints();
	for (int i = 0; i < WordPoints->GetNumberOfPoints(); i++)
	{
		double point[3] = { 0,0,0 };
		WordPoints->GetPoint(i, point);
		for (int j = 0; j < 3; j++)
		{
			point[j] *= this->depth;
		}
		WordPoints->SetPoint(i, point);
	}
	tem_WordPolydata->SetPoints(WordPoints);

	vtkSmartPointer<vtkPolyDataMapper> Mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
	Mapper1->SetInputData(tem_WordPolydata);
	if (WordPolydataActor)
	{
		Renderer->RemoveActor(WordPolydataActor);
	}
	WordPolydataActor = vtkSmartPointer<vtkActor>::New();
	WordPolydataActor->SetMapper(Mapper1);

	Renderer->AddActor(WordPolydataActor);
	qvtkOpenGLWidget_Big->GetRenderWindow()->Render();
	qvtkOpenGLWidget_Big->update();
	qvtkOpenGLWidget_Big->show();

	Rec_Depth_HorizontalSlider->setEnabled(true);
}