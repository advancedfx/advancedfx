// RoundingErrors.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <tchar.h>
#include <shared/AfxMath.h>

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	// abusing this project ....
/*
	int frames = 72510;
	float fps = 60.0f;

	float fTime = 0.0f;
	double dTime = 0.0;

	for(int i=0;i<frames;i++)
	{
		float fDuration = ((i+1)/fps)-(i/fps);
		double dDuration = (1.0/(double)fps);

		fTime += fDuration;
		dTime += dDuration;

		cout << i << ": " << fTime << "/" << dTime << endl;
	}
*/

	/*
	double Matrix[4][4] = {
		1, 2, 3, 0,
		1, 1, 1, 0,
		3, 3, 1, 0,
		0, 0, 0, 1
	};
	double b[4] = {1, 1, 1, 1};
	*/

	double Matrix[4][4] = {
		0.569779, -0.487701, -0.000328, 0.000000,
		0.034150, 0.039003, 1.332325, 0.000000,
		0.649924, 0.759329, -0.038888, 0.000000,
		0.649764, 0.759142, -0.038878, -1.000000
	};
	double b[4] = {867.811279, -234.318237, -1869.297119, -1876.835693};

	unsigned char p[4];
	unsigned char q[4];
	double L[4][4];
	double R[4][4];

	bool bOk = Afx::Math::LUdecomposition(Matrix, p, q, L, R);

	cout << bOk << endl;

	for(int i=0;i<4;i++)
		for(int k=0;k<4;k++)
		{
			cout << "L[" << i << "," << k << "]=" << L[i][k] << " R[,]=" << R[i][k] << std::endl;
		};

	double x[4];

	Afx::Math::SolveWithLU(L, R,p,q,b,x);
	for(int k=0;k<4;k++)
	{
		cout << "x[" << k << "]=" << x[k] << std::endl;
	};

	for(int i=0;i<4;i++)
	{
		double sum = 0;
		for(int k=0;k<4;k++)
			sum += x[k] * Matrix[i][k];

		cout << sum << std::endl;
	}


	unsigned char dummy;

	cin >> dummy;

	return 0;
}

