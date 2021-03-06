// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

//! dataPoint stores kinematic information of a dalitz plot
/*!
 * @file DataPoint.hpp
 *\class dataPoint
 *      dataPoint is a singleton class which provides a
 *      certain phase space point to all classes of the framework. The point can
 *be set anywhere in
 *      the framework and can be read by any amplitude class.
 */

#ifndef DPPOINT2_HPP_
#define DPPOINT2_HPP_

#include <cstdlib>
#include <math.h>
#include <vector>
#include <map>

namespace ComPWA {

class dataPoint {

public:
  dataPoint();

  void Reset(unsigned int size);

  std::size_t Size() const { return var.size(); }

  void SetValue(unsigned int pos, double val);
  
  double GetValue(unsigned int num) const;

  std::vector<double>& GetPoint() { return var; };

  void SetWeight(double w) { weight = w; };
  
  double GetWeight() const { return weight; };
  
  void SetEfficiency(double e) { eff = e; };
  
  double GetEfficiency() const { return eff; };

protected:
  std::vector<double> var;
  double weight;
  double eff;
  friend std::ostream &operator<<(std::ostream &os, const dataPoint &p);
};

} /* namespace ComPWA */
#endif /*DPPOINT2_HPP_*/
