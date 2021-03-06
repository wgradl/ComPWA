//-------------------------------------------------------------------------------
// Copyright (c) 2014 Peter Weidenkaff.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
// Contributors:
//     Peter Weidenkaff
//-------------------------------------------------------------------------------
/** Test application to validate tree and amplitude
 * @file CompareTreeAmpApp.cpp
 * Application to validate that fitting using the function tree and using the amplitude produces the
 * same result. The decay D0->K_S0 K+ K- is used as example and the Breit-Wigner description of the
 * phi(1020) and the Flatte description of the a_0(980)0 are compared between tree and amplitude.
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>
#include <string>
#include <memory>

#include "boost/program_options.hpp"

// Root header files go here
#include "TF1.h"
#include "TH1D.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TH2D.h"

//Core header files go here
#include "Core/Event.hpp"
#include "Core/Particle.hpp"
#include "Core/Parameter.hpp"
#include "Core/ParameterList.hpp"
#include "Core/RunManager.hpp"
#include "Core/Efficiency.hpp"
#include "Core/FunctionTree.hpp"

// ComPWA header files go here
#include "DataReader/RootReader/RootReader.hpp"
#include "Physics/AmplitudeSum/AmpSumIntensity.hpp"
#include "Physics/AmplitudeSum/AmpRelBreitWignerRes.hpp"
#include "Physics/AmplitudeSum/AmpFlatteRes.hpp"
#include "Estimator/MinLogLH/MinLogLH.hpp"
#include "Optimizer/Minuit2/MinuitIF.hpp"
#include "Physics/DPKinematics/RootEfficiency.cpp"
#include "Physics/DPKinematics/RootGenerator.cpp"
#include "Core/TableFormater.hpp"
#include "Core/AbsParameter.hpp"
#include "Core/Logging.hpp"

using namespace ComPWA;
using namespace Physics::AmplitudeSum;

/************************************************************************************************/
/**
 * The main function.
 */
