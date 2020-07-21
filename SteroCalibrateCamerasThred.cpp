#include"SteroCalibrateCamerasThred.h"
SteroCalibrateCamerasThred::SteroCalibrateCamerasThred()
{
}

SteroCalibrateCamerasThred::~SteroCalibrateCamerasThred()
{
}
void SteroCalibrateCamerasThred::run()
{
	Mat  E, F;

	double reprojectionerror = stereoCalibrate(*object_points, *Corner_buf_Left,*Corner_buf_Right, *LeftCameraMatrix, *LeftCameradistCoeffs,
		*RightCameraMatrix, *RightCameradistCoeffs,
		image_size, *R, *T, E, F,
		CALIB_FIX_INTRINSIC +
		CALIB_ZERO_TANGENT_DIST +
		CALIB_FIX_K3 + CALIB_FIX_K4 + CALIB_FIX_K5);

	std::cout << "-----R-----" << std::endl << R << std::endl << "-----T-----" << std::endl << T << std::endl << "-----E-----" << std::endl << E << std::endl << "-----F-----" << std::endl << F << std::endl;
	std::cout << "The reprojection error =" << reprojectionerror << " pixels." << std::endl;

	Mat t_x = (Mat_<double>(3, 3) <<
		0, -T->at<double>(2, 0), T->at<double>(1, 0),
		T->at<double>(2, 0), 0, -T->at<double>(0, 0),
		-T->at<double>(1, 0), T->at<double>(0, 0), 0);
	cout << t_x << std::endl;
	for (int index = 0; index < Corner_buf_Left->size(); index++)
	{
		for (int j = 0; j < Corner_buf_Left->at(index).size(); j++)
		{
			Point2d x1 = pixel2cam(Corner_buf_Left->at(index)[j], *LeftCameraMatrix);
			//cout << "x1 " <<x1 << std::endl;
			Mat y1 = (Mat_<double>(3, 1) << x1.x, x1.y, 1);
			//cout << "y1 " << y1 << std::endl;
			Point2d x2 = pixel2cam(Corner_buf_Right->at(index)[j], *RightCameraMatrix);
			//cout << "x2 " << x2 << std::endl;
			Mat y2 = (Mat_<double>(3, 1) << x2.x, x2.y, 1);
			//cout << "y2 " << y2 << std::endl;
			Mat d = y2.t() * t_x * *R * y1;
			//cout << "epipolar constraint = " << d << endl;

		}
	}
	// 立体矫正  BOUGUET'S METHOD
	Mat R1, R2, P1, P2, Q;
	Rect validRoi[2];

	cv::stereoRectify(*LeftCameraMatrix, *LeftCameradistCoeffs,
		*RightCameraMatrix, *RightCameradistCoeffs,
		image_size, *R, *T, R1, R2, P1, P2, Q,
		CALIB_ZERO_DISPARITY, 1, image_size, &validRoi[0], &validRoi[1]);


	ofstream OutPutTwocameraParameters(WorkSpace + "\\extrinsicParametersOfTwoCameras.txt");

	OutPutTwocameraParameters << "R" << std::endl << *R << std::endl;
	OutPutTwocameraParameters << "T" << std::endl <<*T << std::endl;
	OutPutTwocameraParameters << "E" << std::endl << E << std::endl;
	OutPutTwocameraParameters << "F" << std::endl << F << std::endl;
	OutPutTwocameraParameters << "R1" << std::endl << R1 << std::endl;
	OutPutTwocameraParameters << "R2" << std::endl << R2 << std::endl;
	OutPutTwocameraParameters << "P1" << std::endl << P1 << std::endl;
	OutPutTwocameraParameters << "P2" << std::endl << P2 << std::endl;
	OutPutTwocameraParameters << "Q" << std::endl << Q << std::endl;

	OutPutTwocameraParameters.close();
	emit FinishedSteroCalibration(reprojectionerror);
	quit();
}
Point2d SteroCalibrateCamerasThred::pixel2cam(const Point2d& p, const Mat& K)
{
	return Point2d
	(
		(p.x - K.at<double>(0, 2)) / K.at<double>(0, 0),
		(p.y - K.at<double>(1, 2)) / K.at<double>(1, 1)
	);
}
//void SteroCalibrateCamerasThred::TransformTV2TM(Mat rvecsMatOfCamera, Mat tvecsMatOfCamera, vtkSmartPointer<vtkMatrix4x4> RTM)
//{
//	//double R_matrix[9];
//	//CvMat pr_vec = rvecsMatOfCamera;
//	//CvMat pR_matrix;
//	//cvInitMatHeader(&pR_matrix, 3, 3, CV_64FC1, R_matrix, CV_AUTOSTEP);
//
//	//cvRodrigues2(&pr_vec, &pR_matrix, 0);//从旋转向量求旋转矩阵
//	Mat outputArray;
//	Rodrigues(rvecsMatOfCamera, outputArray);
//	//std::cout << "output array"<<outputArray<<std::endl;
//	//string ty = cv::typeToString(outputArray.type());
//	//printf("Matrix: %s %dx%d \n", ty.c_str(), outputArray.cols, outputArray.rows);
//	for (int i = 0; i < 3; i++)
//	{
//		for (int j = 0; j < 3; j++)
//		{
//			//std::cout << outputArray.at<double>(i, j)<<" ";
//			RTM->SetElement(i, j, outputArray.at<double>(i, j));
//		}
//	}
//	//for (int i = 0; i < 3; i++)
//	//{
//	//	for (int j = 0; j < 3; j++)
//	//	{
//	//		RTM->SetElement(i, j, cvmGet(&pR_matrix, i, j));
//	//	}
//	//}
//
//	for (int i = 0; i < 3; i++)
//	{
//		RTM->SetElement(i, 3, tvecsMatOfCamera.at<double>(i));
//		RTM->SetElement(3, i, 0);
//	}
//
//	RTM->SetElement(3, 3, 1);
//
//}
