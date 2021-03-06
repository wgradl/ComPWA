//-------------------------------------------------------------------------------
// Copyright (c) 2013 Stefan Pflueger.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
//----------------------------------------------------------------------------------

#include <fstream>

#include "Physics/DecayTree/DecayConfiguration.hpp"
#include "Physics/DecayTree/DecayXMLConfigReader.hpp"
#include "Physics/DecayTree/DecayTreeFactory.hpp"

int main(int argc, char **argv) {
  BOOST_LOG_TRIVIAL(info) << "  ComPWA Copyright (C) 2013  Stefan Pflueger ";
  BOOST_LOG_TRIVIAL(info)
      << "  This program comes with ABSOLUTELY NO WARRANTY; for details see license.txt";
  BOOST_LOG_TRIVIAL(info) << " ";

  std::string input_config_file("Physics/HelicityAmplitude/JPSI_ypipi.xml");
  std::string output_file("graph.dot");

  ComPWA::Physics::DecayTree::DecayConfiguration decay_configuration;
  ComPWA::Physics::DecayTree::DecayXMLConfigReader xml_reader(decay_configuration);
  xml_reader.readConfig(input_config_file);

  ComPWA::Physics::DecayTree::DecayTreeFactory decay_tree_factory(decay_configuration);

  std::vector<ComPWA::Physics::DecayTree::DecayTree> decay_trees =
      decay_tree_factory.createDecayTrees();

  BOOST_LOG_TRIVIAL(info) << "created " << decay_trees.size() << " decay trees from "
      << input_config_file << " config file!";
  BOOST_LOG_TRIVIAL(info) << "printing to " << output_file << " output file";

  std::ofstream dot(output_file);

  std::vector<ComPWA::Physics::DecayTree::DecayTree>::iterator decay_tree;
  for (decay_tree = decay_trees.begin(); decay_tree != decay_trees.end();
      ++decay_tree) {
    decay_tree->print(dot);
  }

  xml_reader.writeConfig("test_config.xml");

  return 0;
}