int main(int argc, char **argv){
	int seed = 3041; //default seed

	//number of calls for numeric integration and number of events for phsp integration
	unsigned int mcPrecision = 500000;
	Logging log("log-compareTreeAmp.txt",boost::log::trivial::debug); //initialize logging

	//initialize kinematics of decay
	Physics::DPKinematics::DalitzKinematics::createInstance("D0","K_S0","K-","K+");//setup kinematics
	//initialize random generator
	std::shared_ptr<Generator> gen =
			std::shared_ptr<Generator>(
					new Physics::DPKinematics::RootGenerator(seed)
			);

	RunManager run;
	run.setGenerator(gen);
	//======================= DATA =============================
	unsigned int numEvents = 2000;//data size to be generated

	//empty file: run generation before fit
	std::shared_ptr<Data> inputData(
			new DataReader::RootReader()
	);

	//======================= EFFICIENCY =============================
	std::shared_ptr<Efficiency> eff(new UnitEfficiency());

	//======================= AMPLITUDE =============================
	//true amplitude model
	std::string trueModelFile = "test/CompareTreeAmp-model.xml";

	auto a = new Physics::AmplitudeSum::AmpSumIntensity(
			"amp", normStyle::one, eff, mcPrecision
	);
	a->Configure(trueModelFile);
	std::shared_ptr<Amplitude> trueAmp(a);

	//fit amplitude model
	std::string fitModelFile = trueModelFile;
	auto fitAmpPtr = new Physics::AmplitudeSum::AmpSumIntensity(
			"fitAmp", normStyle::one, eff, mcPrecision
	);
	fitAmpPtr->Configure(fitModelFile);
	auto fitAmpTreePtr = new Physics::AmplitudeSum::AmpSumIntensity(
			"fitAmpTree", normStyle::one, eff, mcPrecision
	);
	fitAmpTreePtr->Configure(fitModelFile);
	//	AmplitudeSetup ini(fitModelFile);//put start parameters here
	//	AmplitudeSetup iniTree(fitModelFile);//put start parameters here
	//	AmpSumIntensity* fitAmpPtr = new AmpSumIntensity(ini, normStyle::one, eff, mcPrecision);
	//	AmpSumIntensity* fitAmpTreePtr = new AmpSumIntensity(iniTree, normStyle::one, eff, mcPrecision);
	std::shared_ptr<Amplitude> fitAmp(fitAmpPtr);
	std::shared_ptr<Amplitude> fitAmpTree(fitAmpTreePtr);

	run.setAmplitude(trueAmp);//set true model here for generation

	//empty phsp sample
	std::shared_ptr<Data> toyPhspData(
			new DataReader::RootReader()
	);

	run.setPhspSample(toyPhspData);
	if( !toyPhspData->getNEvents() ) {
		run.generatePhsp(mcPrecision);
	}
	toyPhspData->setEfficiency(eff);

	run.setData(inputData);
	if( !inputData->getNEvents() ) {
		run.generate(numEvents);
	}

	//======================= PARAMETERS =============================
	ParameterList fitPar;
	fitAmp->FillParameterList(fitPar);
	ParameterList fitParTree;
	fitAmpTree->FillParameterList(fitParTree);
	ParameterList truePar;
	trueAmp->FillParameterList(truePar); //true values

	//	fitParTree.GetDoubleParameter("g1_a_0")->FixParameter(1);
	//	fitPar.GetDoubleParameter("g1_a_0")->FixParameter(1);

	for(unsigned int i=0; i<fitParTree.GetNDouble(); i++)
		fitParTree.GetDoubleParameter(i)->SetError(.1);
	for(unsigned int i=0; i<fitPar.GetNDouble(); i++)
		fitPar.GetDoubleParameter(i)->SetError(.1);

	fitAmpTree->UpdateParameters(fitParTree);
	fitAmp->UpdateParameters(fitPar);
	fitAmp->to_str();

	BOOST_LOG_TRIVIAL(info)<<"Entries in data file: "<<inputData->getNEvents();
	BOOST_LOG_TRIVIAL(info)<<"True model file: "<<trueModelFile ;
	BOOST_LOG_TRIVIAL(info)<<"Fit model file: "<<fitModelFile ;

	//======================= TREE FIT =============================
	std::shared_ptr<ControlParameter> esti(Estimator::MinLogLH::MinLogLH::createInstance(
			fitAmpTree, inputData, toyPhspData));
	Estimator::MinLogLH::MinLogLH* minLog = dynamic_cast<Estimator::MinLogLH::MinLogLH*>(&*(esti->Instance()));
	minLog->setUseFunctionTree(1);
	std::shared_ptr<FunctionTree> physicsTree = minLog->getTree();
	BOOST_LOG_TRIVIAL(debug) << physicsTree->head()->to_str(20);
	double initialLHTree = esti->controlParameter(fitParTree);
	std::shared_ptr<Optimizer::Optimizer> optiTree(
			new Optimizer::Minuit2::MinuitIF(esti, fitParTree)
	);
	run.setOptimizer(optiTree);

	//======================= Compare tree and amplitude =============================
	std::shared_ptr<AmpRelBreitWignerRes> phiRes =
			std::dynamic_pointer_cast<AmpRelBreitWignerRes>(
					fitAmpPtr->GetResonance("phi(1020)")
			);
	double phimag = fitAmpPtr->GetResonance("phi(1020)")->GetMagnitude();
	double phiphase = fitAmpPtr->GetResonance("phi(1020)")->GetPhase();
	std::complex<double> phiCoeff( phimag*cos(phiphase), phimag*sin(phiphase) );

	std::shared_ptr<AmpFlatteRes> a0Res =
			std::dynamic_pointer_cast<AmpFlatteRes>(
					fitAmpPtr->GetResonance("a_0(980)0")
			);
	double a0mag = fitAmpPtr->GetResonance("a_0(980)0")->GetMagnitude();
	double a0phase = fitAmpPtr->GetResonance("a_0(980)0")->GetPhase();
	std::complex<double> a0Coeff( a0mag*cos(a0phase), a0mag*sin(a0phase) );

	std::shared_ptr<AmpFlatteRes> aplusRes =
			std::dynamic_pointer_cast<AmpFlatteRes>(
					fitAmpPtr->GetResonance("a_0(980)+")
			);
	double aplusmag = fitAmpPtr->GetResonance("a_0(980)+")->GetMagnitude();
	double aplusphase = fitAmpPtr->GetResonance("a_0(980)+")->GetPhase();
	std::complex<double> aplusCoeff(
			aplusmag*cos(aplusphase),
			aplusmag*sin(aplusphase)
	);

	dataPoint point(inputData->getEvent(0)); //first data point in sample
	ParameterList intens = fitAmpPtr->intensity(point);

	/*the tree contains multiple node with the same names. We search for node 'Intens' first and
	 * afterwards for the resonance.*/
	std::shared_ptr<TreeNode> intensNode;
	MultiDouble* intensValue;
	intensNode = physicsTree->head()->getChildNode("AmpSq");
	if(intensNode)
		intensValue = dynamic_cast<MultiDouble*>( &*(intensNode->getValue()) );
	else {
		BOOST_LOG_TRIVIAL(error)<<"Failed to get Node 'AmpSq'!";
		throw;
	}

	BOOST_LOG_TRIVIAL(info) <<" Total integral of phi(1020) "<<phiRes->GetTotalIntegral();
	std::cout<<std::setprecision(8)<<std::endl;
	BOOST_LOG_TRIVIAL(info) <<"===========================================";
	BOOST_LOG_TRIVIAL(info) <<"Compare values: (use first event of data sample) TREE/AMPLITUDE";
	BOOST_LOG_TRIVIAL(info) <<"===========================================";
	BOOST_LOG_TRIVIAL(info) <<"Intensity: "<<intensValue->GetValue(0)
											<<"/"<<*intens.GetDoubleParameter(0)
											<<" = "<<intensValue->GetValue(0) / *intens.GetDoubleParameter(0);
	double norm_tree = physicsTree->head()->getChildSingleValue("N").real();
	double norm_amp = 1/fitAmp->GetIntegral();
	BOOST_LOG_TRIVIAL(info)<<" Norm: "<<norm_tree <<"/"<<norm_amp
			<<" = "<<norm_tree/norm_amp;
	BOOST_LOG_TRIVIAL(info) <<"================= phi(1020) ==========================";
	BOOST_LOG_TRIVIAL(info) <<"Reso_phi(1020): "<<intensNode->getChildSingleValue("Reso_phi(1020)")
											<<"/"<<phiRes->Evaluate(point);
	BOOST_LOG_TRIVIAL(info) <<"N_phi(1020): "<<intensNode->getChildSingleValue("N_phi(1020)").real()
											<<"/"<<phiRes->GetNormalization()
											<<" = "<<intensNode->getChildSingleValue("N_phi(1020)").real()/phiRes->GetNormalization();
	BOOST_LOG_TRIVIAL(info) <<"RelBW_phi(1020): "<<intensNode->getChildSingleValue("RelBW_phi(1020)")
											<<"/"<<phiRes->EvaluateAmp(point);
	BOOST_LOG_TRIVIAL(info) <<"AngD_phi(1020): "<<intensNode->getChildSingleValue("AngD_phi(1020)").real()
											<<"/"<<phiRes->EvaluateWignerD(point);
	BOOST_LOG_TRIVIAL(info) <<"================= a_0(980)0 ==========================";
	BOOST_LOG_TRIVIAL(info) <<"Reso_a_0(980)0: "<<intensNode->getChildSingleValue("Reso_a_0(980)0")
											<<"/"<<a0Res->Evaluate(point);
	BOOST_LOG_TRIVIAL(info) <<"N_a_0(980)0: "<<intensNode->getChildSingleValue("N_a_0(980)0").real()
											<<"/"<<a0Res->GetNormalization();
	BOOST_LOG_TRIVIAL(info) <<"FlatteRes_a_0(980)0: "<<intensNode->getChildSingleValue("FlatteRes_a_0(980)0")
											<<"/"<<a0Res->EvaluateAmp(point);
	BOOST_LOG_TRIVIAL(info) <<"================= a_0(980)+ ==========================";
	BOOST_LOG_TRIVIAL(info) <<"Reso_a_0(980)+: "<<intensNode->getChildSingleValue("Reso_a_0(980)+")
											<<"/"<<aplusRes->Evaluate(point);
	BOOST_LOG_TRIVIAL(info) <<"N_a_0(980)+: "<<intensNode->getChildSingleValue("N_a_0(980)+").real()
											<<"/"<<aplusRes->GetNormalization();
	BOOST_LOG_TRIVIAL(info) <<"FlatteRes_a_0(980)+: "<<intensNode->getChildSingleValue("FlatteRes_a_0(980)+")
											<<"/"<<aplusRes->EvaluateAmp(point);
	BOOST_LOG_TRIVIAL(info) <<"===========================================";
	std::shared_ptr<FitResult> resultTree = run.startFit(fitParTree);
	double finalLHTree = resultTree->getResult();
	resultTree->setTrueParameters(truePar);//set true parameters

	//======================= AMPLITUDE FIT =============================
	esti->resetInstance();
	esti = std::shared_ptr<ControlParameter>(
			Estimator::MinLogLH::MinLogLH::createInstance(
					fitAmp, inputData, toyPhspData)
	);
	double initialLH = esti->controlParameter(fitPar);

	std::cout<<std::setprecision(20);
	BOOST_LOG_TRIVIAL(info) <<"Initial likelihood: "<<initialLHTree<< "/"<<initialLH
			<< " Deviation = "<<initialLHTree-initialLH;

	std::shared_ptr<Optimizer::Optimizer> opti(
			new Optimizer::Minuit2::MinuitIF(esti, fitPar)
	);
	run.setOptimizer(opti);

	std::shared_ptr<FitResult> result= run.startFit(fitPar);
	double finalLH = result->getResult();
	result->setTrueParameters(truePar);//set true parameters

	//======================= OUTPUT =============================
	BOOST_LOG_TRIVIAL(info) <<"TREE fit result:";
	resultTree->print("P");
	BOOST_LOG_TRIVIAL(info) <<"AMPLITUDE fit result:";
	result->print("P");
	BOOST_LOG_TRIVIAL(info) <<"Comparison TREE/AMPLITUDE:";
	BOOST_LOG_TRIVIAL(info) <<"Timings[s]: "<<resultTree->getTime()<<"/"<<result->getTime()
											<<"="<<resultTree->getTime()/result->getTime();
	BOOST_LOG_TRIVIAL(info) <<"Initial likelihood: "<<initialLHTree<< "/"<<initialLH
			<< " Deviation = "<<initialLHTree/initialLH;
	BOOST_LOG_TRIVIAL(info) <<"Final likelihood: "<<finalLHTree<< "/"<<finalLH
			<< " Deviation = "<<finalLHTree/finalLH;


	BOOST_LOG_TRIVIAL(info) << "FINISHED!";
	return 0;
}
