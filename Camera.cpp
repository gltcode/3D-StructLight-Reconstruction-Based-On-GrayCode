#include "Camera.h"
Camera::Camera(QObject  *parent) : QObject(parent)

{
	
	// 这个类与多线程的实现没有直接的关系，它就老老实实地做好自己的事情
	hmv = NULL;
	detectChessBoard = false;
	ifundistort = false;
	wheatherCapturePicture = false;
	wheatherCapturePicture2 = false;
	iftracking = false;
	iftracking2 = false;
	ifreconstruction = false;
	ifreconstruction2 = false;
	imageSavePath = "./";
	IMGCOUNT = 0;
	Name = "Camera";
	cameraMatrix = Mat(3, 3, CV_64FC1, Scalar::all(0));//摄像机内参数矩阵 
	distCoeffs= Mat(1, 5, CV_64FC1, Scalar::all(0));//摄像机的5个畸变系数：k1,k2,p1,p2,k3


	board_size = Size(4,6);
}


//Camera::Camera()
//{
//	hmv = NULL;
//}
//
//Camera::~Camera()
//{
//}

Mat Camera::ImageCapture()
{
	//tSdkCameraCapbility sCameraInfo;
	tSdkFrameHead imgFrInfo;		//用于保存当前图像帧的帧头信息
	BYTE *imgBuffer;        //用于原始图像数据的缓冲区

	CameraGetImageBuffer(hmv, &imgFrInfo, &imgBuffer, 1000);	//原始数据都是Bayer格式的
	BYTE *m_imgBuffer = new BYTE[imgFrInfo.iWidth*imgFrInfo.iHeight * 3];//用于RGB数据的缓冲区
	CameraImageProcess(hmv, imgBuffer, m_imgBuffer, &imgFrInfo);	//将获得的原始数据转换成RGB格式的数据
	Mat res(imgFrInfo.iHeight, imgFrInfo.iWidth, CV_8UC3);		//图像大小已知
	memcpy(res.ptr(0), m_imgBuffer, imgFrInfo.iWidth*imgFrInfo.iHeight * 3);//复制到Mat矩阵res中
	CameraReleaseImageBuffer(hmv, imgBuffer);//释放缓存
	delete[]m_imgBuffer;
	m_imgBuffer = NULL;
	return res;
}

void Camera::myCreate(int AEC, int Gain)// 根据HMV初始化和loc信息初始化相机
{
	double Time;
	bool mode = false; //TRUE，使能自动曝光；FALSE，停止自动曝光。
					   //设置曝光模式
	CameraSetAeState(hmv, mode);
	//曝光时间
	CameraSetExposureTime(hmv, AEC);
	//调节增益
	CameraSetGain(hmv, Gain, Gain, Gain);
	//查看曝光时间
	CameraGetExposureTime(hmv, &Time);
	////设置频率
	//CameraSetFrameSpeed(hmv, 1);
	//调节分辨率
	//tSdkImageResolution *Resolution = {};
	//CameraSetResolutionForSnap(hmv, Resolution);


}

void Camera::CamInit(tSdkCameraDevInfo sCameraList)
{

	//初始化相机。(-1,-1)表示加载上次退出前保存的参数，如果是第一次使用该相机，则加载默认参数.
	if ((CameraInit(&sCameraList, -1, -1, &hmv)) != CAMERA_STATUS_SUCCESS)
	{
		printf("Failed to init the camera!\n");
	}

	//tSdkCameraCapbility *Capbility;

	//CameraGetCapability(hmv, Capbility);

	//std::cout << Capbility->iFrameSpeedDesc << endl;
	//自定义函数自定义相机参数
	myCreate(30000.0, 500);

	//相机开始采集图像
	CameraPlay(hmv);
}

