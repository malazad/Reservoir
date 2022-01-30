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
 **/

#include "ndn-producer5tables.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

#include <memory>
//#include"readcsv.hpp"

NS_LOG_COMPONENT_DEFINE("ndn.Producer5tables");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(Producer5tables);

TypeId
Producer5tables::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::Producer5tables")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<Producer5tables>()
      .AddAttribute("Prefix", "Prefix, for which Producer5tables has the data", StringValue("/"),
                    MakeNameAccessor(&Producer5tables::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding Producer5tables-uniqueness)",
         StringValue("/"), MakeNameAccessor(&Producer5tables::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&Producer5tables::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&Producer5tables::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&Producer5tables::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&Producer5tables::m_keyLocator), MakeNameChecker());
  return tid;
}

Producer5tables::Producer5tables()
: dataFilePath("/home/malazad@unomaha.edu/Desktop/LSH/simulation_data_pre/feedVehicle2_ssim_0.6_newend2_nonce.txt")
{
  NS_LOG_FUNCTION_NOARGS();
}

// inherited from Application base class.
void
Producer5tables::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  //_file.open(dataFilePath);
  
  
  App::StartApplication();

  FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
  //FibHelper::AddRoute(GetNode(), "/prefix/task", m_face, 0);
  
}

void
Producer5tables::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
Producer5tables::OnInterest(shared_ptr<const Interest> interest)
{
  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);

  if (!m_active)
    return;

  Name dataName(interest->getName());
  // dataName.append(m_postfix);
  // dataName.appendVersion();
  NS_LOG_INFO("incoming image  Nonce = " << interest->getNonce());
  std::string incoming_image = data.getImageName(std::to_string(interest->getNonce()), dataFilePath);
  
  NS_LOG_INFO("incoming image  Nonce = " << interest->getNonce() << " Name = " << incoming_image);

  std::vector<std::string> matchList = data.getMatch( std::to_string(interest->getNonce()), dataFilePath);
  NS_LOG_INFO("Match size  = " << matchList.size());
  /*
  for(int _ = 0; _ < matchList.size(); _++)
  {
    NS_LOG_INFO("Match " << _ << " = " << matchList[_]);
  }
  */
  bool newExecution = true;
  std::string payload;
  for(int it = 0; it < matchList.size(); it++)
  {
    if( std::find(cacheList.begin(), cacheList.end(), matchList[it]) != cacheList.end())
    {
      int position = std::distance(std::begin(cacheList), std::find(cacheList.begin(), cacheList.end(), matchList[it]));
      payload = "class=" + cacheList[position].substr(16,2) + "endline";
      NS_LOG_INFO("Results reused for Nonce " << interest->getNonce() << " Name: " << incoming_image << "endline");
      newExecution = false;
      break;
    }
  }
  
  if(newExecution)
  {

    //------------------------------------for wrong CN -------------------------------

    for(int it = 0; it < matchList.size(); it++)
    {
      if( std::find(cacheList.begin(), cacheList.end(), matchList[it]) != cacheList.end())
      {
        NS_LOG_INFO("wrong CN");
        break;
      }
    }






    //--------------------------------------------------------------------------------



      payload = "class=" + incoming_image.substr(16,2) + "endline";
      NS_LOG_INFO("No reused for Nonce " << interest->getNonce() << " Name: " << incoming_image << "endline");
  }
  
  cacheList.push_back(incoming_image);


  
  const char* buff = payload.c_str();
  size_t buff_size = payload.length();


  auto data = make_shared<Data>();
  data->setName(dataName);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

  data->setContent(make_shared< ::ndn::Buffer>(buff,buff_size));

  Signature signature;
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  signature.setInfo(signatureInfo);
  signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

  data->setSignature(signature);

  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());

  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
}

} // namespace ndn
} // namespace ns3
