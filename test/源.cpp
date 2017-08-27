#include "method.h"

int main(){
	get_std_cnt();
	server_init();

	bool init_flag = false;
	int num_iter = 0;
	vector <int> tran;
	VideoCapture cap(0);
	if (!cap.isOpened())
		return -1;
	Mat frame;

	while (true){
		cap >> frame; // get a new frame from camera
		Mat Tmp1;
		Rect Tmp2;
		get_white_board_init(frame, Tmp1, Tmp2);
		imshow("debug", Tmp1);
		imshow("frame", frame);
		waitKey(10);

		if (init_flag == false){
			Mat imgRect;
			Rect rectTmp;
			double results[4];
			if (get_white_board_init(frame, imgRect, rectTmp)){
				if (get_cross_line(imgRect, frame, rectTmp, results)){
					tran.push_back(102);
					tran.push_back(int(results[0] + 0.5));
					tran.push_back(int(results[1] + 0.5));
					tran.push_back(int(results[2] + 0.5));
					tran.push_back(int(results[3] + 0.5));
					get_data(tran);//transport
					tran.clear();
					init_flag = true;
				}
			}
			if (init_flag){
				cout << "initialization completed." << endl;
			}
			else cout << "initialization failed." << endl;
		}


		if (!listening()){
			continue;
		}

		if (SE_flag == 100){
			num_iter = 0;
			if (!tran.empty()){
				get_data(tran);
			}
			tran.clear();
			init_flag = false;
			continue;
		}

		num_iter++;

		Mat imgRect;
		Rect rectTmp;
		if (Motion_Flag == 1){
			if (Goal_Flag == 0){
				if (get_white_board(frame, imgRect, rectTmp)){
					Mat gray;
					cvtColor(imgRect, gray, CV_BGR2GRAY);

					//get width and height for creating drawing mat
					int h = imgRect.rows, w = imgRect.cols;

					vector<vector<Point> > contours;
					vector<Vec4i> hierarchy;

					get_contours(gray, contours, hierarchy);

					//number of cnts
					index = 0;
					for (int i = 0; i < (int)contours.size(); i++){
						init();
						vector<Point> cnt;
						if (contourArea(contours[i]) < CONTOUR_AREA_MIN_VAL)//to be seen
							continue;
						else
							cnt = contours[i];

						//area
						area = int(contourArea(contours[i]) + 0.5);

						//center
						Moments M = moments(cnt);
						double center_x = M.m10 / M.m00, center_y = M.m01 / M.m00;

						center_x_true = int(center_x+0.5);
						center_y_true = int(center_y+0.5);

						//angel
						RotatedRect minRect;
						minRect = minAreaRect(Mat(cnt));
						angel = int(minRect.angle + 0.5);
						if (minRect.size.width < minRect.size.height) {
							angel = 90 + angel;
							angel = 180 - angel;
						}
						else{
							angel = -angel;
						}

						//color
						color = judge_color(imgRect, center_x, center_y);

						//shape
						shape = judge_shape(cnt, h, w);

						if (shape != -1)index++;
						else continue;

						tran.push_back(103);
						tran.push_back(index + num_iter * 10);
						tran.push_back(color);
						tran.push_back(center_x_true);
						tran.push_back(center_y_true);
						tran.push_back(angel);
						tran.push_back(area);
						tran.push_back(shape);
						get_data(tran);//transport
						tran.clear();
					}
				}
			}else{
				if (get_white_board(frame, imgRect, rectTmp)){
					//imshow("debug", imgRect);
					Mat gray;
					cvtColor(imgRect, gray, CV_BGR2GRAY);

					vector<vector<Point> > contours;
					vector<Vec4i> hierarchy;

					get_contours(gray, contours, hierarchy);

					index = 0;
					for (int i = 0; i < (int)contours.size(); i++){
						init();
						vector<Point> cnt;
						if (contourArea(contours[i]) < CONTOUR_AREA_MIN_VAL)//to be seen
							continue;
						else
							cnt = contours[i];

						//area
						area = int(contourArea(contours[i])+0.5);
						cout << "area:" << area << endl;

						//center
						Moments M = moments(cnt);
						double center_x = M.m10 / M.m00, center_y = M.m01 / M.m00;

						center_x_true = int(center_x+0.5);
						center_y_true = int(center_y+0.5);

						//angel
						angel = 0;

						printf("center:%d,%d\n", center_x_true, center_y_true);

						//shape
						shape = judge_shape_entity(cnt);

						if (shape != -1)index++;
						else continue;

						tran.push_back(103);
						tran.push_back(index + num_iter * 10);
						tran.push_back(shape);
						tran.push_back(center_x_true);
						tran.push_back(center_y_true);
						tran.push_back(angel);
						tran.push_back(area);
						tran.push_back(color);
						get_data(tran);//transport
						tran.clear();
					}
				}
			}
		}
		 else{
			 if (Goal_Flag == 0){
				 if (0 == (num_iter - 1) % 5 && num_iter != 1){
					 if (get_white_board(frame, imgRect, rectTmp)){
						 Mat gray;
						 cvtColor(imgRect, gray, CV_BGR2GRAY);

						 //get width and height for creating drawing mat
						 int h = imgRect.rows, w = imgRect.cols;

						 vector<vector<Point> > contours;
						 vector<Vec4i> hierarchy;

						 get_contours(gray, contours, hierarchy);

						 //number of cnts
						 index = 0;
						 for (int i = 0; i < (int)contours.size(); i++){
							 init();
							 vector<Point> cnt;
							 if (contourArea(contours[i]) < CONTOUR_AREA_MIN_VAL)//to be seen
								 continue;
							 else
								 cnt = contours[i];

							 //area
							 area = int(contourArea(contours[i])+0.5);

							 //center
							 Moments M = moments(cnt);
							 double center_x = M.m10 / M.m00, center_y = M.m01 / M.m00;

							 center_x_true = int(center_x+0.5);
							 center_y_true = int(center_y+0.5);

							 //angel
							 RotatedRect minRect;
							 minRect = minAreaRect(Mat(cnt));
							 angel = int(minRect.angle+0.5);
							 if (minRect.size.width < minRect.size.height) {
								 angel = 90 + angel;
								 angel = 180 - angel;
							 }
							 else{
								 angel = -angel;
							 }

							 //color
							 color = judge_color(imgRect, center_x, center_y);

							 //shape
							 shape = judge_shape(cnt, h, w);

							 if (shape != -1)index++;
							 else continue;

							 tran.push_back(103);
							 tran.push_back(index + num_iter / 5 * 10);
							 tran.push_back(color);
							 tran.push_back(center_x_true);
							 tran.push_back(center_y_true);
							 tran.push_back(angel);
							 tran.push_back(area);
							 tran.push_back(shape);
						 }
					 }
				 }
			 }
			 else{
				 if (0 == (num_iter - 1) % 5 && num_iter != 1){
					 if (get_white_board(frame, imgRect, rectTmp)){
						 //imshow("debug", imgRect);
						 Mat gray;
						 cvtColor(imgRect, gray, CV_BGR2GRAY);

						 vector<vector<Point> > contours;
						 vector<Vec4i> hierarchy;

						 get_contours(gray, contours, hierarchy);

						 index = 0;
						 for (int i = 0; i < (int)contours.size(); i++){
							 init();
							 vector<Point> cnt;
							 if (contourArea(contours[i]) < CONTOUR_AREA_MIN_VAL)//to be seen
								 continue;
							 else
								 cnt = contours[i];

							 //area
							 area = int(contourArea(contours[i])+0.5);
							 cout << "area:" << area << endl;

							 //center
							 Moments M = moments(cnt);
							 double center_x = M.m10 / M.m00, center_y = M.m01 / M.m00;

							 center_x_true = int(center_x+0.5);
							 center_y_true = int(center_y+0.5);

							 //angel
							 angel = 0;

							 printf("center:%d,%d\n", center_x_true, center_y_true);

							 //shape
							 shape = judge_shape_entity(cnt);

							 if (shape != -1)index++;
							 else continue;

							 tran.push_back(103);
							 tran.push_back(index + num_iter / 5 * 10);
							 tran.push_back(shape);
							 tran.push_back(center_x_true);
							 tran.push_back(center_y_true);
							 tran.push_back(angel);
							 tran.push_back(area);
							 tran.push_back(color);
						 }
					 }
				 }
			 }
		}
	}
}
