// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

//! Estimator for a Fit of Dalitz-Plot Slices.
/*! \class SliceFit
 * @file SliceFit.hpp
 * This class performs a Chi2-Fit on slices along one axis of a dalitz-plot.
 * The Dalitz-Plot is generated directly in the constructor of this Estimator.
 * Data and Model are provided in the constructor using the Amplitude and Data
 * interfaces. The class itself fulfills the Estimator interface.
*/

#ifndef _SLICEFIT_HPP
#define _SLICEFIT_HPP

#include <vector>
#include <memory>
#include <string>

//Root Header
#include "TH2D.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TF1.h"

//PWA-Header
#include "Estimator/Estimator.hpp"
#include "Core/Amplitude.hpp"
#include "Physics/AmplitudeSum/AmpSumIntensity.hpp"
#include "DataReader/Data.hpp"
#include "Core/Event.hpp"
#include "Core/ParameterList.hpp"

/*class  FitFuncObject {
 public:
   // use constructor to customize your function object
   FitFuncObject(std::shared_ptr<AmpSumIntensity> pPIF, ParameterList& par)
     :pPIF_(pPIF),par_(par){

   }

   double operator() (double *x, double *p) {
     //double m12sq=M*M-x[0]-par[0];
    // if(m12sq<0)
     //  m12sq=0.0001;

     std::vector<double> point;
     point.push_back(p[0]); point.push_back(x[0]); //point.push_back(m12sq);

     std::complex<double> reso[3];
     reso[2]=std::complex<double>(p[6],0.);
     reso[0]=std::complex<double>(p[2],p[3]);
     reso[1]=std::complex<double>(p[4],p[5]);
     double result = p[1]*pPIF_->sliceIntensity(point, par_,reso, 3);
     //double result = par[1]*totAmp23.evaluate();

     if(result!=result) return 0;
     return result;
     //return totAmp23.evaluate();
   }

 private:
   ParameterList& par_; //original parameter

   std::shared_ptr<AmpSumIntensity> pPIF_;

};*/

namespace ComPWA {
namespace Estimator {
namespace SliceFit {

class SliceFit : public Estimator {

public:
  /// Default Constructor (0x0)
  //SliceFit(std::shared_ptr<Amplitude>, std::shared_ptr<Data>);

  virtual double controlParameter(ParameterList& minPar);
  static std::shared_ptr<ControlParameter> createInstance(std::shared_ptr<Physics::AmplitudeSum::AmpSumIntensity>, std::shared_ptr<DataReader::Data>, ParameterList& inPar, unsigned int startEvent=0, unsigned int nEvents=0, unsigned int nBins=200, unsigned int nF0=3, unsigned int nF2=2);
  static std::shared_ptr<ControlParameter> createInstance(std::shared_ptr<Physics::AmplitudeSum::AmpSumIntensity>, std::shared_ptr<DataReader::Data>, std::shared_ptr<DataReader::Data>, ParameterList& inPar, unsigned int startEvent=0, unsigned int nEvents=0, unsigned int nBins=200, unsigned int nF0=3, unsigned int nF2=2);

  double setSlice(unsigned int i) {
    if(i<nBins_){
      whichSlice_=i;
      return dalitzPlot_->GetBinCenter(whichSlice_);
    }
    return 0;
  };

  std::shared_ptr<TH1D> getSliceHist() { return aSlice_;}
  std::shared_ptr<TH1D> getAmpSlHist() { return theAmpSl_;}
  std::shared_ptr<TH1D> getAmpClHist() { return theAmpCl_;}

  /** Destructor */
  virtual ~SliceFit();

protected:
  /// Default Constructor (0x0)
  SliceFit(std::shared_ptr<Physics::AmplitudeSum::AmpSumIntensity>, std::shared_ptr<DataReader::Data>, ParameterList&, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
  SliceFit(std::shared_ptr<Physics::AmplitudeSum::AmpSumIntensity>, std::shared_ptr<DataReader::Data>, std::shared_ptr<DataReader::Data>, ParameterList&, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);

  void init();

  double lambda(double x, double y, double z){
    return x*x+y*y+z*z-2.*x*y-2.*x*z-2.*y*z;
  };

  double m13_sq_max_constr(double m23_sq){
    return m1*m1+m3*m3+0.5/m23_sq*((M*M-m23_sq-m1*m1)*(m23_sq-m2*m2+m3*m3)+std::sqrt(lambda(m23_sq,M*M,m1*m1))*std::sqrt(lambda(m23_sq,m2*m2,m3*m3)));
  };

  double m13_sq_min_constr(double m23_sq){
    return m1*m1+m3*m3+0.5/m23_sq*((M*M-m23_sq-m1*m1)*(m23_sq-m2*m2+m3*m3)-std::sqrt(lambda(m23_sq,M*M,m1*m1))*std::sqrt(lambda(m23_sq,m2*m2,m3*m3)));
  };

private:
  static double fitsliceAMP(Double_t*, Double_t*);

  TH2D* dalitzPlot_;
  std::shared_ptr<TH1D> aSlice_;
  std::shared_ptr<TH1D> theAmpSl_;
  std::shared_ptr<TH1D> theAmpCl_;
  ParameterList& par_;
  //FitFuncObject func_;

  std::shared_ptr<Physics::AmplitudeSum::AmpSumIntensity> pPIF_;
  std::shared_ptr<DataReader::Data> pDIF_;
  std::shared_ptr<DataReader::Data> pPHSP_;
  double phspVolume;
  unsigned int nEvts_;
  unsigned int nPhsp_;
  unsigned int nStartEvt_;
  unsigned int nUseEvt_;

  unsigned int nBins_;
  unsigned int whichSlice_;
  unsigned int nF0_;
  unsigned int nF2_;

  double M; // GeV/c² (J/psi+)
  double Br; // GeV/c² (width)
  double m1; // GeV/c² (gamma)
  double m2; // GeV/c² (pi)
  double m3; // GeV/c² (pi)
  double PI; // m/s

};

} /* namespace SliceFit */
} /* namespace Estimator */
} /* namespace ComPWA */

#endif /* _SLICEFIT_HPP */
