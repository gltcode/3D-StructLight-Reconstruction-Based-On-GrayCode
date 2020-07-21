#ifndef DCBoard_H
#define DCBoard_H
#include<qthread.h>
#include<opencv2\opencv.hpp>
class DetectChessBoard: public QThread 
{
	Q_OBJECT
public:
	DetectChessBoard();
	~DetectChessBoard();

	cv::Mat* image;
	cv::Size board_size;
	std::vector<cv::Point2f> *image_points_buf;
	bool * flag;
	//bool detect;
	//bool m_stop;
protected:
	void run() Q_DECL_OVERRIDE;
//signals:
//	void detect_result(bool);


};

#endif // !DCBoard
