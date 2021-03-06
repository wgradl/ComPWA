// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#ifndef ABSTRACTDYNAMICALFUNCTION_HPP
#define ABSTRACTDYNAMICALFUNCTION_HPP

#include <complex>

#include <boost/property_tree/ptree.hpp>

#include "Core/Properties.hpp"
#include "Core/DataPoint.hpp"
#include "Core/ParameterList.hpp"
#include "Core/Spin.hpp"
#include "Core/FunctionTree.hpp"

#include "Physics/DecayDynamics/FormFactor.hpp"

namespace ComPWA {
namespace Physics {
namespace DecayDynamics {

class AbstractDynamicalFunction {
  
public:
  //============ CONSTRUCTION ==================
  
  AbstractDynamicalFunction()
      : _daughterMasses(std::pair<double, double>(-999, -999)),
        _current_mass(-999), _mcPrecision(1000000), _modified(true){};

  virtual ~AbstractDynamicalFunction(){};

  //======= INTEGRATION/NORMALIZATION ===========
  
  bool CheckModified() const {
    if (GetMass() != _current_mass) {
      SetModified();
      const_cast<double &>(_current_mass) = _mass->GetValue();
      return true;
    }
    return false;
  }

  //================ EVALUATION =================
  
  virtual std::complex<double>
  Evaluate(const ComPWA::dataPoint &point, int pos) const = 0;

  //============ SET/GET =================
  
  virtual void SetName(std::string n) { _name = n; }

  virtual std::string GetName() { return _name; }

  virtual void SetModified(bool b = true) const {
    const_cast<bool &>(_modified) = b;
    const_cast<double &>(_current_mass) = _mass->GetValue();
  }

  virtual bool GetModified() const { return _modified; }

  virtual void GetParameters(ParameterList &list);

  virtual void GetParametersFast(std::vector<double> &list) const {
    list.push_back(GetMass());
  }
  
  /// Update parameters to the values given in \p par
  virtual void UpdateParameters(const ParameterList &par) = 0;

  virtual void SetMassParameter(std::shared_ptr<DoubleParameter> mass) {
    _mass = mass;
  }

  virtual std::shared_ptr<DoubleParameter> GetMassParameter() { return _mass; }

  virtual void SetMass(double mass) { _mass->SetValue(mass); }

  virtual double GetMass() const { return _mass->GetValue(); }

  virtual void SetDecayMasses(std::pair<double, double> m) {
    _daughterMasses = m;
  }

  virtual std::pair<double, double> GetDecayMasses() const {
    return _daughterMasses;
  }

  virtual void SetDecayNames(std::pair<std::string, std::string> n) {
    _daughterNames = n;
  }

  virtual std::pair<std::string, std::string> GetDecayNames() const {
    return _daughterNames;
  }

  virtual ComPWA::Spin GetSpin() const { return _spin; }

  virtual void SetSpin(ComPWA::Spin spin) { _spin = spin; }

  //=========== FUNCTIONTREE =================
  
  virtual bool HasTree() const { return false; }
  
  virtual std::shared_ptr<ComPWA::FunctionTree>
  GetTree(const ComPWA::ParameterList &sample, int pos, std::string suffix = "") = 0;

protected:
  std::string _name;

  /// Precision of MC integration
  int _mcPrecision;

  /// Masses of daughter particles
  std::pair<double, double> _daughterMasses;

  /// Names of daughter particles
  std::pair<std::string, std::string> _daughterNames;

  /// Resonance mass
  std::shared_ptr<ComPWA::DoubleParameter> _mass;

  /// Resonance spin
  ComPWA::Spin _spin;

private:
  /// Resonance shape was modified (recalculate the normalization)
  bool _modified;

  /// Temporary value of mass (used to trigger recalculation of normalization)
  double _current_mass;
};

} /* namespace DecayDynamics */
} /* namespace Physics */
} /* namespace ComPWA */

#endif /* ABSTRACTDYNAMICALFUNCTION_HPP */