void Camera::RenderPicture()
{	
	CorresponedChessBoard.clear();
	//std::cout << this->Name << std::endl;
	Mat mat,mat1,mat2;
	int flag=0; //普通态
	mat = ImageCapture();
	cvtColor(mat, mat, COLOR_RGB2GRAY);
	if(ifundistort)
	{
		undistort(mat, mat1,this->cameraMatrix,this->distCoeffs);
	}
	else
	{
		mat.copyTo(mat1);
	}
	
	cv::resize(mat1, mat1, Size(mat1.cols / 4, mat1.rows / 4));
	cv::flip(mat1, mat1, 0);


	mat1.copyTo(mat2);
	if (this->detectChessBoard)
	{
		
		//Mat img1;
		vector<Point2f> image_points_buf1;//缓存每幅图像上检测到的角点
		bool findcorner1 = findChessboardCorners(mat1, board_size, image_points_buf1);
		//cvtColor(mat1, img1, CV_RGB2GRAY);
		if (!findcorner1)
		{
			//cout << "can not find chessboard corners in cmaera 2 !\n"; //找不到角点  
			flag = 2; //未成功检测棋盘格态												   /*return;*/
		}
		else
		{
			/*亚像素精确化*/
			cornerSubPix(mat1, image_points_buf1, Size(5, 5), Size(-1, -1), TermCriteria(1+ 2, 30, 0.1));
			//find4QuadCornerSubpix(img2, image_points_buf_2, Size(5, 5)); //对粗提取的角点进行精确化  
			flag = 1;//成功检测棋盘格态

		}

	

		if (findcorner1)
		{
			drawChessboardCorners(mat2, board_size, image_points_buf1, true); //用于在图片中标记角点
			if (wheatherCapturePicture || iftracking)
			{
				for (int index = 0; index < image_points_buf1.size(); index++)
				{
					//std::cout << "Write" << std::endl;
					CorresponedChessBoard.push_back(image_points_buf1[index]);
				}
			}
			if (wheatherCapturePicture)
			{

				//WriteCapturedImage();
				CapturedImage = mat1;

				
				
			}
			
			
		}
		

	}

	if (this->ifreconstruction)
	{
		CapturedImage = mat1;
	}



	//cvtColor(mat2, mat2, COLOR_BGR2RGB);

	//imshow("window", mat2);
	//waitKey(30);
	//QImage disImage1 = QImage((const unsigned char*)(mat2.data), mat2.cols, mat2.rows, QImage::Format_RGB888);
	QImage disImage1 = QImage((const unsigned char*)(mat2.data), mat2.cols, mat2.rows, QImage::Format_Grayscale8);
	//QGraphicsScene *scene1 = new QGraphicsScene;//图像显示        
	//scene1->addPixmap(QPixmap::fromImage(disImage1));        
	//ui->graphicsView_Left->setScene(scene1);        
	//ui->graphicsView_Left->show();

	Label->setPixmap(QPixmap::fromImage(disImage1.scaled(Label->size(), Qt::KeepAspectRatio)));
	if (wheatherCapturePicture)
	{
		wheatherCapturePicture2 = true;
	}
	if (iftracking)
	{
		iftracking2 = true;
	}
	if (ifreconstruction)
	{
		ifreconstruction2 = true;
	}
	emit wheatherSuccessCapturePicture(flag);

}

//void Camera::CapturePicture(bool flag)
//{
//	wheatherCapturePicture = flag;
//
//}
//void Camera::SetTracking(bool tracking)
//{
//	iftracking = tracking;
//}
void Camera::WriteCapturedImage()
{

	char s1[100], sc1[100];
	sprintf(s1, (imageSavePath + "\\" + Name + "Image%d.png").c_str(), IMGCOUNT);

	imwrite(s1, CapturedImage);

	sprintf(sc1, (imageSavePath + "\\" + Name + "ChessBoard%d.txt").c_str(), IMGCOUNT);
	ofstream OutPut1(sc1);

	std::cout << CorresponedChessBoard.size() << std::endl;

	for (int index = 0; index < CorresponedChessBoard.size(); index++)
	{
		OutPut1 << CorresponedChessBoard.at(index).x << "," << CorresponedChessBoard.at(index).y << std::endl;
		//std::cout << "WRITE " << Name << std::endl;
	}
	OutPut1.close();
	CorresponedChessBoard.clear();
}
void Camera::GetChessBoard(vector<Point2f>& chessboard)
{
	chessboard.clear();
	for (int i = 0; i < CorresponedChessBoard.size(); i++)
	{
		chessboard.push_back(CorresponedChessBoard[i]);
}
}

