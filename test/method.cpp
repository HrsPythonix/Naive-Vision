#include "method.h"

#pragma comment(lib, "ws2_32.lib")
#define SERVER_PORT 5050//侦听端口

int a[7] = { 0 };
int b[5] = { 1, 2, 3, 4, 5 };
int j=0;
int data;
int index = 0;
int color = 0;
int center_x_true = 0;
int center_y_true = 0;
int angel = 0;
int area =0;
int shape = 0;
int SE_flag = 101;
int Goal_Flag = 0;//(0:色块目标，1 : 实物照片)
int Motion_Flag = 1;//(0:速度模式，1 : 位置模式)；
int Vel_Pos = 30;//(当前速度或位置)；

WSADATA wsaData;
SOCKET oldSocket, newSocket;

//客户地址长度
int iLen = 0;
//发送的数据长度
int iSend = 0;
//接收的数据长度
int ircv = 0;
//处世要发送给客户的信息
char buf[20] = "I am a server";
//接收来自用户的信息
char fromcli[512];
//客户和服务器的SOCKET地址结构
struct sockaddr_in ser, cli;

vector<Point> ksf_cnt, cola_cnt, ao_cnt, double_cnt;

bool sortFun(const Rect& r1, const Rect& r2)
{
	return (r1.area() > r2.area());
}

