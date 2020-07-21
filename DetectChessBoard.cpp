#include"DetectChessBoard.h"
DetectChessBoard::DetectChessBoard()
{
	//detect = false;
	//m_stop = true;
}

DetectChessBoard::~DetectChessBoard()
{
}
void DetectChessBoard::run()
{
	//while (m_stop)
	//{
	//	if (detect)
	//	{
			image_points_buf->clear();
			//std::cout <<"image"<< *image<<std::endl;
			//std::cout << "board_size" << board_size << std::endl;
			//std::cout << "image_points_buff" << *image_points_buf << std::endl;

			bool findcorner_left = findChessboardCorners(*image, board_size, *image_points_buf);
			if (!(findcorner_left))
			{
				//std::cout << "can not find chessboard corners in cmaera !\n"; //找不到角点  
																			  //flag = 2; //未成功检测棋盘格态												   /*return;*/
			}
			else
			{

				find4QuadCornerSubpix(*image, *image_points_buf, cv::Size(5, 5)); //对粗提取的角点进行精确化  


			}
			*flag = findcorner_left;
	//		emit detect_result(true);
	//	}
	//	detect = false;
	//}
	quit();
	
}