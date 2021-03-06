// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#ifndef PHYSICS_DECAYDYNAMICS_RELATIVISTICBREITWIGNER_HPP_
#define PHYSICS_DECAYDYNAMICS_RELATIVISTICBREITWIGNER_HPP_

#include <vector>
#include <memory>
#include <boost/property_tree/ptree.hpp>

#include "Core/Spin.hpp"
#include "Core/Functions.hpp"
#include "Core/Exceptions.hpp"
#include "Physics/DecayDynamics/AbstractDynamicalFunction.hpp"
#include "Physics/HelicityFormalism/AmpWignerD.hpp"

namespace ComPWA {
namespace Physics {
namespace DecayDynamics {

class PartialDecay;

/// \class RelativisticBreitWigner
/// Relativistic Breit-Wigner model with barrier factors
/// The dynamical function implemented here is taken from PDG2014 (Eq.47-22) for
/// the one channel case.
class RelativisticBreitWigner
    : public ComPWA::Physics::DecayDynamics::AbstractDynamicalFunction {

public:
  static std::shared_ptr<ComPWA::Physics::DecayDynamics::AbstractDynamicalFunction>
  Factory(std::shared_ptr<ComPWA::PartList> partL,
          const boost::property_tree::ptree &pt);

  /// Check of parameters have changed and normalization has to be recalculatecd
  virtual bool CheckModified() const;

  //================ EVALUATION =================
  std::complex<double> Evaluate(const ComPWA::dataPoint &point, int pos) const;

  /// Dynamical Breit-Wigner function.
  /// \param mSq Invariant mass squared
  /// \param mR Mass of the resonant state
  /// \param ma Mass of daughter particle
  /// \param mb Mass of daughter particle
  /// \param width Decay width
  /// \param J Spin
  /// \param mesonRadius Meson Radius
  /// \param ffType Form factor type
  /// \return Amplitude value
  static std::complex<double>
  dynamicalFunction(double mSq, double mR, double ma, double mb, double width,
                    unsigned int J, double mesonRadius, formFactorType ffType);

  //============ SET/GET =================

  void SetWidthParameter(std::shared_ptr<ComPWA::DoubleParameter> w) {
    _width = w;
  }

  std::shared_ptr<ComPWA::DoubleParameter> GetWidthParameter() {
    return _width;
  }

  void SetWidth(double w) { _width->SetValue(w); }

  double GetWidth() const { return _width->GetValue(); }

  void SetMesonRadiusParameter(std::shared_ptr<ComPWA::DoubleParameter> r) {
    _mesonRadius = r;
  }

  std::shared_ptr<ComPWA::DoubleParameter> GetMesonRadiusParameter() {
    return _mesonRadius;
  }

  /// \see GetMesonRadius() const { return _mesonRadius->GetValue(); }
  void SetMesonRadius(double w) { _mesonRadius->SetValue(w); }

  /// Get meson radius.
  /// The meson radius is a measure of the size of the resonant state. It is
  /// used to calculate the angular momentum barrier factors.
  double GetMesonRadius() const { return _mesonRadius->GetValue(); }

  /// \see GetFormFactorType()
  void SetFormFactorType(formFactorType t) { _ffType = t; }

  /// Get form factor type.
  /// The type of formfactor that is used to calculate the angular momentum
  /// barrier factors.
  formFactorType GetFormFactorType() { return _ffType; }

  virtual void GetParameters(ComPWA::ParameterList &list);

  virtual void GetParametersFast(std::vector<double> &list) const {
    AbstractDynamicalFunction::GetParametersFast(list);
    list.push_back(GetWidth());
    list.push_back(GetMesonRadius());
  }

  /// Update parameters to the values given in \p par
  virtual void UpdateParameters(const ComPWA::ParameterList &par);

  //=========== FUNCTIONTREE =================

  virtual bool HasTree() const { return true; }

  virtual std::shared_ptr<ComPWA::FunctionTree>
  GetTree(const ParameterList &sample, int pos, std::string suffix = "");

protected:
  /// Decay width of resonante state
  std::shared_ptr<ComPWA::DoubleParameter> _width;

  /// Meson radius of resonant state
  std::shared_ptr<ComPWA::DoubleParameter> _mesonRadius;

  /// Form factor type
  formFactorType _ffType;

private:
  /// Temporary values (used to trigger recalculation of normalization)
  double _current_mesonRadius;
  double _current_width;
};

class BreitWignerStrategy : public ComPWA::Strategy {
public:
  BreitWignerStrategy(const std::string resonanceName)
      : ComPWA::Strategy(ParType::MCOMPLEX), name(resonanceName) {}

  virtual const std::string to_str() const {
    return ("relativistic BreitWigner of " + name);
  }

  virtual bool execute(ComPWA::ParameterList &paras,
                       std::shared_ptr<ComPWA::AbsParameter> &out);

protected:
  std::string name;
};

} // namespace DecayDynamics
} // namespace Physics
} // namespace ComPWA

#endif
