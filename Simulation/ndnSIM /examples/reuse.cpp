/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>. 
 * 
 **/

// ndn-simple.cpp


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include <vector>
#include <string>

namespace ns3 {

/**
 * This scenario simulates a very simple network topology:
 *
 *
 *      +----------+     1Mbps      +--------+     1Mbps      +----------+
 *      | consumer | <------------> | router | <------------> | producer |
 *      +----------+         10ms   +--------+          10ms  +----------+
 *
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.reuseConsumer:ndn.Producer ./waf --run=reuse_102 2>&1 | tee dumplog/Dodge2/ssim_0.1/reuse_102_topo_1_seed_0.txt
 */

int
main(int argc, char* argv[])
{
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("1.5ms"));
  Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20000p"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 25);
  // topo 1
  //topologyReader.SetFileName("src/ndnSIM/examples/topologies/Selected 5 topologies/topology_21_nodes_10CNs_11ENs_singleedgeEN_2.txt");
  // topo 2
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/Selected 5 topologies/topology_26_nodes_10CNs_16ENs_singleedgeEN_11.txt");
  // topo 3
  //topologyReader.SetFileName("src/ndnSIM/examples/topologies/Selected 5 topologies/topology_30_nodes_10CNs_19ENs_singleedgeEN_62.txt");
  // topo 4
  //topologyReader.SetFileName("src/ndnSIM/examples/topologies/Selected 5 topologies/topology_35_nodes_10CNs_25ENs_singleedgeEN_48.txt");
  // topo 5
  //topologyReader.SetFileName("src/ndnSIM/examples/topologies/Selected 5 topologies/topology_40_nodes_10CNs_29ENs_singleedgeEN_45.txt");
  topologyReader.Read();

  uint32_t NumberOfUsers = 100;
  uint32_t NumberOfEN = 16;
  uint32_t NumberOfCN = 10;

  RngSeedManager::SetSeed(1);
  Ptr<UniformRandomVariable> m_rand;
  m_rand = CreateObject<UniformRandomVariable>();

  // Creating nodes
  NodeContainer nodes;
  nodes.Create(NumberOfUsers);

  // Connecting nodes using two links
  PointToPointHelper p2p;

  for(int i = 0; i < NumberOfUsers; i++)
  {
    uint64_t pickedEN = m_rand->GetValue(0, NumberOfEN);
    Ptr<Node> ConnectedNode = Names::Find<Node>("EN" + std::to_string(pickedEN));
    p2p.Install(nodes.Get(i),ConnectedNode);
    NS_LOG_UNCOND("User = " << i << " is connected to EN = " << pickedEN );
  }

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();


  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll( "/cn", "/localhost/nfd/strategy/best-route3");
  for(int i = 0; i < NumberOfEN; i++)
  {
    NS_LOG_UNCOND("installing b3 at EN " << i );
    NS_LOG_UNCOND("installing consumer app " << i );
    Ptr<Node> EN = Names::Find<Node>("EN" + std::to_string(i));
    ndn::StrategyChoiceHelper::Install(EN, "/service/1", "/localhost/nfd/strategy/best-route3");
  }
  for(int i = 0; i < NumberOfUsers; i++)
  {
    NS_LOG_UNCOND("installing consumer app " << i );
    ndn::AppHelper consumerHelper("ns3::ndn::reuseConsumer");

    consumerHelper.SetAttribute("InterestName", StringValue("/service/1"));
    consumerHelper.SetAttribute("UserID", StringValue(std::to_string(i)));
    auto apps = consumerHelper.Install(nodes.Get(i));

  }

  for(int i = 0; i < NumberOfCN; i++)
  {
    NS_LOG_UNCOND("installing producer app " << i );
    Ptr<Node> CN = Names::Find<Node>("CN" + std::to_string(i));
    ndn::AppHelper producerHelper("ns3::ndn::Producer");
    // Producer will reply to all requests starting with /prefix
    producerHelper.SetPrefix("/cn/" + std::to_string(i) );
    producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
    producerHelper.Install(CN); // last node
    //ndnGlobalRoutingHelper.AddOrigin("/service", CN);
    ndnGlobalRoutingHelper.AddOrigin("/cn/" + std::to_string(i), CN);
  }



  
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(10.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
