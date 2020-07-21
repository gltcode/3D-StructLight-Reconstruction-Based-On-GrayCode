#include<CalibrateCamerasThred.h>
CalibrateCamerasThred::CalibrateCamerasThred()
{

}

CalibrateCamerasThred::~CalibrateCamerasThred()
{
}

void CalibrateCamerasThred::run()
{
	Left_rvecsMat->clear();//每幅图像的旋转向量
	Left_tvecsMat->clear();//每幅图像的平移向量
	cv::calibrateCamera(*Left_object_points, *Left_Corner_buf, image_size, *Left_CameraMatrix, *Left_CameradistCoeffs, *Left_rvecsMat, *Left_tvecsMat, 0);//相机标定


	std::cout << "camera left :" << std::endl << "camera matrix :" << *Left_CameraMatrix << std::endl;



	double total_err_Of_Camera1 = 0.0; /* 所有图像的平均误差的总和 */
	double err_Of_Camera1 = 0.0; /* 每幅图像的平均误差 */
	std::vector<cv::Point2f> Camera1_image_points2; /* 保存重新计算得到的投影点 */
	std::cout << "\t每幅图像的标定误差：\n";
	for (int i = 0; i < Left_rvecsMat->size(); i++)
	{
		vector<Point3f> tempPointSet = Left_object_points->at(i);		/* 通过得到的摄像机内外参数，对空间的三维点进行重新投影计算，得到新的投影点 */
		projectPoints(tempPointSet, Left_rvecsMat->at(i), Left_tvecsMat->at(i), *Left_CameraMatrix, *Left_CameradistCoeffs, Camera1_image_points2);		/* 计算新的投影点和旧的投影点之间的误差*/
		vector<Point2f> tempImagePoint = Left_Corner_buf->at(i);

		Mat image_points2Mat = Mat(1, Camera1_image_points2.size(), CV_32FC2);
		Mat tempImagePointMat = Mat(1, tempImagePoint.size(), CV_32FC2);
		for (int j = 0; j < tempImagePoint.size(); j++)
		{
			image_points2Mat.at<Vec2f>(0, j) = Vec2f(Camera1_image_points2[j].x, Camera1_image_points2[j].y);
			tempImagePointMat.at<Vec2f>(0, j) = Vec2f(tempImagePoint[j].x, tempImagePoint[j].y);
		}

		err_Of_Camera1 = norm(image_points2Mat, tempImagePointMat, NORM_L2);
		err_Of_Camera1 /= board_size.height*board_size.width;
		total_err_Of_Camera1 += err_Of_Camera1;
		std::cout << "第" << i + 1 << "幅图像的平均误差：" << err_Of_Camera1 << "像素" << endl;
	}
	std::cout << "总体平均误差：" << total_err_Of_Camera1 / Left_rvecsMat->size() << "像素" << endl;
	Left_averageerror = total_err_Of_Camera1 / Left_rvecsMat->size();

	char cm1[100];
	std::sprintf(cm1, (WorkSpace + "\\calibrationParametersOfLeftCamera.txt").c_str());
	ofstream OutPut1(cm1);
	OutPut1 << "Camera Matrix: \n";
	std::cout << "Camera Matrix: \n";
	OutPut1 << *Left_CameraMatrix << std::endl;
	std::cout << *Left_CameraMatrix << std::endl;

	OutPut1 << "distortion Coeffs: \n";
	std::cout << "distortion Coeffs: \n";
	OutPut1 << *Left_CameradistCoeffs << std::endl;
	std::cout << *Left_CameradistCoeffs << std::endl;

	OutPut1 << "rvecsMatOfCamera: \n";
	OutPut1 << Left_rvecsMat->size() << std::endl;
	for (int i = 0; i < Left_rvecsMat->size(); i++)
	{
		OutPut1 << Left_rvecsMat->at(i) << std::endl;
	}
	OutPut1 << "tvecsMatOfCamera: \n";
	OutPut1 << Left_tvecsMat->size() << std::endl;
	for (int i = 0; i < Left_tvecsMat->size(); i++)
	{
		OutPut1 << Left_tvecsMat->at(i) << std::endl;
	}

	OutPut1.close();



	Right_rvecsMat->clear();//每幅图像的旋转向量
	Right_tvecsMat->clear();//每幅图像的平移向量
	cv::calibrateCamera(*Right_object_points, *Right_Corner_buf, image_size, *Right_CameraMatrix, *Right_CameradistCoeffs, *Right_rvecsMat, *Right_tvecsMat, 0);//相机标定


	std::cout << "camera right :" << std::endl << "camera matrix :" << *Right_CameraMatrix << std::endl;



	total_err_Of_Camera1 = 0.0; /* 所有图像的平均误差的总和 */
	err_Of_Camera1 = 0.0; /* 每幅图像的平均误差 */
	Camera1_image_points2.clear(); /* 保存重新计算得到的投影点 */
	std::cout << "\t每幅图像的标定误差：\n";
	for (int i = 0; i < Right_rvecsMat->size(); i++)
	{
		vector<Point3f> tempPointSet = Right_object_points->at(i);		/* 通过得到的摄像机内外参数，对空间的三维点进行重新投影计算，得到新的投影点 */
		projectPoints(tempPointSet, Right_rvecsMat->at(i), Right_tvecsMat->at(i), *Right_CameraMatrix, *Right_CameradistCoeffs, Camera1_image_points2);		/* 计算新的投影点和旧的投影点之间的误差*/
		vector<Point2f> tempImagePoint = Right_Corner_buf->at(i);

		Mat image_points2Mat = Mat(1, Camera1_image_points2.size(), CV_32FC2);
		Mat tempImagePointMat = Mat(1, tempImagePoint.size(), CV_32FC2);
		for (int j = 0; j < tempImagePoint.size(); j++)
		{
			image_points2Mat.at<Vec2f>(0, j) = Vec2f(Camera1_image_points2[j].x, Camera1_image_points2[j].y);
			tempImagePointMat.at<Vec2f>(0, j) = Vec2f(tempImagePoint[j].x, tempImagePoint[j].y);
		}

		err_Of_Camera1 = norm(image_points2Mat, tempImagePointMat, NORM_L2);
		err_Of_Camera1 /= board_size.height*board_size.width;
		total_err_Of_Camera1 += err_Of_Camera1;
		std::cout << "第" << i + 1 << "幅图像的平均误差：" << err_Of_Camera1 << "像素" << endl;
	}
	std::cout << "总体平均误差：" << total_err_Of_Camera1 / Right_rvecsMat->size() << "像素" << endl;
	Right_averageerror = total_err_Of_Camera1 / Right_rvecsMat->size();

	char cm2[100];
	std::sprintf(cm2, (WorkSpace + "\\calibrationParametersOfRightCamera.txt").c_str());
	ofstream OutPut2(cm2);
	OutPut2 << "Camera Matrix: \n";
	std::cout << "Camera Matrix: \n";
	OutPut2 << *Right_CameraMatrix << std::endl;
	std::cout << *Right_CameraMatrix << std::endl;

	OutPut2 << "distortion Coeffs: \n";
	std::cout << "distortion Coeffs: \n";
	OutPut2 << *Right_CameradistCoeffs << std::endl;
	std::cout << *Right_CameradistCoeffs << std::endl;

	OutPut2 << "rvecsMatOfCamera: \n";
	OutPut2 << Right_rvecsMat->size() << std::endl;
	for (int i = 0; i < Right_rvecsMat->size(); i++)
	{
		OutPut2 << Right_rvecsMat->at(i) << std::endl;
	}
	OutPut2 << "tvecsMatOfCamera: \n";
	OutPut2 << Right_tvecsMat->size() << std::endl;
	for (int i = 0; i < Right_tvecsMat->size(); i++)
	{
		OutPut2 << Right_tvecsMat->at(i) << std::endl;
	}
	OutPut2.close();

	//RenderPlaneAndCamera();
	emit FinishedCalibrate(Left_averageerror, Right_averageerror);
	quit();
}


//void CalibrateCamerasThred::TransformTV2TM(Mat rvecsMatOfCamera, Mat tvecsMatOfCamera, vtkSmartPointer<vtkMatrix4x4> RTM)
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