
/** a number of 'common' functions for navigation

\file xmnavtools.cpp
\date    2012 - 2015
\version 2.0

*/


#include "xmnavtools.h"


//  TODO: fix it 
Qua	orientationFromAccelerations(const Vector3& mv) {
	Vector3  bf_z;   //  direction of rotation 
	Vector3	gn(mv);   // "g" in imu frame
	Vector3  rv1;
	Qua		rq, ret;
	XMType f, cos_f;

	//  1.  normalization of g(bf)  vector:
	gn.normalize();
	//  2. angle of rotation:
	cos_f = gn(2);

	if(fabs(cos_f) > 0.999999997) {
		Qua ret1;
		return ret1;   //   do not know azimut!
	} else {
		f = acos(cos_f);
	}

	//  3.   direction of rotation:
	bf_z.clear();
	bf_z[2] = ONE;
	rv1 = cross(gn, bf_z); // 

	//  3.1  rv1 normalization:
	rv1.dmul(f / rv1.length());

	//  4.    rotation!
	rq.from_rv(rv1);
	ret = ret.qprod(rq); // !
	ret.normalize();

#if defined _DEBUG
#ifdef PC__VERSION
	{
		Vector3 g;
		g[0] = ZERO;   g[1] = ZERO;    g[2] = -9.815;
		DCM dcm = !(ret.dcm());
		Vector3 a_in_ENU = dcm * mv;
		Vector3 a_in_ENU_1 = dcm * mv + g;

		double f_deg = f * 180. / 3.14159;
		int i53453 = 0;
	}
#endif
#endif

	return ret;
}