//get contours from white board and store them in contours_output
void get_contours(Mat & image, vector<vector<Point> > & contours_output, vector<Vec4i> & hierarchy_output){
	GaussianBlur(image, image, Size(7, 7), 1.5, 1.5);
	Mat threshold_output;

	threshold(image, threshold_output, THRESH_VAL, 255, THRESH_BINARY_INV);
	Mat kernel = Mat::ones(7, 7, CV_8U);
	morphologyEx(threshold_output, threshold_output, MORPH_OPEN, kernel);

	//imshow("roi", threshold_output);

	findContours(threshold_output, contours_output, hierarchy_output, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
}

//judge color given image and point and return
int judge_color(Mat & image, double center_x, double center_y){
	Mat hsv;
	cvtColor(image, hsv, CV_BGR2HSV);
	blur(hsv, hsv, Size(7, 7));
	//imshow("hsv", hsv);

	IplImage temp = (IplImage)hsv;
	IplImage *prt = &temp;

	int hue = (uint8_t)prt->imageData[int(center_y) * prt->widthStep + int(center_x) * 3 + 0],
		v = (uint8_t)prt->imageData[int(center_y) * prt->widthStep + int(center_x) * 3 + 2];

	cout << "hue:" << hue << endl;
	cout << "value:" << v << endl;
	if (v <= 46){
		printf("color:black\n");
		return BLACK;
	}
	else if (hue <= 10 || hue >= 156){
		printf("color:red\n");
		return RED;
	}
	else if (hue >= 11 && hue <= 34){
		printf("color:yellow\n");
		return YELLOW;
	}
	else if (hue >= 35 && hue <= 99){
		printf("color:green\n");
		return GREEN;
	}
	else if (hue >= 100 && hue <= 124){
		printf("color:blue\n");
		return BLUE;
	}
	else{
		cout << "color not recognized" << endl;
		return -1;
	}
}

//calculate the rate of a rect
double cal_rate(Point2f rect_points[]){
	float rect_w = (rect_points[0].x - rect_points[1].x)*(rect_points[0].x - rect_points[1].x)
		+ (rect_points[0].y - rect_points[1].y)*(rect_points[0].y - rect_points[1].y);

	float rect_h = (rect_points[1].x - rect_points[2].x)*(rect_points[1].x - rect_points[2].x)
		+ (rect_points[1].y - rect_points[2].y)*(rect_points[1].y - rect_points[2].y);

	double rate = rect_w / rect_h;
	if (rate < 1)
		rate = 1.0 / rate;
	return rate;
}

//get the rect contour
void get_rect_contour(Mat & rect_drawing, Point2f rect_points[], vector<vector<Point> > & contour_rect, vector<Vec4i> & hierarchy_rect){
	for (int j = 0; j < 4; j++)
		line(rect_drawing, rect_points[j], rect_points[(j + 1) % 4], 255, 1);
	threshold(rect_drawing, rect_drawing, 120, 255, THRESH_BINARY);
	//imshow("rect", rect_drawing);

	findContours(rect_drawing, contour_rect, hierarchy_rect, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
}

//get the circle contour
void get_cir_contour(Mat & circle_drawing, vector<Point> & cnt, vector<vector<Point> > & contour_circle, vector<Vec4i> & hierarchy_circle){
	Point2f center;
	float radius;
	minEnclosingCircle((Mat)cnt, center, radius);
	circle(circle_drawing, center, (int)radius, 255, 1);

	threshold(circle_drawing, circle_drawing, 127, 255, THRESH_BINARY);

	//imshow("circle or ellispe", circle_drawing);

	findContours(circle_drawing, contour_circle, hierarchy_circle, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
}

//get the ellipse contour
void get_ell_contour(Mat & ellipse_drawing, vector<Point> & cnt, vector<vector<Point> > & contour_ellipse, vector<Vec4i> & hierarchy_ellipse){
	RotatedRect minEllipse;
	minEllipse = fitEllipse(Mat(cnt));
	ellipse(ellipse_drawing, minEllipse, 255, 1);

	threshold(ellipse_drawing, ellipse_drawing, 127, 255, THRESH_BINARY);

	//imshow("circle or ellispe", ellipse_drawing);

	findContours(ellipse_drawing, contour_ellipse, hierarchy_ellipse, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
}

//judge shape and return
int judge_shape(vector<Point> & cnt, int h, int w){

	RotatedRect minRect;
	minRect = minAreaRect(Mat(cnt));
	Point2f rect_points[4];
	minRect.points(rect_points);

	//rate
	double rate = cal_rate(rect_points);

	//get rect contour
	Mat rect_drawing = Mat::zeros(h, w, CV_8U);
	vector<vector<Point> > contour_rect;
	vector<Vec4i> hierarchy_rect;
	get_rect_contour(rect_drawing, rect_points, contour_rect, hierarchy_rect);

	if (contour_rect.size() == 0){
		cerr << "error:contour_rect is empty." << endl;
		return -1;
	}
	else{
		//cal similarity between rect and cnt
		double sim_re = matchShapes(cnt, contour_rect[0], 1, 0.0);

		if (rate < 1.1){ //square or circle
			Mat circle_drawing = Mat::zeros(h, w, CV_8U);
			vector<vector<Point> > contour_circle;
			vector<Vec4i> hierarchy_circle;
			get_cir_contour(circle_drawing, cnt, contour_circle, hierarchy_circle);

			if (contour_circle.size() == 0){
				cerr << "error:contour of circle is empty" << endl;
				return -1;
			}
			else{

				double sim_cir = matchShapes(cnt, contour_circle[0], 1, 0.0);

				if (sim_cir < sim_re){
					cout << "It's a circle." << endl;
					return CIRCLE;
				}
				else {
					cout << "It's a square." << endl;
					return SQUARE;
				}
			}
		}
		else{ //ellipse or rect
			if (cnt.size() < 4){
				cerr << "contour is not correct" << endl;
				return -1;
			}
			else{
				Mat ellipse_drawing = Mat::zeros(h, w, CV_8U);

				vector<vector<Point> > contour_ellipse;
				vector<Vec4i> hierarchy_ellipse;

				get_ell_contour(ellipse_drawing, cnt, contour_ellipse, hierarchy_ellipse);

				if (contour_ellipse.size() == 0){
					cerr << "error:contour of ellipse is empty" << endl;
					return -1;
				}
				else{
					//cal similarity between ellipse and cnt
					double sim_ell = matchShapes(cnt, contour_ellipse[0], 1, 0.0);

					if (sim_ell < sim_re){
						cout << "It's a ellipse." << endl;
						return ELLIPSE;
					}
					else {
						cout << "It's a rectangle." << endl;
						return RECTANGLE;
					}
				}
			}
		}
	}
}

bool get_white_board_init(Mat & frame, Mat & imgRect, Rect & rectTmp){

	/*
	Mat imgblur, imgCanny;
	blur(frame, imgblur, cv::Size(5, 5));
	cvtColor(imgblur, imgblur, CV_BGR2GRAY);
	threshold(imgblur, imgCanny, 120, 255, 0);
	Canny(imgCanny, imgCanny, 50, 100);

	vector<vector<Point> > _contours;
	vector<Vec4i> heirarchy;
	vector<Rect> _rects;
	findContours(imgCanny, _contours, heirarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

	for (int i = 0; i < (int)_contours.size(); i++) {
	vector<Point>  approxOut;
	approxPolyDP(_contours[i], approxOut, 5, true);
	Rect tmpRect = boundingRect(approxOut);
	if (tmpRect.area() < 100000)//300 * 300
	_rects.push_back(tmpRect);
	}
	if (_rects.size() != 0){
	sort(_rects.begin(), _rects.end(), sortFun);
	rectTmp = _rects[0];

	Rect rectFinal;
	rectFinal.x = rectTmp.x + rectTmp.width*0.1;
	rectFinal.y = rectTmp.y + rectTmp.height*0.1;
	rectFinal.width = rectTmp.width*0.8;
	rectFinal.height = rectTmp.height*0.8;
	if ((rectFinal.x + rectFinal.width) < imgblur.cols && (rectFinal.y + rectFinal.height) < imgblur.rows){
	imgRect = frame(rectFinal);
	return true;
	}
	else{
	cerr << "white board not find!" << endl;
	return false;
	}
	}
	else{
	cerr << "white board not find!" << endl;
	return false;
	}*/

	Rect  ROI(frame.cols / 2 - WITHE_BOARD_SIZE / 2, frame.rows / 2 - WITHE_BOARD_SIZE / 2, WITHE_BOARD_SIZE, WITHE_BOARD_INIT_SIZE);
	imgRect = frame(ROI);
	return true;
}

//get the white board, return a flag showing whether it succeeds
bool get_white_board(Mat & frame, Mat & imgRect, Rect & rectTmp){

	/*
	Mat imgblur, imgCanny;
	blur(frame, imgblur, cv::Size(5, 5));
	cvtColor(imgblur, imgblur, CV_BGR2GRAY);
	threshold(imgblur, imgCanny, 120, 255, 0);
	Canny(imgCanny, imgCanny, 50, 100);

	vector<vector<Point> > _contours;
	vector<Vec4i> heirarchy;
	vector<Rect> _rects;
	findContours(imgCanny, _contours, heirarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

	for (int i = 0; i < (int)_contours.size(); i++) {
	vector<Point>  approxOut;
	approxPolyDP(_contours[i], approxOut, 5, true);
	Rect tmpRect = boundingRect(approxOut);
	if (tmpRect.area() < 100000)//300 * 300
	_rects.push_back(tmpRect);
	}
	if (_rects.size() != 0){
	sort(_rects.begin(), _rects.end(), sortFun);
	rectTmp = _rects[0];

	Rect rectFinal;
	rectFinal.x = rectTmp.x + rectTmp.width*0.1;
	rectFinal.y = rectTmp.y + rectTmp.height*0.1;
	rectFinal.width = rectTmp.width*0.8;
	rectFinal.height = rectTmp.height*0.8;
	if ((rectFinal.x + rectFinal.width) < imgblur.cols && (rectFinal.y + rectFinal.height) < imgblur.rows){
	imgRect = frame(rectFinal);
	return true;
	}
	else{
	cerr << "white board not find!" << endl;
	return false;
	}
	}
	else{
	cerr << "white board not find!" << endl;
	return false;
	}*/

	Rect  ROI(frame.cols / 2 - WITHE_BOARD_SIZE / 2, frame.rows / 2 - WITHE_BOARD_SIZE/2, WITHE_BOARD_SIZE, WITHE_BOARD_SIZE);
	imgRect = frame(ROI);
	return true;
}

//get the crossline, return a flag showing whether it succeeds,storing the results in results[3]
bool get_cross_line(Mat & imgRect, Mat & frame, Rect & rectTmp, double results[]){
	//imshow("line+rect", imgRect);
	Mat canny_output;
	int thresh = 10;
	Canny(imgRect, canny_output, thresh, thresh * 2, 3);
	//imshow("canny", canny_output);
	vector<Vec4i> lines;
	Mat imgLines = frame;
	HoughLinesP(canny_output, lines, 1, CV_PI / 180, 50, 70, 20);//找直线
	vector<Vec4i> lines_M;
	for (size_t i = 0; i < lines.size(); i++){
		Vec4i L = lines[i];
		if ((L[0] + L[2]) / 2.0 <imgRect.cols*0.65 && (L[0] + L[2]) / 2.0 >imgRect.cols*0.35 && (L[1] + L[3]) / 2.0 <imgRect.rows*0.65 && (L[1] + L[3]) / 2.0 >imgRect.rows*0.35){
			lines_M.push_back(L);
		}
	}
	if (lines_M.size() > 1){
		vector<Point > vec_point;
		cout << "rows " << imgRect.rows << " cols " << imgRect.cols << endl;;

		for (size_t i = 0; i < lines_M.size(); i++){
			for (size_t j = i + 1; j < lines_M.size(); j++){
				double x1 = lines_M[i][0], y1 = lines_M[i][1];
				double x2 = lines_M[i][2], y2 = lines_M[i][3];
				double x3 = lines_M[j][0], y3 = lines_M[j][1];
				double x4 = lines_M[j][2], y4 = lines_M[j][3];
				Point  _point;
				_point.x = ((x1*y2 - y1*x2) *(x3 - x4) - (x1 - x2)*(x3*y4 - y3*x4)) / ((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));
				_point.y = ((x1*y2 - y1*x2) *(y3 - y4) - (y1 - y2)*(x3*y4 - y3*x4)) / ((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));
				//判断直线焦点是否在中心范围内
				if (_point.x<imgRect.cols*0.65 && _point.x>imgRect.cols*0.35 && _point.y<imgRect.rows*0.65 && _point.y>imgRect.rows*0.35){
					vec_point.push_back(_point);
				}
			}
		}

		if (vec_point.size() != 0){
			//因为3mm的黑色线边缘检测再找直线分别是横竖两条，故求均值的中心
			results[0] = results[1] = 0.0;
			for (size_t i = 0; i < vec_point.size(); i++){
				results[0] = vec_point[i].x + results[0];
				results[1] = vec_point[i].y + results[1];
			}
			results[0] = results[0] / vec_point.size() + rectTmp.x + rectTmp.width*0.1;
			results[1] = results[1] / vec_point.size() + rectTmp.y + rectTmp.height*0.1;
			cout << "_cross point " << results[0] << " " << results[1] << endl;
			Vec4i ML;
			double length = 0;
			for (size_t i = 0; i < lines_M.size(); i++){
				Vec4i L = lines_M[i];//在原图显示
				double x1 = L[0] + rectTmp.x + rectTmp.width*0.1;
				double y1 = L[1] + rectTmp.y + rectTmp.height*0.1;
				double x2 = L[2] + rectTmp.x + rectTmp.width*0.1;
				double y2 = L[3] + rectTmp.y + rectTmp.height*0.1;
				double length_tmp = sqrt((x1 - x2)* (x1 - x2) + (y1 - y2)*(y1 - y2));
				if (length_tmp > length){
					length = length_tmp;
					ML[0] = x1;
					ML[1] = y1;
					ML[2] = x2;
					ML[3] = y2;
				}
				line(imgLines, Point(x1, y1), Point(x2, y2), Scalar(255, 0, 0), 1, CV_AA);
			}
			results[2] = abs(90 - atan(long((ML[0] - ML[2]) / (ML[1] - ML[3])) / CV_PI * 180));
			if (results[2] > 45)
				results[2] = 90 - results[2];
			cout << "agree " << results[2] << endl;
			cout << "length " << length << endl;
			results[3] = length;
			//imshow("lines", imgLines);
			return true;
		}
		else{
			cerr << "error occurs while processing the cross" << endl;
			return false;
		}
	}
	else{
		cerr << "error occurs while processing the cross" << endl;
		return false;
	}
}

//get the standard contour
void get_std_contours(Mat & image, vector<vector<Point> > & contours_output, vector<Vec4i> & hierarchy_output){
	GaussianBlur(image, image, Size(7, 7), 1.5, 1.5);
	Mat threshold_output;

	threshold(image, threshold_output, THRESH_VAL, 255, THRESH_BINARY_INV);
	Mat kernel = Mat::ones(7, 7, CV_8U);
	morphologyEx(threshold_output, threshold_output, MORPH_CLOSE, kernel);
	morphologyEx(threshold_output, threshold_output, MORPH_OPEN, kernel);

	//imshow("roi", threshold_output);
	waitKey(0);

	findContours(threshold_output, contours_output, hierarchy_output, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
}

//judge which entity
int judge_shape_entity(vector<Point> & cnt){
	double sim[10];
	sim[KSF] = matchShapes(cnt, ksf_cnt, 1, 0.0);//ksf
	sim[COLA] = matchShapes(cnt, cola_cnt, 1, 0.0);//cola
	sim[AOLIAO] = matchShapes(cnt, ao_cnt, 1, 0.0);//aoliao
	sim[DOUBLEMINT] = matchShapes(cnt, double_cnt, 1, 0.0);//doublemint

	int min = 6;
	for (int i = 6; i <= 9; i++){
		if (sim[i] < sim[min])min = i;
	}

	vector<vector<Point> > contours;

	switch (min)
	{
	case KSF:contours.push_back(ksf_cnt); break;
	case COLA:contours.push_back(cola_cnt); break;
	case AOLIAO:contours.push_back(ao_cnt); break;
	case DOUBLEMINT:contours.push_back(double_cnt); break;
	default:break;
	}

	contours.push_back(cnt);

	vector<int> ctrSelec;

	cout << contours.size() << endl;

	for (int i = 0; i < contours.size(); i++)
		ctrSelec.push_back(i);

	vector<vector<Point2d> > z;
	vector<vector<Point2d> > Z;

	z.resize(ctrSelec.size());
	Z.resize(ctrSelec.size());
	for (int i = 0; i<ctrSelec.size(); i++)
	{
		vector<Point2d> c = ReSampleContour(contours[ctrSelec[i]], 1024);
		for (int j = 0; j<c.size(); j++)
			z[i].push_back(c[(j + i * 10) % c.size()]);
		dft(z[i], Z[i], DFT_SCALE | DFT_REAL_OUTPUT);//转换到频域，小ｚ给大Z
	}

	int indRef = 0;
	MatchDescriptor md;
	md.sContour = Z[indRef];//原始目标轮廓
	md.nbDesFit = 20;
	vector<float> alpha, phi, s;
	vector<vector<Point> > ctrRotated;
	alpha.resize(ctrSelec.size());
	phi.resize(ctrSelec.size());
	s.resize(ctrSelec.size());

	for (int i = 0; i<ctrSelec.size(); i++)
	{
		md.AjustementRtSafe(Z[i], alpha[i], phi[i], s[i]);//和每一个目标轮廓比较旋转角和大小
		cout << " i " << i << " alpha" << alpha[i] << " phi " << phi[i] << " s " << s[i] << endl;
		complex<float> expitheta = s[i] * complex<float>(cos(phi[i]), sin(phi[i]));
		cout << "Contour " << indRef << " with " << i << " origin " << alpha[i] << " and rotated of " << phi[i] * 180 / md.pi 
		     << " and scale " << s[i] << " Distance between contour is " << md.Distance(expitheta, alpha[i]) << " " << endl;
	}

	angel = int (phi[1] * 180 / md.pi + 0.5);//angel
	if (angel < 0){
		angel = 360 + angel;
	}

	return min;
}

//get standard contours and store them in ksf_cnt,cola_cnt,ao_cnt,double_cnt
void get_std_cnt(){
	Mat ksf = imread("ksf.jpg", 0);
	Mat cola = imread("cola.jpg", 0);
	Mat ao = imread("ao.jpg", 0);
	Mat dm = imread("double.jpg", 0);

	vector<vector<Point> > ksf_contours;
	vector<Vec4i> ksf_hierarchy;
	vector<vector<Point> > cola_contours;
	vector<Vec4i> cola_hierarchy;
	vector<vector<Point> > ao_contours;
	vector<Vec4i> ao_hierarchy;
	vector<vector<Point> > double_contours;
	vector<Vec4i> double_hierarchy;

	get_std_contours(ksf, ksf_contours, ksf_hierarchy);
	get_std_contours(cola, cola_contours, cola_hierarchy);
	get_std_contours(ao, ao_contours, ao_hierarchy);
	get_std_contours(dm, double_contours, double_hierarchy);

	if (!ksf_contours.empty() && !cola_contours.empty() && !ao_contours.empty() && !double_contours.empty()){
		ksf_cnt = ksf_contours[0];
		cola_cnt = cola_contours[0];
		ao_cnt = ao_contours[0];
		double_cnt = double_contours[0];
		cout << "succeed to get standard contours." << endl;
	}
	else{
		cerr << "failed to get standard contours." << endl;
	}
}

//reset parameter each loop
void init(){
	color = 0;
	center_x_true = 0;
	center_y_true = 0;
	angel = 0;
	area = 0;
	shape = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////

bool listening(){
		//服务器初始化接收缓冲区
		memset(fromcli, 0, 512);

		ircv = recv(newSocket, fromcli, sizeof(fromcli), 0);
		if (ircv == SOCKET_ERROR)
		{
			//cout << "rcv() failed:" << WSAGetLastError() << endl;
			return false;
		}
		else if (ircv == 0)
			return false;
		else if (ircv == WSAEWOULDBLOCK)
			return false;
		else {
			cout << "-----服务器接收的内容为--------" << fromcli << endl;
		}

		if (atoi(fromcli)==101){

			SE_flag = 101;

			memset(fromcli, 0, 512);
			do{
				ircv = recv(newSocket, fromcli, sizeof(fromcli), 0);
			} while (ircv == SOCKET_ERROR);

			Goal_Flag = atoi(fromcli);
			cout << Goal_Flag << endl;

			memset(fromcli, 0, 512);
			do{
				ircv = recv(newSocket, fromcli, sizeof(fromcli), 0);
			} while (ircv == SOCKET_ERROR);

			Motion_Flag = atoi(fromcli);
			cout << Motion_Flag << endl;

			memset(fromcli, 0, 512);
			do{
				ircv = recv(newSocket, fromcli, sizeof(fromcli), 0);
			} while (ircv == SOCKET_ERROR);
			
			Vel_Pos = atoi(fromcli);
			cout << Vel_Pos << endl;
		}
		else{
			SE_flag = 100;
		}
		return true;
}

void server_init(){

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "failed to load winsock" << endl;
		return;
	}


	cout << "server waiting" << endl;
	cout << "---------------" << endl;



	//创建服务器端帧听SOCKET
	oldSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (oldSocket == INVALID_SOCKET)
	{
		cout << "socket() failed:" << WSAGetLastError() << endl;
		return;
	}


	//以下是建立服务器端的SOCKET地址结构
	ser.sin_family = AF_INET;
	ser.sin_port = htons(5050);
	//使用系统指定的ip地址INADDR_ANY
	// ser.sin_addr.s_addr=htonl(INADDR_ANY);
	ser.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (bind(oldSocket, (LPSOCKADDR)&ser, sizeof(ser)) == SOCKET_ERROR)
	{
		cout << "bind() failed:" << WSAGetLastError() << endl;
		return;
	}

	//进入侦听状态
	if (listen(oldSocket, 5) == SOCKET_ERROR)
	{
		cout << "listen() failed:" << WSAGetLastError() << endl;
		return;
	}

	//接收客户端的连接
	iLen = sizeof(cli);

	newSocket = accept(oldSocket, (struct sockaddr*)&cli, &iLen);//产生一个新的SOCKET

	if (newSocket == INVALID_SOCKET)
	{
		cout << "accept() failed:" << WSAGetLastError() << endl;
		return;
	}
	unsigned long ul = 1;
	int ret1, ret2;
	ret1 = ioctlsocket(newSocket, FIONBIO, (unsigned long *)&ul);
	ret2 = ioctlsocket(oldSocket, FIONBIO, (unsigned long *)&ul);
	if (ret1 == SOCKET_ERROR || ret2 == SOCKET_ERROR){
		cerr << "socket convert failed" << endl;
	}
}

void get_data(vector <int> tran){
	for (int i = 0; i < tran.size(); i++){
		memset(fromcli, 0, 512);
		if (tran[i] < 0){
			itoa(-tran[i], fromcli, 10);
			for (int j = strlen(fromcli); j >= 0; j--){
				fromcli[j + 1] = fromcli[j];
			}
			fromcli[0] = '-';
		}
		else itoa(tran[i], fromcli, 10);
		ircv = send(newSocket, fromcli, sizeof(fromcli), 0);
		if (ircv == 0)
			return;
		else if (ircv == SOCKET_ERROR)
		{
			cout << "send() failed:" << WSAGetLastError() << endl;
			return;
		}
		cout << "-----服务器端发送内容为----" << fromcli << endl;
	}
}