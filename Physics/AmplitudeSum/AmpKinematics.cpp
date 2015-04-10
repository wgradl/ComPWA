//-------------------------------------------------------------------------------
// Copyright (c) 2013 Peter Weidenkaff.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
// Contributors:
//		Peter Weidenkaff - initial API
//-------------------------------------------------------------------------------

#include "Physics/AmplitudeSum/AmpKinematics.hpp"
#include "Physics/DPKinematics/DalitzKinematics.hpp"

AmpKinematics::AmpKinematics(std::shared_ptr<DoubleParameter> mR, int subSys, int spin, int m, int n,
		std::shared_ptr<DoubleParameter> mesonRadius, std::shared_ptr<DoubleParameter> motherRadius) :
				_M(-999), _mR(mR),_subSys(subSys), _spin(spin),_m(m),_n(n),
				_mesonRadius(mesonRadius), _motherRadius(motherRadius), _wignerD(subSys,spin)
{
	DalitzKinematics* kin = dynamic_cast<DalitzKinematics*>(Kinematics::instance());
	_M=kin->M;
	if(_subSys==5){
		_ma=kin->m3;
		_mb=kin->m2;
		_mc=kin->m1;}
	if(_subSys==4){
		_ma=kin->m3;
		_mb=kin->m1;
		_mc=kin->m2;}
	if(_subSys==3){
		_ma=kin->m2;
		_mb=kin->m1;
		_mc=kin->m3;}
}

std::complex<double> AmpKinematics::qValue(double sqrtS, double ma, double mb){
	double mapb = ma + mb;
	double mamb = ma - mb;
	double xSq = sqrtS*sqrtS;
	double t1 = xSq - mapb*mapb;
	double t2 = xSq - mamb*mamb;
	std::complex<double> s( sqrt(std::complex<double>(t1*t2,0)) / (2*sqrtS) );
//	if(s.imag())
//		std::cout<<"qValue: "<<t1*t2<<" "<<2*sqrtS<<" "<<s<<std::endl;
	return s;
}

double AmpKinematics::FormFactor(double sqrtS, double mR, double ma, double mb, double spin, double mesonRadius){
	//Blatt-Weisskopt form factors with normalization F(x=mR) = 1.
	//Reference: S.U.Chung Annalen der Physik 4(1995) 404-430
	if (spin == 0) return 1;
	std::complex<double> q = qValue(sqrtS,ma,mb);
	//z = q / (interaction range). For the interaction range we assume 1/mesonRadius
	double z = std::norm(q)*mesonRadius*mesonRadius;

	double nom=0, denom=0;
	if (spin == 1){
		return( sqrt(2*z/(z+1)) );
	}
	else if (spin == 2) {
		return ( sqrt( 13*z*z/( (z-3)*(z-3)+9*z ) ) );
	}
	else if (spin == 3) {
		return ( sqrt( 277*z*z*z/( z*(z-15)*(z-15) + 9*(2*z-5) ) ) );
	}
	else if (spin == 4) {
		return ( sqrt( 12746*z*z*z*z/( (z*z-45*z+105)*(z*z-45*z+105) + 25*z*(2*z-21)*(2*z-21) ) ) );
	}
	else{
		std::cout<<"Wrong spin value! BLW factors only implemented for spin 0-4! "<<std::endl;
	}
	return 0;
}
double AmpKinematics::phspFactor(double sqrtS, double ma, double mb){
	std::complex<double> phsp = qValue(sqrtS,ma,mb) / (8*M_PI*sqrtS);
	if(phsp.imag())
		return 0;
	return phsp.real();
}

double AmpKinematics::widthToCoupling(double mSq, double mR, double width,
		double ma, double mb, double spin, double mesonRadius){
	double sqrtS = sqrt(mSq);

	//calculate gammaA(s_R)
	double ffR = FormFactor(mR,mR,ma,mb,spin,mesonRadius);
	std::complex<double> qR = std::pow(qValue(mR,ma,mb),spin);
	//calculate phsp factor
	double rho = phspFactor(sqrtS,ma,mb);
	std::complex<double> denom = (std::pow(qR,spin)*ffR*sqrt(rho));
	std::complex<double> result = std::complex<double>(sqrt(mR*width), 0) / denom;
	if(result.imag()) //is a complex coupling a physical solution?
		throw std::runtime_error("AmpKinematics::widthToCoupling| coupling has an imaginary part of"
				+std::to_string((long double) result.imag())
	+" sqrtS="+std::to_string((long double) sqrtS)
	+" mR="+std::to_string((long double) mR)
	+" width="+std::to_string((long double) width)
	+" ma="+std::to_string((long double) ma)
	+" mb="+std::to_string((long double) mb));
	return result.real();
}

double AmpKinematics::couplingToWidth(double mSq, double mR, double g,
		double ma, double mb, double spin, double mesonRadius){
	double sqrtM = sqrt(mSq);
	//calculate gammaA(s_R)
	double ffR = FormFactor(mR,mR,ma,mb,spin,mesonRadius);
	std::complex<double> qR = std::pow(qValue(mR,ma,mb),spin);
	std::complex<double> gammaA = ffR*qR;
	//calculate phsp factor
	double rho = phspFactor(sqrtM,ma,mb);
	return ( std::norm(gammaA)*g*g*rho / mR );
}
