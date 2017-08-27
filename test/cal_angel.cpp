#include "cal_angel.h"

void MatchDescriptor::InitFrequence()
{
	long i;
	int nbElt = sContour.size();
	frequence.resize(sContour.size());

	for (i = 0; i <= nbElt / 2; i++)
		frequence[i] = 2 * pi*(float)i / nbElt;
	for (i = nbElt / 2 + 1; i<nbElt; i++)
		frequence[i] = 2 * pi*(float)(i - nbElt) / nbElt;
}


void MatchDescriptor::falpha(float x, float *fn, float *df)
{
	long    n, nbElt = sContour.size();
	float   s1 = 0, s2 = 0, s3 = 0, s4 = 0;
	float   ds1 = 0, ds2 = 0, ds3 = 0, ds4 = 0;

	for (n = 1; n <= nbDesFit; n++)
	{
		s1 += rho[n] * sin(psi[n] + frequence[n] * x) +
			rho[nbElt - n] * sin(psi[nbElt - n] + frequence[nbElt - n] * x);
		s2 += frequence[n] * rho[n] * cos(psi[n] + frequence[n] * x) +
			frequence[nbElt - n] * rho[nbElt - n] * cos(psi[nbElt - n] + frequence[nbElt - n] * x);
		s3 += rho[n] * cos(psi[n] + frequence[n] * x) +
			rho[nbElt - n] * cos(psi[nbElt - n] + frequence[nbElt - n] * x);
		s4 += frequence[n] * rho[n] * sin(psi[n] + frequence[n] * x) +
			frequence[nbElt - n] * rho[nbElt - n] * sin(psi[nbElt - n] + frequence[nbElt - n] * x);
		ds1 += frequence[n] * rho[n] * cos(psi[n] + frequence[n] * x) +
			frequence[nbElt - n] * rho[nbElt - n] * cos(psi[nbElt - n] + frequence[nbElt - n] * x);
		ds2 += -frequence[n] * frequence[n] * rho[n] * sin(psi[n] + frequence[n] * x) -
			frequence[nbElt - n] * frequence[nbElt - n] * rho[nbElt - n] * sin(psi[nbElt - n] + frequence[nbElt - n] * x);
		ds3 += -frequence[n] * rho[n] * sin(psi[n] + frequence[n] * x) -
			frequence[nbElt - n] * rho[nbElt - n] * sin(psi[nbElt - n] + frequence[nbElt - n] * x);
		ds4 += frequence[n] * frequence[n] * rho[n] * cos(psi[n] + frequence[n] * x) +
			frequence[nbElt - n] * frequence[nbElt - n] * rho[nbElt - n] * cos(psi[nbElt - n] + frequence[nbElt - n] * x);
	}
	*fn = s1 * s2 - s3 *s4;
	*df = ds1 * s2 + s1 * ds2 - ds3 * s4 - s3 * ds4;
}

