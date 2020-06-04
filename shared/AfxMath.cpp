#include "stdafx.h"

#include "AfxMath.h"

#define _USE_MATH_DEFINES
#include <math.h>


namespace Afx {
namespace Math {

void MakeVectors(
	double rForward, double rLeft, double rUp,
	double (& outForward)[3], double (& outRight)[3], double (& outUp)[3]
)
{
	double angle;
	double sr, sp, sy, cr, cp, cy;

	angle = rUp * (M_PI*2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = rLeft * (M_PI*2 / 360);
	sp = sin(angle);
	cp = cos(angle);
	angle = rForward * (M_PI*2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	outForward[0] = cp*cy;
	outForward[1] = cp*sy;
	outForward[2] = -sp;

	outRight[0] = (-1*sr*sp*cy+-1*cr*-sy);
	outRight[1] = (-1*sr*sp*sy+-1*cr*cy);
	outRight[2] = -1*sr*cp;

	outUp[0] = (cr*sp*cy+-sr*-sy);
	outUp[1] = (cr*sp*sy+-sr*cy);
	outUp[2] = cr*cp;
}

bool LUdecomposition(const double matrix[4][4], unsigned char outP[4], unsigned char outQ[4], double outL[4][4], double outU[4][4])
{
    const double* nMatrix[4] = { matrix[0], matrix[1], matrix[2], matrix[3] };
    double* nOutL[4] = { outL[0], outL[1], outL[2], outL[3] };
    double* nOutU[4] = { outU[0], outU[1], outU[2], outU[3] };

    return LUdecompositionEx(nMatrix, outP, outQ, nOutL, nOutU, 4);
}

void SolveWithLU(const double L[4][4], const double U[4][4], const unsigned char P[4], const unsigned char Q[4], const double b[4], double outX[4])
{
    double y[4];
    const double* nL[4] = { L[0], L[1], L[2], L[3] };
    const double* nU[4] = { U[0], U[1], U[2], U[3] };

    return SolveWithLUEx(nL, nU, P, Q, b, outX, 4, y);
}

bool LUdecompositionEx(const double** matrix, unsigned char* outP, unsigned char* outQ, double** outL, double** outU, int size)
{
	for(int i=0; i< size; ++i)
	{
		outP[i] = i; outQ[i] = i;

		for(int j=0; j< size; ++j)
		{
			outU[i][j] = matrix[i][j];
		}
	}

	for(int n=0; n< size -1; ++n)
	{
		int t = -1;
		double maxVal = 0;
		for(int i=n; i< size; ++i)
		{
			double tabs = abs(outU[i][n]);
			if(maxVal < tabs)
			{
				t = i;
				maxVal = tabs;
			}
		}

		if(t < 0)
			return false;

		if(n!=t)
		{
			unsigned char ucTmp = outP[n];
			outP[n] = outP[t];
			outP[t] = ucTmp;

			for(int i=0; i< size; ++i)
			{
				double dTmp = outU[n][i];
				outU[n][i] = outU[t][i];
				outU[t][i] = dTmp;
			}
		}

		for(int i=n+1; i< size; ++i)
		{
			outU[i][n] = outU[i][n]/outU[n][n];
			for(int j=n+1; j< size; ++j)
			{
				outU[i][j] = outU[i][j] -outU[i][n] * outU[n][j];
			}
		}
	}

	for(int i=0; i< size; ++i)
	{
		for(int j=0; j<i; ++j)
		{
			outL[i][j] = outU[i][j];
			outU[i][j] = 0;
		}
		outL[i][i] = 1;
		for(int j=i+1; j< size; ++j)
		{
			outL[i][j] = 0;
		}
	}

	return true;
}

void SolveWithLUEx(const double** L, const double** U, const unsigned char* P, const unsigned char* Q, const double* b, double* outX, int size, double* y)
{
	// solve L*y = b with forward subsitituion:
	for(int i=0; i< size; i++)
	{
		double sum = 0;
		for(int k=0;k<i;k++)
			sum += L[i][k]*y[k];

		y[i] = 1.0/L[i][i] * (b[P[i]] -sum);
	}

	for(int i= size -1; i>=0; i--)
	{
		double sum = 0;
		for(int k=i+1;k< size;k++)
			sum += U[i][k]*outX[Q[k]];
		
		outX[Q[i]] = 1.0/U[i][i] * (y[i] - sum);
	}
}

//int round(double x)
//{
//	return x < 0 ? (int)(x -0.5) : (int)(x +0.5);
//}

// NUMERICAL RECIPES IN C: THE ART OF SCIENTIFIC COMPUTING (ISBN 0-521-43108-5)
void spline(double x[], double y[], int n, bool y1Natural, double yp1, bool ynNatural, double ypn, double y2[])
{
    int i, k;
    double p, qn, sig, un;
    double *u = new double[n - 1 - 1 + 1];

    if (y1Natural)
        y2[0] = u[0] = 0.0f;
    else
    {
        y2[0] = -0.5f;
        u[0] = (3.0f / (x[1] - x[0])) * ((y[1] - y[0]) / (x[1] - x[0]) - yp1);
    }

    for (i = 1; i <= n - 2; i++)
    {
        sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
        p = sig * y2[i - 1] + 2.0f;
        y2[i] = (sig - 1.0f) / p;
        u[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
        u[i] = (6.0f * u[i] / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p;
    }

    if (ynNatural)
        qn = un = 0.0f;
    else
    {
        qn = 0.5f;
        un = (3.0f / (x[n - 1] - x[n - 2])) * (ypn - (y[n - 1] - y[n - 2]) / (x[n - 1] - x[n - 2]));
    }

    y2[n - 1] = (un - qn * u[n - 2]) / (qn * y2[n - 2] + 1.0f);

    for (k = n - 2; k >= 0; k--)
        y2[k] = y2[k] * y2[k + 1] + u[k];

	delete u;
}

// NUMERICAL RECIPES IN C: THE ART OF SCIENTIFIC COMPUTING (ISBN 0-521-43108-5)
void splint(double xa[], double ya[], double y2a[], int n, double x, double *y)
{
    int klo, khi, k;
    double h, b, a;

    klo = 0;
    khi = n - 1;
    while (khi - klo > 1)
    {
        k = (khi + klo) >> 1;
        if (xa[k] > x) khi = k;
        else klo = k;
    }
    h = xa[khi] - xa[klo];
    if (h == 0.0) throw "splint: Bad xa input.";
    a = (xa[khi] - x) / h;
    b = (x - xa[klo]) / h;
    *y = a * ya[klo] + b * ya[khi] + ((a * a * a - a) * y2a[klo] + (b * b * b - b) * y2a[khi]) * (h * h) / 6.0f;
}

////////////////////////////////////////////////////////////////////////////////

#define DZERO (double **)0
#define ZERO (double *)0

double getang(double qi[], double qf[], double e[]);

void rates(
	int n, int maxit, double tol, double wi[], double wf[], double h[],
	double a[], double b[], double c[], double dtheta[], double e[][3],
	double w[][3], double wprev[][3]
);

int bd(
	double e[], double dtheta, int flag, double xin[], double xout[]
);

void rf(
	double e[], double dtheta, double win[], double rhs[]
);

void slew3_init(
	double dt, double dtheta, double e[], double wi[], double ai[],
	double wf[], double af[]
);

void slew3(
	double t, double dt, double qi[], double q[],
	double omega[], double alpha[], double jerk[]
);

double unvec(
	double a[], double au[]
);

void crossp(
	double b[], double c[], double a[]
);

// Note: This function is based on the qspline CC0 project by James McEnnan:
// http://sourceforge.net/projects/qspline-cc0
//
/// <param name="n">number of input points (n >= 4).</param>
/// <param name="maxit">maximum number of iterations.</param>
/// <param name="tol">convergence tolerance (rad/sec) for iteration termination.</param>
/// <param name="wi">initial angular rate vector.</param>
/// <param name="wf">final angular rate vector.</param>
/// <param name="x">input vector of n time values.</param>
/// <param name="y">input vector of quaternion values.</param>
/// <param name="h">out: vector of n-1 x-interval values.</param>
/// <param name="dtheta">out: vector of n-1 rotation angles.</param>
/// <param name="e">out: rray of n-1 rotation axis vectors.</param>
/// <param name="w">out: n intermediate angular rates.</param>
void qspline_init(
	int n, int maxit, double tol, double wi[], double wf[],
	double x[], double y[][4],
	double h[], double dtheta[], double e[][3], double w[][3]
)
{
  int i, j;
  double *a, *b, *c, (*wprev)[3];

  if(n < 4) throw "qspline_init: insufficient input data.\n";

  wprev = new double[n][3];
  a = new double[n-1];
  b = new double[n-1];
  c = new double[n-1];

  for(i = 0;i < n;i++)
    for(j = 0;j < 3;j++)
      w[i][j] = 0.0;

  for(i = 0;i < n - 1;i++)
  {
    h[i] = x[i + 1] - x[i];

    if(h[i] <= 0.0) throw "qspline_init: x is not monotonic.\n";
  }

  /* compute spline coefficients. */

  for(i = 0;i < n - 1;i++)
    dtheta[i] = getang(y[i],y[i + 1],e[i]);

  rates(n,maxit,tol,wi,wf,h,a,b,c,dtheta,e,w,wprev);

  delete c;
  delete b;
  delete a;
  delete [] wprev;
}

// Note: This function is based on the qspline CC0 project by James McEnnan:
// http://sourceforge.net/projects/qspline-cc0
// Todo: if used in a class it could reduce calls to slew3_init.
//
/// <summary>Interpolates a quaternion value.</summary>
/// <param name="n">number of input points (n>=4)</param>
/// <param name="xi">input time</param>
/// <param name="x">input vector of n time values.</param>
/// <param name="y">input vector of n quaternion values.</param>
/// <param name="h">vector of n-1 x-interval values.</param>
/// <param name="dtheta">vector of n-1 rotation angles.</param>
/// <param name="e">array of n-1 rotation axis vectors.</param>
/// <param name="w">n intermediate angular rates.</param>
/// <param name="q">out: interpolated quaternion value.</param>
/// <param name="omega">out: interpolated angular rate value (rad/sec).</param>
/// <param name="alpha">out: interpolated angular acceleration value (rad/sec^2).</param>
void qspline_interp(
	int n, double xi, double x[], double y[][4],
	double h[], double dtheta[], double e[][3], double w[][3], 
	double q[4], double omega[3], double alpha[3]
)
{
	double dum1[3], dum2[3];
	
	int klo, khi, k;

	klo = 0;
	khi = n - 1;
	while (khi - klo > 1)
	{
		k = (khi + klo) >> 1;
		if (x[k] > xi) khi = k;
		else klo = k;
	}

	/* interpolate and output results. */

	slew3_init(h[klo],dtheta[klo],e[klo],w[klo],dum1,w[klo+1],dum2);

    slew3(xi - x[klo],h[klo],y[klo],q,omega,alpha,dum1);
}

// Note: This function has been slighlty modified from it's original (definition only).
double getang(double qi[], double qf[], double e[])
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

purpose

Subroutine getang computes the slew angle and axis between the input initial and
final states.

calling sequence

variable     i/o     description
--------     ---     -----------

qi            i      initial attitude quaternion.

qf            i      final attitude quaternion.

e             o      unit vector along slew eigen-axis.

return value

slew angle in radians

external references

unvec

programming

J. J. McEnnan, May, 2000.

COPYRIGHT (C) 2003 by James McEnnan

    To the extent possible under law, James McEnnan ( jmcennan@mailaps.org )
    has waived all copyright and related or neighboring rights to qspline CC0
    ( http://sourceforge.net/projects/qspline-cc0 ) under the CC0 1.0
    license ( http://creativecommons.org/publicdomain/zero/1.0/ ).
    This work is published from: United States.

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
{
  double dtheta, sa, ca, temp[3];

  temp[0] = qi[3]*qf[0] - qi[0]*qf[3] - qi[1]*qf[2] + qi[2]*qf[1];
  temp[1] = qi[3]*qf[1] - qi[1]*qf[3] - qi[2]*qf[0] + qi[0]*qf[2];
  temp[2] = qi[3]*qf[2] - qi[2]*qf[3] - qi[0]*qf[1] + qi[1]*qf[0];

  ca =  qi[0]*qf[0] + qi[1]*qf[1] + qi[2]*qf[2] + qi[3]*qf[3];

  sa = unvec(temp,e);

  dtheta = 2.0*atan2(sa,ca);

  return dtheta;
}

// Note: This function has been slighlty modified from it's original (definition only).
void rates(
	int n, int maxit, double tol, double wi[], double wf[], double h[],
	double a[], double b[], double c[], double dtheta[], double e[][3],
	double w[][3], double wprev[][3]
)
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

purpose

subroutine rates computes intermediate angular rates for interpolation.

calling sequence

variable     i/o     description
--------     ---     -----------

n             i      number of input data points.

maxit         i      maximum number of iterations.

tol           i      convergence tolerance (rad/sec) for iteration termination.

wi            i      initial angular rate vector.

wf            i      final angular rate vector.

h             i      pointer to vector of time interval values.

a             i      pointer to intermediate work space.

b             i      pointer to intermediate work space.

c             i      pointer to intermediate work space.

dtheta        i      pointer to vector of rotation angles.

e             i      pointer to array of rotation axis vectors.

w             o      pointer to output intermediate angular rate values.

wprev         o      pointer to previous intermediate angular rate values.

return value

none

external references

bd
rf

programming

J. J. McEnnan, April, 2003.

COPYRIGHT (C) 2003 by James McEnnan

    To the extent possible under law, James McEnnan ( jmcennan@mailaps.org )
    has waived all copyright and related or neighboring rights to qspline CC0
    ( http://sourceforge.net/projects/qspline-cc0 ) under the CC0 1.0
    license ( http://creativecommons.org/publicdomain/zero/1.0/ ).
    This work is published from: United States.

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
{
  int i, j, iter;
  double dw, temp1[3], temp2[3];

  iter = 0;

  do                                                 /* start iteration loop. */
  {
    for(i = 1;i < n - 1;i++)
      for(j = 0;j < 3;j++)
        wprev[i][j] = w[i][j];

    /* set up the tridiagonal matrix. d initially holds the RHS vector array;
       it is then overlaid with the calculated angular rate vector array. */

    for(i = 1;i < n - 1;i++)
    {
      a[i] = 2.0/h[i - 1];
      b[i] = 4.0/h[i - 1] + 4.0/h[i];
      c[i] = 2.0/h[i];
  
      rf(e[i - 1],dtheta[i - 1],wprev[i],temp1);

      for(j = 0;j < 3;j++)
        w[i][j] = 6.0*(dtheta[i - 1]*e[i - 1][j]/(h[i - 1]*h[i - 1]) +
                       dtheta[i    ]*e[i    ][j]/(h[i    ]*h[i    ])) -
                  temp1[j];
    }
  
    bd(e[0    ],dtheta[0    ],1,wi,temp1);
    bd(e[n - 2],dtheta[n - 2],0,wf,temp2);
  
    for(j = 0;j < 3;j++)
    {
      w[1    ][j] -= a[1    ]*temp1[j];
      w[n - 2][j] -= c[n - 2]*temp2[j];
    }
  
    /* reduce the matrix to upper triangular form. */
  
    for(i = 1;i < n - 2;i++)
    {
      b[i + 1] -= c[i]*a[i + 1]/b[i];
  
      for(j = 0;j < 3;j++)
      {
        bd(e[i],dtheta[i],1,w[i],temp1);
        
        w[i + 1][j] -= temp1[j]*a[i + 1]/b[i];
      }
    }
  
    /* solve using back substitution. */
  
    for(j = 0;j < 3;j++)
      w[n - 2][j] /= b[n - 2];
  
    for(i = n - 3;i > 0;i--)
    {
      bd(e[i],dtheta[i],0,w[i + 1],temp1);
  
      for(j = 0;j < 3;j++)
        w[i][j] = (w[i][j] - c[i]*temp1[j])/b[i];
    }
  
    dw = 0.0;

    for(i = 1;i < n - 1;i++)
      for(j =  0;j < 3;j++)
        dw += (w[i][j] - wprev[i][j])*(w[i][j] - wprev[i][j]);

    dw = sqrt(dw);
  }
  while(iter++ < maxit && dw > tol);

  /* solve for end conditions. */
  
  for(j = 0;j < 3;j++)
  {
    w[0    ][j] = wi[j];
    w[n - 1][j] = wf[j];
  }
}

// Note: This function has been slighlty modified from it's original (definition only).
int bd(
	double e[], double dtheta, int flag, double xin[], double xout[]
)
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

purpose

Subroutine bd performs the transformation between the coefficient vector and
the angular rate vector.

calling sequence

variable     i/o     description
--------     ---     -----------

e             i      unit vector along slew eigen-axis.

dtheta        i      slew angle (rad).

flag          i      flag determining direction of transformation.
                      = 0 -> compute coefficient vector from
                      angular rate vector
                      = 1 -> compute angular rate vector from
                      coefficient vector

xin           i      input vector.

xout          o      output vector.

return value

 0 -> no error
-1 -> transformation direction incorrectly specified.

external references

crossp

programming

J. J. McEnnan, April, 2003.

COPYRIGHT (C) 2003 by James McEnnan

    To the extent possible under law, James McEnnan ( jmcennan@mailaps.org )
    has waived all copyright and related or neighboring rights to qspline CC0
    ( http://sourceforge.net/projects/qspline-cc0 ) under the CC0 1.0
    license ( http://creativecommons.org/publicdomain/zero/1.0/ ).
    This work is published from: United States.

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
{
  int i;
  double sa, ca, b0, b1, b2, temp1[3], temp2[3];

  if(dtheta > AFX_MATH_EPS)
  {
    ca = cos(dtheta);
    sa = sin(dtheta);

    if(flag == 0)
    {
      b1 = 0.5*dtheta*sa/(1.0 - ca);
      b2 = 0.5*dtheta;
    }
    else if(flag == 1)
    {
      b1 = sa/dtheta;
      b2 = (ca - 1.0)/dtheta;
    }
    else
      return -1;

    b0 = xin[0]*e[0] + xin[1]*e[1] + xin[2]*e[2];

    crossp(e,xin,temp2);

    crossp(temp2,e,temp1);

    for(i = 0;i < 3;i++)
      xout[i] = b0*e[i] + b1*temp1[i] + b2*temp2[i];
  }
  else
  {
    for(i = 0;i < 3;i++)
      xout[i] = xin[i];
  }

  return 0;
}

// Note: This function has been slighlty modified from it's original (definition only).
void rf(
	double e[], double dtheta, double win[], double rhs[]
)
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

purpose

Subroutine rf computes the non-linear rate contributions to the final
angular acceleration.

calling sequence

variable     i/o     description
--------     ---     -----------

e             i      unit vector along slew eigen-axis.

dtheta        i      slew angle (rad).

win           i      input final angular rate vector.

rhs           o      output vector containing non-linear rate contributions
                     to the final acceleration.

return value

none

external references

crossp

programming

J. J. McEnnan, May, 2003.

COPYRIGHT (C) 2003 by James McEnnan

    To the extent possible under law, James McEnnan ( jmcennan@mailaps.org )
    has waived all copyright and related or neighboring rights to qspline CC0
    ( http://sourceforge.net/projects/qspline-cc0 ) under the CC0 1.0
    license ( http://creativecommons.org/publicdomain/zero/1.0/ ).
    This work is published from: United States.

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
{
  int i;
  double sa, ca, dot, mag, c1, r0, r1, temp1[3], temp2[3];

  if(dtheta > AFX_MATH_EPS)
  {
    ca = cos(dtheta);
    sa = sin(dtheta);

    crossp(e,win,temp2);

    crossp(temp2,e,temp1);

    dot = win[0]*e[0] + win[1]*e[1] + win[2]*e[2];

    mag = win[0]*win[0] + win[1]*win[1] + win[2]*win[2];

    c1 = (1.0 - ca);

    r0 = 0.5*(mag - dot*dot)*(dtheta - sa)/c1;

    r1 = dot*(dtheta*sa - 2.0*c1)/(dtheta*c1);

    for(i = 0;i < 3;i++)
      rhs[i] = r0*e[i] + r1*temp1[i];
  }
  else
  {
    for(i = 0;i < 3;i++)
      rhs[i] = 0.0;
  }
}

static double a[3][3], b[3][3], c[2][3], d[3];

// Note: This function has been slighlty modified from it's original (definition only).
void slew3_init(
	double dt, double dtheta, double e[], double wi[], double ai[],
	double wf[], double af[]
)
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

purpose

Subroutine slew3_init computes the coefficients for a third-order polynomial
interpolation function describing a slew between the input initial and
final states.

calling sequence

variable     i/o     description
--------     ---     -----------

dt            i      slew time (sec).

dtheta        i      slew angle (rad).

e             i      unit vector along slew eigen-axis.

wi            i      initial body angular rate (rad/sec).

ai            i      initial body angular acceleration (rad/sec^2)
                     (included for compatibility only).

wf            i      final body angular rate (rad/sec).

af            i      final body angular acceleration (rad/sec^2)
                     (included for compatibility only).

return value

none

external references

none

programming

J. J. McEnnan, March, 2003.

COPYRIGHT (C) 2003 by James McEnnan

    To the extent possible under law, James McEnnan ( jmcennan@mailaps.org )
    has waived all copyright and related or neighboring rights to qspline CC0
    ( http://sourceforge.net/projects/qspline-cc0 ) under the CC0 1.0
    license ( http://creativecommons.org/publicdomain/zero/1.0/ ).
    This work is published from: United States.

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
{
  int i;
  double sa, ca, c1, c2;
  double b0, bvec1[3], bvec2[3], bvec[3];

  if(dt <= 0.0)
    return;

  sa = sin(dtheta);
  ca = cos(dtheta);

  /* final angular rate terms. */

  if(dtheta > AFX_MATH_EPS)
  {
    c1 = 0.5*sa*dtheta/(1.0 - ca);

    c2 = 0.5*dtheta;

    b0 = e[0]*wf[0] + e[1]*wf[1] + e[2]*wf[2];

    crossp(e,wf,bvec2);

    crossp(bvec2,e,bvec1);

    for(i = 0;i < 3;i++)
      bvec[i] = b0*e[i] + c1*bvec1[i] + c2*bvec2[i];
  }
  else
  {
    for(i = 0;i < 3;i++)
      bvec[i] = wf[i];
  }

  /* compute coefficients. */

  for(i = 0;i < 3;i++)
  {
    b[0][i] = wi[i];
    a[2][i] = e[i]*dtheta;
    b[2][i] = bvec[i];

    a[0][i] =  b[0][i]*dt;
    a[1][i] = (b[2][i]*dt - 3.0*a[2][i]);

    b[1][i] = (2.0*a[0][i] + 2.0*a[1][i])/dt;
    c[0][i] = (2.0*b[0][i] +     b[1][i])/dt;
    c[1][i] = (    b[1][i] + 2.0*b[2][i])/dt;

       d[i] = (    c[0][i] +     c[1][i])/dt;
  }
}

// Note: This function has been slighlty modified from it's original (definition only).
void slew3(
	double t, double dt, double qi[], double q[],
	double omega[], double alpha[], double jerk[]
)
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

purpose

Subroutine slew3 computes the quaternion, body angular rate, acceleration and
jerk as a function of time corresponding to a third-order polynomial
interpolation function describing a slew between initial and final states.

calling sequence

variable     i/o     description
--------     ---     -----------

t             i      current time (seconds from start).

dt            i      slew time (sec).

qi            i      initial attitude quaternion.

q             o      current attitude quaternion.

omega         o      current body angular rate (rad/sec).

alpha         o      current body angular acceleration (rad/sec^2).

jerk          o      current body angular jerk (rad/sec^3).

return value

none

external references

unvec

programming

J. J. McEnnan, March, 2003.

COPYRIGHT (C) 2003 by James McEnnan

    To the extent possible under law, James McEnnan ( jmcennan@mailaps.org )
    has waived all copyright and related or neighboring rights to qspline CC0
    ( http://sourceforge.net/projects/qspline-cc0 ) under the CC0 1.0
    license ( http://creativecommons.org/publicdomain/zero/1.0/ ).
    This work is published from: United States.

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
{
  int i;
  double x, ang, sa, ca, u[3], x1[2];
  double th0[3], th1[3], th2[3], th3[3], temp0[3], temp1[3], temp2[3];
  double thd1, thd2, thd3, w2, td2, ut2, wwd;
  double w[3], udot[3], wd1[3], wd1xu[3], wd2[3], wd2xu[3];

  if(dt <= 0.0)
    return;

  x = t/dt;

  x1[0] = x - 1.0;

  for(i = 1;i < 2;i++)
    x1[i] = x1[i - 1]*x1[0];

  for(i = 0;i < 3;i++)
  {
    th0[i] = ((x*a[2][i] + x1[0]*a[1][i])*x + x1[1]*a[0][i])*x;

    th1[i] = (x*b[2][i] + x1[0]*b[1][i])*x + x1[1]*b[0][i];

    th2[i] = x*c[1][i] + x1[0]*c[0][i];

    th3[i] = d[i];
  }

  ang = unvec(th0,u);

  ca = cos(0.5*ang);
  sa = sin(0.5*ang);

  q[0] = ca*qi[0] + sa*( u[2]*qi[1] - u[1]*qi[2] + u[0]*qi[3]);
  q[1] = ca*qi[1] + sa*(-u[2]*qi[0] + u[0]*qi[2] + u[1]*qi[3]);
  q[2] = ca*qi[2] + sa*( u[1]*qi[0] - u[0]*qi[1] + u[2]*qi[3]);
  q[3] = ca*qi[3] + sa*(-u[0]*qi[0] - u[1]*qi[1] - u[2]*qi[2]);

  ca = cos(ang);
  sa = sin(ang);

  if(ang > AFX_MATH_EPS)
  {
    /* compute angular rate vector. */

    crossp(u,th1,temp1);

    for(i = 0;i < 3;i++)
      w[i] = temp1[i]/ang;

    crossp(w,u,udot);

    thd1 = u[0]*th1[0] + u[1]*th1[1] + u[2]*th1[2];

    for(i = 0;i < 3;i++)
      omega[i] = thd1*u[i] + sa*udot[i] - (1.0 - ca)*w[i];

    /* compute angular acceleration vector. */

    thd2 = udot[0]*th1[0] + udot[1]*th1[1] + udot[2]*th1[2] +
              u[0]*th2[0] +    u[1]*th2[1] +    u[2]*th2[2];

    crossp(u,th2,temp1);

    for(i = 0;i < 3;i++)
      wd1[i] = (temp1[i] - 2.0*thd1*w[i])/ang;

    crossp(wd1,u,wd1xu);

    for(i = 0;i < 3;i++)
      temp0[i] = thd1*u[i] - w[i];

    crossp(omega,temp0,temp1);

    for(i = 0;i < 3;i++)
      alpha[i] = thd2*u[i] + sa*wd1xu[i] - (1.0 - ca)*wd1[i] +
      thd1*udot[i] + temp1[i];

    /* compute angular jerk vector. */

    w2 = w[0]*w[0] + w[1]*w[1] + w[2]*w[2];

    thd3 = wd1xu[0]*th1[0] + wd1xu[1]*th1[1] + wd1xu[2]*th1[2] -
           w2*(u[0]*th1[0] + u[1]*th1[1] + u[2]*th1[2]) +
           2.0*(udot[0]*th2[0] + udot[1]*th2[1] + udot[2]*th2[2]) +
           u[0]*th3[0] + u[1]*th3[1] + u[2]*th3[2];

    crossp(th1,th2,temp1);

    for(i = 0;i < 3;i++)
      temp1[i] /= ang;

    crossp(u,th3,temp2);

    td2 = (th1[0]*th1[0] + th1[1]*th1[1] + th1[2]*th1[2])/ang;

    ut2 = u[0]*th2[0] + u[1]*th2[1] + u[2]*th2[2];

    wwd = w[0]*wd1[0] + w[1]*wd1[1] + w[2]*wd1[2];

    for(i = 0;i < 3;i++)
      wd2[i] = (temp1[i] + temp2[i] - 2.0*(td2 + ut2)*w[i] -
      4.0*thd1*wd1[i])/ang;

    crossp(wd2,u,wd2xu);

    for(i = 0;i < 3;i++)
      temp2[i] = thd2*u[i] + thd1*udot[i] - wd1[i];

    crossp(omega,temp2,temp1);

    crossp(alpha,temp0,temp2);

    for(i = 0;i < 3;i++)
      jerk[i] = thd3*u[i] + sa*wd2xu[i] - (1.0 - ca)*wd2[i] +
      2.0*thd2*udot[i] + thd1*((1.0 + ca)*wd1xu[i] - w2*u[i] - sa*wd1[i]) -
      wwd*sa*u[i] + temp1[i] + temp2[i];
  }
  else
  {
    crossp(th1,th2,temp1);

    for(i = 0;i < 3;i++)
    {
      omega[i] = th1[i];
      alpha[i] = th2[i];
       jerk[i] = th3[i] - 0.5*temp1[i];
    }
  }
}

// Note: This function has been slighlty modified from it's original (definition only).
double unvec(
	double a[], double au[]
)
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

purpose

subroutine unvec unitizes a vector and computes its magnitude.

calling sequence

variable     i/o     description
--------     ---     -----------

a             i      input vector.

au            o      output unit vector.

return value

magnitude of vector a.

external references

none

programming

J. J. McEnnan, December, 1987.

COPYRIGHT (C) 2003 by James McEnnan

    To the extent possible under law, James McEnnan ( jmcennan@mailaps.org )
    has waived all copyright and related or neighboring rights to qspline CC0
    ( http://sourceforge.net/projects/qspline-cc0 ) under the CC0 1.0
    license ( http://creativecommons.org/publicdomain/zero/1.0/ ).
    This work is published from: United States.

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
{
  double amag;

  amag = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);

  if(amag > 0.0)
  {
    au[0] = a[0]/amag;
    au[1] = a[1]/amag;
    au[2] = a[2]/amag;
  }
  else
  {
    au[0] = 0.0;
    au[1] = 0.0;
    au[2] = 0.0;
  }

  return amag;
}

// Note: This function has been slighlty modified from it's original (definition only).
void crossp(
	double b[], double c[], double a[]
)
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

purpose

subroutine crossp computes the vector cross product b x c.

calling sequence

variable     i/o     description
--------     ---     -----------

b             i      input vector.

c             i      input vector.

a             o      output vector = b x c.

return value

none

external references

none

programming

J. J. McEnnan, February, 1988.

COPYRIGHT (C) 2003 by James McEnnan

    To the extent possible under law, James McEnnan ( jmcennan@mailaps.org )
    has waived all copyright and related or neighboring rights to qspline CC0
    ( http://sourceforge.net/projects/qspline-cc0 ) under the CC0 1.0
    license ( http://creativecommons.org/publicdomain/zero/1.0/ ).
    This work is published from: United States.

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
{
  a[0] = b[1]*c[2] - b[2]*c[1];
  a[1] = b[2]*c[0] - b[0]*c[2];
  a[2] = b[0]*c[1] - b[1]*c[0];
}

// QEulerAngles ////////////////////////////////////////////////////////////////

QEulerAngles::QEulerAngles(double pitch, double yaw, double roll)
{
    Pitch = pitch;
    Yaw = yaw;
    Roll = roll;
}

// QREulerAngles ///////////////////////////////////////////////////////////////

QREulerAngles::QREulerAngles(double pitch, double yaw, double roll)
{
    Pitch = pitch;
    Yaw = yaw;
    Roll = roll;
}

QREulerAngles QREulerAngles::FromQEulerAngles(QEulerAngles a)
{
	return QREulerAngles(
		M_PI * a.Pitch / 180.0,
		M_PI * a.Yaw / 180.0,
		M_PI * a.Roll / 180.0
	);
}

QEulerAngles QREulerAngles::ToQEulerAngles(void)
{
	return QEulerAngles(
		180.0 * Pitch / M_PI,
		180.0 * Yaw / M_PI,
		180.0 * Roll / M_PI
	);
}

// Quaternion //////////////////////////////////////////////////////////////////

Quaternion operator +(Quaternion a, Quaternion b)
{
    return Quaternion(
        a.W + b.W,
        a.X + b.X,
        a.Y + b.Y,
        a.Z + b.Z
    );
}

Quaternion operator *(double a, Quaternion b)
{
    return Quaternion(
        a*b.W,
        a*b.X,
        a*b.Y,
        a*b.Z
    );
}

Quaternion operator *(Quaternion a, Quaternion b)
{
    return Quaternion(
        a.W*b.W - a.X*b.X - a.Y*b.Y - a.Z*b.Z,
        a.W*b.X + a.X*b.W + a.Y*b.Z - a.Z*b.Y,
        a.W*b.Y - a.X*b.Z + a.Y*b.W + a.Z*b.X,
        a.W*b.Z + a.X*b.Y - a.Y*b.X + a.Z*b.W
    );
}

double DotProduct(Quaternion a, Quaternion b)
{
	return a.W*b.W + a.X*b.X +a.Y*b.Y +a.Z*b.Z;
}

Quaternion Quaternion::FromQREulerAngles(QREulerAngles a)
{
	// todo: this can be optimized (since many components are 0),
	// but there was a bug in it, so let's do it inefficiently for now:

	double pitchH = 0.5 * a.Pitch;
	Quaternion qPitchY(cos(pitchH), 0.0, sin(pitchH), 0.0);
	 
	double yawH = 0.5 * a.Yaw;
	Quaternion qYawZ(cos(yawH), 0.0, 0.0, sin(yawH));
	 
	double rollH = 0.5 * a.Roll;
	Quaternion qRollX(cos(rollH), sin(rollH), 0.0, 0.0);
	 
	return qYawZ * qPitchY * qRollX;
}

Quaternion::Quaternion()
{
	W = 1.0;
	X = 0.0;
	Y = 0.0;
	Z = 0.0;
}

Quaternion::Quaternion(double w, double x, double y, double z)
{
	W = w;
	X = x;
	Y = y;
	Z = z;
}

double Quaternion::Norm() const
{
    return sqrt(W*W +X*X +Y*Y +Z*Z);
}

Quaternion Quaternion::Normalized() const
{
	double norm = this->Norm();

	return Quaternion(W / norm, X / norm, Y / norm, Z / norm);
}

Quaternion Quaternion::Conjugate() const
{
	return Quaternion(W, -X, -Y, -Z);
}

double Quaternion::GetAng(Quaternion const & y, Vector3 & outEigenAxis) const
{
	double qi[4] = {this->X, this->Y, this->Z, this->W};
	double qf[4] = {y.X, y.Y, y.Z, y.W};

	double axis[3];
	double angle = getang(qi, qf, axis);

	outEigenAxis = Vector3(axis[0], axis[1], axis[2]);
	
	return angle;
}

Quaternion Quaternion::Slerp(Quaternion const & y, double t) const
{
	Vector3 eigenAxis;

	double dtheta = GetAng(y, eigenAxis);

	double tDthetaDiv2 = t * dtheta / 2;
		
	double cosTDheta = cos(tDthetaDiv2);
	double sinTDHeta = sin(tDthetaDiv2);

	Quaternion dq_pow_t = Quaternion(cosTDheta, eigenAxis.X*sinTDHeta, eigenAxis.Y*sinTDHeta,eigenAxis.Z*sinTDHeta);

	return (*this) * dq_pow_t;
}

QREulerAngles Quaternion::ToQREulerAngles() const
{
	// TODO: There might still be a problem with singualrities in here!

	// Quaternion to matrix conversion taken from:
	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm

	// Quaternion to euler conversion analog (but changed) to:
	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm

	double sqw = W*W;
	double sqx = X*X;
	double sqy = Y*Y;
	double sqz = Z*Z;

	double ssq = sqx + sqy + sqz + sqw;
	double invs = ssq ? 1 / ssq : 0;
	double m00 = ( sqx - sqy - sqz + sqw)*invs;
	//double m11 = (-sqx + sqy - sqz + sqw)*invs;
	double m22 = (-sqx - sqy + sqz + sqw)*invs;
    
	double tmp1 = X*Y;
	double tmp2 = Z*W;
	double m10 = 2.0 * (tmp1 + tmp2)*invs;
	//double m01 = 2.0 * (tmp1 - tmp2)*invs;
    
	tmp1 = X*Z;
	tmp2 = Y*W;
	double m20 = 2.0 * (tmp1 - tmp2)*invs;
	//double m02 = 2.0 * (tmp1 + tmp2)*invs;

	tmp1 = Y*Z;
	tmp2 = X*W;
	double m21 = 2.0 * (tmp1 + tmp2)*invs;
	//double m12 = 2.0 * (tmp1 - tmp2)*invs;

	// X =            Y =            Z =
	// |1, 0 , 0  | |cp , 0, sp| |cy, -sy, 0|
	// |0, cr, -sr| |0  , 1, 0 | |sy, cy , 0|
	// |0, sr, cr | |-sp, 0, cp| |0 , 0  , 1|

	// Y*X =
	// |cp , sp*sr, sp*cr|
	// |0  , cr   , -sr  |
	// |-sp, cp*sr, cp*cr|

	// Z*(Y*X) =
	// |cy*cp, cy*sp*sr -sy*cr, cy*sp*cr +sy*sr |
	// |sy*cp, sy*sp*sr +cy*cr, sy*sp*cr +cy*-sr|
	// |-sp  , cp*sr          , cp*cr           |

	// 1) cy*cp = m00
	// 2) cy*sp*sr -sy*cr = m01
	// 3) cy*sp*cr +sy*sr = m02
	// 4) sy*cp = m10
	// 5) sy*sp*sr +cy*cr = m11
	// 6) sy*sp*cr +cy*-sr = m12
	// 7) -sp = m20
	// 8) cp*sr = m21
	// 9) cp*cr = m22
	//
	// 7=> p = arcsin( -m20 )
	//
	// 4/1=> y = arctan2( m10, m00 )
	//
	// 8/9=> r = arctan2( m21, m22 )
	
	double sinYPitch = -m20;
	double yPitch;
	double zYaw;
	double xRoll;

	if(sinYPitch > 1.0 -AFX_MATH_EPS)
	{
		// sout pole singularity:

		yPitch = M_PI / 2.0;

		xRoll = -2.0*atan2(Z*invs,W*invs);
		zYaw = 0;
	}
	else
	if(sinYPitch < -1.0 +AFX_MATH_EPS)
	{
		// north pole singularity:

		yPitch = -M_PI / 2.0;
		xRoll = 2.0*atan2(Z*invs,W*invs);
		zYaw = 0;
	}
	else
	{
		// hopefully no singularity:

		yPitch = asin( sinYPitch );
		zYaw = atan2( m10, m00 );
		xRoll = atan2( m21, m22 );
	}

    return QREulerAngles(
        yPitch,
        zYaw,
        xRoll
    );
}

// Vector3 /////////////////////////////////////////////////////////////////////

Vector3 operator * (double value, Vector3 x)
{
	return Vector3(value * x.X, value * x.Y, value * x.Z);
}

Vector3::Vector3()
{
}

Vector3::Vector3(double x, double y, double z)
: X(x)
, Y(y)
, Z(z)
{
}

Vector3::Vector3(double value[3])
: X(value[0])
, Y(value[1])
, Z(value[2])
{
}

Vector3::Vector3(const Vector3 & v)
: X(v.X)
, Y(v.Y)
, Z(v.Z)
{
}

Vector3 Vector3::operator + (const Vector3 & y) const
{
	return Vector3(X +y.X, Y +y.Y, Z +y.Z);
}
	
void Vector3::operator += (const Vector3 & y)
{
	X += y.X;
	Y += y.Y;
	Z += y.Z;
}

Vector3 Vector3::operator - (const Vector3 & y) const
{
	return Vector3(X -y.X, Y -y.Y, Z -y.Z);
}

void Vector3::operator -= (const Vector3 & y)
{
	X -= y.X;
	Y -= y.Y;
	Z -= y.Z;
}

Vector3 Vector3::operator * (double value) const
{
	return Vector3(X * value, Y * value, Z * value);
}

void Vector3::operator *= (double value)
{
	X *= value;
	Y *= value;
	Z *= value;
}

Vector3 Vector3::operator / (double value) const
{
	return Vector3(X / value, Y / value, Z / value);
}

void Vector3::operator /= (double value)
{
	X /= value;
	Y /= value;
	Z /= value;
}

void Vector3::ToArray(double (& outValue)[3]) const
{
	outValue[0] = X;
	outValue[1] = Y;
	outValue[2] = Z;
}

double Vector3::Length() const
{
	return sqrt(X*X + Y*Y + Z*Z);
}

Vector3 Vector3::Normalize()
{
	double length = Length();

	if(length) *this = *this / length;

	return *this;
}

double AngleModDeg(double x)
{
	x = fmod(x + 180, 360.0);
	if (x < 0)
		x += 360;
	return x - 180;
}

} // namespace Afx {
} // namespace Math
