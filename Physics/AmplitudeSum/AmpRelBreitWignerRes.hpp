//-------------------------------------------------------------------------------
// Copyright (c) 2013 Mathias Michel.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
// Contributors:
//     Mathias Michel - initial API and implementation
//		Peter Weidenkaff - correct nominator, using dataPoint for data handling
//-------------------------------------------------------------------------------
//****************************************************************************
// Class for defining the relativistic Breit-Wigner resonance model, which
// includes the use of Blatt-Weisskopf barrier factors.
//****************************************************************************

// --CLASS DESCRIPTION [MODEL] --
// Class for defining the relativistic Breit-Wigner resonance model, which
// includes the use of Blatt-Weisskopf barrier factors.

#ifndef AMP_REL_BREIT_WIGNER_RES
#define AMP_REL_BREIT_WIGNER_RES

#include <vector>

//#include "TObject.h"
//#include "TString.h"
#include "Physics/AmplitudeSum/AmpAbsDynamicalFunction.hpp"
#include "Physics/AmplitudeSum/AmpKinematics.hpp"
#include "Physics/AmplitudeSum/AmpWigner2.hpp"
#include "Core/Functions.hpp"
#include "Core/Exceptions.hpp"

class BreitWignerStrategy : public Strategy {
public:
  BreitWignerStrategy(const std::string resonanceName):name(resonanceName){
    //name = +resonanceName;
  }

  virtual const std::string to_str() const {
    return ("relativistic BreitWigner of "+name);
  }

  virtual std::shared_ptr<AbsParameter> execute(ParameterList& paras) {

    double Gamma0, GammaV, m0, m, ma, mb, d;
    unsigned int spin;
    try{
      m0 = double(paras.GetParameterValue("m0_"+name));
    }catch(BadParameter& e){
      m0 = double(paras.GetParameterValue("ParOfNode_m0_"+name));
    }

    /*std::cout << "N Paras: " << paras.GetNParameter() << std::endl;
    for(unsigned int par=0; par<paras.GetNParameter(); par++ ){
      std::shared_ptr<AbsParameter> tmp = paras.GetParameter(par);
      std::cout << "Par " << par << " : " << tmp->GetName() << std::endl;
    }*/

    m  = double(paras.GetParameterValue("x"));
    ma = double(paras.GetParameterValue("m23"));
    mb = double(paras.GetParameterValue("m13"));
    spin = (unsigned int)(paras.GetParameterValue("ParOfNode_spin_"+name));
    d = double(paras.GetParameterValue("ParOfNode_d_"+name));

    try{
      Gamma0 = double(paras.GetParameterValue("resWidth_"+name));
    }catch(BadParameter& e){
      Gamma0 = double(paras.GetParameterValue("ParOfNode_resWidth_"+name));
    }
    GammaV = Gamma0 * (m0 / m) * pow(q(ma,mb,m) / q0(ma,mb,m0), 2.*spin + 1.)  * BLprime2(ma,mb,m0,m,d,spin);

    std::complex<double> denom(m0*m0 - m*m, -m0 * GammaV);
    std::complex<double> res(m0 * Gamma0);
    res = res / denom;

    //std::complex<double> result (res.re(),res.im());
    std::shared_ptr<ComplexParameter> bw(new ComplexParameter("relBW of "+name, res));
    return bw;
  }

protected:
  std::string name;

  double q0(const double& ma, const double& mb, const double& m0) const {
    double mapb = ma + mb;
    double mamb = ma - mb;

    return sqrt ( (m0*m0 - mapb*mapb) * (m0*m0 - mamb*mamb) ) / (2. * m0 );
  }

  double q(const double& ma, const double& mb, const double& x) const {
    double mapb = ma + mb;
    double mamb = ma - mb;

    return sqrt ( (x*x - mapb*mapb) * (x*x - mamb*mamb) ) / (2. * x );
  }


  // compute part of the Blatt-Weisskopf barrier factor
  //   BLprime = sqrt (F(q0)/F(q))
  double F(const double& p, const double& d, unsigned int& spin) const {
    double retVal = 1;

    if (spin == 0)
      retVal = 1;
    else if (spin == 1)
      retVal = 1 + p*p * d*d;
    else if (spin == 2) {
      double z = p*p * d*d;
      retVal = (z-3.)*(z-3.) + 9*z;
    }
    return retVal;
  }


  // compute square of Blatt-Weisskopf barrier factor
  double BLprime2(const double& ma, const double& mb, const double& m0, const double& x, const double& d, unsigned int& spin) const {
    //  cout << q0() << " " << q() << "\t" << F(q0()) << " " << F(q()) << endl;
    return F(q0(ma, mb, m0),d,spin) / F(q(ma, mb, x),d,spin);
  }

};

class AmpRelBreitWignerRes : public AmpAbsDynamicalFunction, public AmpKinematics {
public:

	AmpRelBreitWignerRes(const char *name,
			DoubleParameter& _resMass, DoubleParameter& _resWidth,
			double& _radius,
			int _subsys,
			int resSpin, int m, int n
	) ;
	AmpRelBreitWignerRes(const AmpRelBreitWignerRes&, const char*);
	AmpRelBreitWignerRes(const AmpRelBreitWignerRes&);

	virtual ~AmpRelBreitWignerRes();

	//  double operator() (double *x, size_t dim, void*);

	virtual void initialise();
	virtual std::complex<double> evaluate() const ;
	virtual std::complex<double> evaluateAmp() const;
	//virtual std::complex<double> evaluateTree(const ParameterList& paras) const;
	virtual double evaluateWignerD() const { return _wignerD.evaluate(); };
	//  virtual double eval(double x[],size_t dim, void *param) const;//used for MC integration
	//  double (*eval2)(double x[],size_t dim, void *param);//used for MC integration

	void setDecayMasses(double, double, double, double);
	//  double getMaximum() const{return 1;};
	double getSpin() {return _spin;}; //needs to be declared in AmpAbsDynamicalFunction
	inline virtual bool isSubSys(const unsigned int subSys) const{ return (subSys==_subSys); };

protected:
	DoubleParameter _resWidth;
//	AmpWigner _wignerD;
	AmpWigner2 _wignerD;

private:
	//ClassDef(AmpRelBreitWignerRes,1) // Relativistic Breit-Wigner resonance model

};

#endif
