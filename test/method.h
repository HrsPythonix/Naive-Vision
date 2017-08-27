#ifndef METHOD_H_
#define	METHOD_H_

#include "cal_angel.h"

#pragma comment(lib, "ws2_32.lib")
#define SERVER_PORT 5050//侦听端口

//color param
#define BLACK 1
#define RED 2
#define YELLOW 3
#define GREEN 4
#define BLUE 5

//shape of color block
#define CIRCLE 1
#define SQUARE 2
#define ELLIPSE 4
#define RECTANGLE 3

//shape of entity
#define KSF 8
#define COLA 6
#define AOLIAO 9
#define DOUBLEMINT 7

//some param
#define CAMERA_HZ 60 //useless
#define CONTOUR_AREA_MIN_VAL 600
#define THRESH_VAL 127
#define WITHE_BOARD_SIZE 280 //边长
#define WITHE_BOARD_INIT_SIZE 168 //边长

typedef unsigned char uint8_t;

extern int a[7];
extern int b[5];
extern int j;
extern int data;
extern int index;
extern int color;
extern int center_x_true;
extern int center_y_true;
extern int angel;
extern int area;
extern int shape;
extern int SE_flag;;
extern int Goal_Flag;//(0:色块目标，1 : 实物照片)
extern int Motion_Flag;//(0:速度模式，1 : 位置模式)；
extern int Vel_Pos;//(当前速度或位置)；
extern WSADATA wsaData;
extern SOCKET oldSocket, newSocket;

//客户地址长度
extern int iLen;
//发送的数据长度
extern int iSend;
//接收的数据长度
extern int ircv;
//处世要发送给客户的信息
extern char buf[20];
//接收来自用户的信息
extern char fromcli[512];
//客户和服务器的SOCKET地址结构
extern struct sockaddr_in ser, cli;

extern vector<Point> ksf_cnt, cola_cnt, ao_cnt, double_cnt;

bool sortFun(const Rect& r1, const Rect& r2);

//get contours from white board and store them in contours_output
void get_contours(Mat & image, vector<vector<Point> > & contours_output, vector<Vec4i> & hierarchy_output);

//judge color given image and point and return
int judge_color(Mat & image, double center_x, double center_y);

//calculate the rate of a rect
double cal_rate(Point2f rect_points[]);

//get the rect contour
void get_rect_contour(Mat & rect_drawing, Point2f rect_points[], vector<vector<Point> > & contour_rect, vector<Vec4i> & hierarchy_rect);

//get the circle contour
void get_cir_contour(Mat & circle_drawing, vector<Point> & cnt, vector<vector<Point> > & contour_circle, vector<Vec4i> & hierarchy_circle);

//get the ellipse contour
void get_ell_contour(Mat & ellipse_drawing, vector<Point> & cnt, vector<vector<Point> > & contour_ellipse, vector<Vec4i> & hierarchy_ellipse);

//judge shape and return
int judge_shape(vector<Point> & cnt, int h, int w);

//get the white board, return a flag showing whether it succeeds
bool get_white_board(Mat & frame, Mat & imgRect, Rect & rectTmp);

bool get_white_board_init(Mat & frame, Mat & imgRect, Rect & rectTmp);

//get the crossline, return a flag showing whether it succeeds,storing the results in results[3]
bool get_cross_line(Mat & imgRect, Mat & frame, Rect & rectTmp, double results[]);

//get the standard contour
void get_std_contours(Mat & image, vector<vector<Point> > & contours_output, vector<Vec4i> & hierarchy_output);

//judge which entity
int judge_shape_entity(vector<Point> & cnt);

//get standard contours and store them in ksf_cnt,cola_cnt,ao_cnt,double_cnt
void get_std_cnt();

//reset parameter each loop
void init();

//get data and transport
void get_data(vector <int> tran);

bool listening();

void server_init();
#endif