float MatchDescriptor::AjustementRtSafe(vector<Point2d> &c, float &alphaMin, float &phiMin, float &sMin)
{
	long            n, nbElt = sContour.size();
	float           s1, s2, sign1, sign2, df, x1 = nbElt, x2 = nbElt, dx;
	float           dist, distMin = 10000, alpha, s, phi;
	complex<float>  j(0, 1), zz;

	InitFrequence();
	rho.resize(nbElt);
	psi.resize(nbElt);

	b.resize(nbElt);
	a.resize(nbElt);
	if (nbElt != c.size())
		return -1;
	for (n = 0; n<nbElt; n++)
	{
		b[n] = complex<float>(sContour[n].x, sContour[n].y);
		a[n] = complex<float>(c[n].x, c[n].y);
		zz = conj(a[n])*b[n];
		rho[n] = abs(zz);
		psi[n] = arg(zz);
	}
	float xp = -nbElt, fnp, dfp;
	falpha(xp, &fnp, &dfp);

	x1 = nbElt, x2 = nbElt;
	sMin = 1;
	alphaMin = 0;
	phiMin = arg(a[1] / b[1]);
	do
	{
		x2 = x1;
		falpha(x2, &sign2, &df);
		dx = 1;
		x1 = x2;
		do
		{
			x2 = x1;
			x1 -= dx;
			falpha(x1, &sign1, &df);
		} while ((sign1*sign2>0) && (x1>-nbElt));
		if (sign1*sign2<0)
		{
			alpha = rtsafe(x1, x2, 1e-8);
			falpha(alpha, &sign1, &df);
			//alpha = alpha;
			s1 = 0;
			s2 = 0;
			for (n = 1; n<nbElt; n++)
			{
				s1 += rho[n] * sin(psi[n] + frequence[n] * alpha);
				s2 += rho[n] * cos(psi[n] + frequence[n] * alpha);
			}
			phi = -atan(s1 / s2);
			phi = -atan2(s1, s2);
			s1 = 0;
			s2 = 0;
			for (n = 1; n < nbElt; n++)
			{
				s1 += rho[n] * cos(psi[n] + frequence[n] * alpha + phi);
				s2 += abs(b[n] * conj(b[n]));
			}
			s = s1 / s2;
			zz = s*exp(j*phi);
			if (s>0)
				dist = Distance(zz, alpha);
			else
				dist = 10000;
			if (dist<distMin)
			{
				distMin = dist;
				alphaMin = alpha;
				phiMin = phi;
				sMin = s;
			}
		}
	} while ((x1>-nbElt));
	return distMin;
}


#define MAXIT 100

float MatchDescriptor::rtsafe(float x1, float x2, float xacc)
{
	long j;
	float df, dx, dxold, f, fh, fl;
	float temp, xh, xl, rts;

	falpha(x1, &fl, &df);
	falpha(x2, &fh, &df);
	if (fl < 0.0) {
		xl = x2;
		xh = x1;
	}
	else {
		xh = x2;
		xl = x1;
	}
	rts = 0.5*(x1 + x2);
	dxold = fabs(x2 - x1);
	dx = dxold;
	falpha(rts, &f, &df);
	for (j = 1; j <= MAXIT; j++)
	{
		if ((((rts - xh)*df - f)*((rts - xl)*df - f) >= 0.0)
			|| (fabs(2.0*f) > fabs(dxold*df)))
		{
			dxold = dx;
			dx = 0.5*(xh - xl);
			rts = xl + dx;
			if (xl == rts) return rts;
		}
		else
		{
			dxold = dx;
			dx = f / df;
			temp = rts;
			rts -= dx;
			if (temp == rts)
				return rts;
		}
		if (fabs(dx) < xacc)
			return rts;
		falpha(rts, &f, &df);
		if (f < 0.0)
			xl = rts;
		else
			xh = rts;
	}
	return 0.0;
}

Point2d Echantillon(vector<Point> &c, long i, float l1, float l2, float s)
{

	Point2d d = c[(i + 1) % c.size()] - c[i % c.size()];
	Point2d p = c[i%c.size()];
	p = p + d *float((s - l1) / (l2 - l1));
	return p;
}


vector<Point2d> ReSampleContour(vector<Point> &c, int nbElt)
{
	long        nb = c.size();
	float       l1 = 0, l2, p, d, s;
	vector<Point2d> r;
	int j = 0;
	p = arcLength(c, true);

	l2 = norm(c[j] - c[j + 1]) / p;
	for (int i = 0; i<nbElt; i++)
	{
		s = (float)i / (float)nbElt;
		while (s >= l2)
		{
			j++;
			l1 = l2;
			d = norm(c[j % nb] - c[(j + 1) % nb]);
			l2 = l1 + d / p;
		}
		if ((s >= l1) && (s<l2))
			r.push_back(Echantillon(c, j, l1, l2, s));
	}
	return r;
}