/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>
#include "ns3/netanim-module.h"
//#include "ns3/gtk-config-store.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SimpleLteExample");

int main (int argc, char *argv[])
{	
  std::string animFile = "mysimple-animation.xml" ;  // Name of file for animation output
  CommandLine cmd;
  cmd.AddValue ("animFile",  "File Name for Animation Output", animFile);

  cmd.Parse (argc, argv);
  // to save a template default attribute file run it like this:
  // ./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Save --ns3::ConfigStore::FileFormat=RawText" --run src/lte/examples/lena-first-sim
  //
  // to load a previously created default attribute file
  // ./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Load --ns3::ConfigStore::FileFormat=RawText" --run src/lte/examples/lena-first-sim

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // Parse again so you can override default values from the command line
  cmd.Parse (argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  // Uncomment to enable logging
  // lteHelper->EnableLogComponents ();

  // Create Nodes: 1 eNodeB, 2 Pedestrian UE's and 1 Driver UE
  NodeContainer enbNodes;
  NodeContainer PueNodes;
  NodeContainer DueNodes;
  enbNodes.Create (1);
  PueNodes.Create (2);
  DueNodes.Create (1);

  // Install eNodeB Mobility Model
  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbMobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);

  // Install Pedestrian UE  Mobility Model
  MobilityHelper PueMobility;
  PueMobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (10.0),
                                 "MinY", DoubleValue (10.0),
                                 "DeltaX", DoubleValue (1.0),
                                 "DeltaY", DoubleValue (5.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
  PueMobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle (0, 25, 0, 25)));
  PueMobility.Install (PueNodes);
  BuildingsHelper::Install (PueNodes);

 // Install Driver UE  Mobility Model
  MobilityHelper DueMobility;
  DueMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  DueMobility.Install (DueNodes);
  DueNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (0, 0, 0));
  DueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (70, 70, 0));
  BuildingsHelper::Install (DueNodes);

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer PueDevs;
  NetDeviceContainer DueDevs;

  // Default scheduler is PF, uncomment to use RR
  // lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");

  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  PueDevs = lteHelper->InstallUeDevice (PueNodes);
  DueDevs = lteHelper->InstallUeDevice (DueNodes);

  // Attach an UE to a eNB
  lteHelper->Attach (PueDevs, enbDevs.Get (0));
  lteHelper->Attach (DueDevs, enbDevs.Get (0));

  // Activate a data radio bearer
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (PueDevs, bearer);
  lteHelper->ActivateDataRadioBearer (DueDevs, bearer);

  lteHelper->EnableTraces ();

  AnimationInterface::SetConstantPosition (enbNodes.Get (0), 25, 25); 

  // Create the animation object and configure for specified output
  AnimationInterface anim (animFile);

  Simulator::Stop (Seconds (5.00));

  Simulator::Run ();
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;

  // GtkConfigStore config;
  // config.ConfigureAttributes ();

  Simulator::Destroy ();

  return 0;
}
