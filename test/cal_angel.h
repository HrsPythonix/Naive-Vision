#ifndef CAL_ANGEL_H_
#define CAL_ANGEL_H_

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <tchar.h>
#include <SDKDDKVer.h>
#include <stdio.h>
#include <winsock2.h>
#include <Windows.h>
#include <iostream>
using namespace cv;
using namespace std;

class MatchDescriptor {
public:
	vector<Point2d> sContour;
	vector<complex<float> > b;
	vector<complex<float> > a;
	vector<float> frequence;
	vector<float> rho, psi;
	double pi;
	int nbDesFit;

public:
	MatchDescriptor(){ nbDesFit = 7; pi = acos(-1.0); };;
	float AjustementRtSafe(vector<Point2d> &c, float &alphaMin, float &phiMin, float &sMin);
	float Ajustement(vector<Point2d> &c, float &alphaMin, float &phiMin, float &sMin);
	void falpha(float x, float *fn, float *df);
	void InitFrequence();
	float rtsafe(float x1, float x2, float xacc);
	float Distance(complex<float> r, float alpha)
	{
		long            i;
		complex<float>      j(0, 1);
		float       d = 0;

		for (i = 1; i <= nbDesFit; i++)
		{
			d += abs(a[i] - b[i] * r*exp(j*float(alpha*frequence[i]))) + abs(a[a.size() - i] - b[a.size() - i] * r*exp(j*float(alpha*frequence[a.size() - i])));
		}
		return d;
	};

};

Point2d Echantillon(vector<Point> &c, long i, float l1, float l2, float s);

vector<Point2d> ReSampleContour(vector<Point> &c, int nbElt);

#endif