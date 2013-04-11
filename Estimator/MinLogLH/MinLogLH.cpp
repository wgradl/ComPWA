#include <sstream>
#include <iostream>
#include <memory>
#include <vector>
#include <cmath>

#include "Estimator/MinLogLH/MinLogLH.hpp"
#include "Core/Event.hpp"
#include "Core/Particle.hpp"
#include "Core/ParameterList.hpp"

MinLogLH::MinLogLH(std::shared_ptr<Amplitude> inPIF, std::shared_ptr<Data> inDIF) : pPIF_(inPIF), pDIF_(inDIF){

}

MinLogLH::~MinLogLH(){

}

double MinLogLH::controlParameter(ParameterList& minPar){
  unsigned int nEvents = pDIF_->getNEvents();

  double norm = pPIF_->integral(minPar);

  /*std::cout << std::endl << "ControlPar LH: " << std::endl;
  std::cout << "Events: " << nEvents << std::endl;
  for(unsigned int i=0; i<minPar.GetNDouble(); i++){
    std::cout << minPar.GetParameterValue(i) << " ";
  }
  std::cout << std::endl;*/

  double lh=0; //calculate LH:
  for(unsigned int evt = 0; evt < nEvents; evt++){
    Event theEvent(pDIF_->getEvent(evt));

   /* TODO: try read exceptions
    if( !(pDIF_->getEvent(evt, theEvent)) ){
      std::cout << "EIFChiOneD::controlParameter: Event not readable!" << std::endl; //TODO Exception
      continue;
    }*/
    /*if( !(theEvent.getParticle(0,a)) ){
      std::cout << "EIFChiOneD::controlParameter: Particle A not readable!" << std::endl; //TODO Exception
      continue;
    }
    if( !(theEvent.getParticle(1,b)) ){
      std::cout << "EIFChiOneD::controlParameter: Particle B not readable!" << std::endl; //TODO Exception
      continue;
    }*/

    std::vector<double> x;
    unsigned int nParts = theEvent.getNParticles();

    switch(nParts){ //TODO: other cases, better "x" description (selection of particles?)
    case 2:{
      const Particle &a(theEvent.getParticle(0));
      const Particle &b(theEvent.getParticle(1));

      double masssq = 0;
      masssq += (pow(a.E+b.E,2) - pow(a.px+b.px ,2) - pow(a.py+b.py ,2) - pow(a.pz+b.pz ,2));
      x.push_back(sqrt(masssq));

      break;
    }
    case 3:{
      const Particle &a(theEvent.getParticle(0));
      const Particle &b(theEvent.getParticle(1));
      const Particle &c(theEvent.getParticle(2));

      double masssqa = 0, masssqb = 0, masssqc = 0;
      masssqa += (pow(a.E+b.E,2) - pow(a.px+b.px ,2) - pow(a.py+b.py ,2) - pow(a.pz+b.pz ,2));
      masssqb += (pow(a.E+c.E,2) - pow(a.px+c.px ,2) - pow(a.py+c.py ,2) - pow(a.pz+c.pz ,2));
      masssqc += (pow(c.E+b.E,2) - pow(c.px+b.px ,2) - pow(c.py+b.py ,2) - pow(c.pz+b.pz ,2));
      x.push_back(sqrt(masssqa));
      x.push_back(sqrt(masssqb));
      x.push_back(sqrt(masssqc));

      break;
    }
    default:{
      //TODO: exception "i dont know how to handle this data"
      break;
    }
    }

    //std::cout << "ControlPar list " << minPar.GetNDouble() <<std::endl;

    double intens = pPIF_->intensity(x, minPar);
    if(intens>0){
      lh -= (log(intens/norm));
    }
  }

  return lh;
}
