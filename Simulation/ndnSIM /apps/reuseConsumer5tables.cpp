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

#include "reuseConsumer5tables.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"
#include "ns3/random-variable-stream.h"

#include <memory>
#include <fstream>
#include <iostream>
#include <string>



#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/double.h"

#include "utils/ndn-ns3-packet-tag.hpp"
#include "utils/ndn-rtt-mean-deviation.hpp"

#include <ndn-cxx/lp/tags.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/ref.hpp>

NS_LOG_COMPONENT_DEFINE("ndn.reuseConsumer5tables");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(reuseConsumer5tables);

TypeId
reuseConsumer5tables::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::reuseConsumer5tables")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<reuseConsumer5tables>()
      .AddAttribute("ProducerPrefix", "Prefix, for which reuseConsumer5tables has the data", StringValue("/"),
                    MakeNameAccessor(&reuseConsumer5tables::m_prefix), MakeNameChecker())
	  .AddAttribute("InterestName", "Interest Name", StringValue("/"),
                    MakeNameAccessor(&reuseConsumer5tables::m_interestName), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding reuseConsumer5tables-uniqueness)",
         StringValue("/"), MakeNameAccessor(&reuseConsumer5tables::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&reuseConsumer5tables::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      
    .AddAttribute("UserID", "", UintegerValue(0),
                    MakeUintegerAccessor(&reuseConsumer5tables::m_UserID),
                    MakeUintegerChecker<uint32_t>())


	  .AddAttribute("LifeTime", "LifeTime for interest packet", StringValue("2s"),
			        MakeTimeAccessor(&reuseConsumer5tables::m_interestLifeTime), MakeTimeChecker())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&reuseConsumer5tables::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&reuseConsumer5tables::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&reuseConsumer5tables::m_keyLocator), MakeNameChecker());
  return tid;
}

reuseConsumer5tables::reuseConsumer5tables()
{
  NS_LOG_FUNCTION_NOARGS();
}

// inherited from Application base class.
void
reuseConsumer5tables::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  dataFilePath = "/home/malazad@unomaha.edu/Desktop/LSH/simulation_data_pre/userData/Vehicle2/ssim_0.6/seed1/user_" + std::to_string(m_UserID) +"_seed_1.txt";
  _file.open(dataFilePath);
  NS_LOG_INFO(" User ID = " << m_UserID << "Datapath = " << dataFilePath);
  App::StartApplication();


  //FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);

  SendInterest();
}

void
reuseConsumer5tables::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
reuseConsumer5tables::SendInterest()
{
    
	  shared_ptr<Name> interestName = make_shared<Name>(m_interestName);
    std::string nonce;
    getline(_file, nonce, ',');
    if (nonce == "ENDofTXT")
    {
      NS_LOG_DEBUG("Reached the end 1 : " << m_nonce);
      return;
    }

    //interestName->append(nonce);
    getline(_file, image_name, ',');
    image_class = image_name.substr(16,2);
    getline(_file, hash1, ',');
    interestName->append(hash1);
    getline(_file, hash2, ',');
    interestName->append(hash2);
    getline(_file, hash3, ',');
    interestName->append(hash3);
    getline(_file, hash4, ',');
    interestName->append(hash4);
    getline(_file, hash5, ',');
    interestName->append(hash5);
    getline(_file, bin, ',');
    interestName->append(bin);
    std::string temp_dump;
    getline(_file, temp_dump);
	  //interestName->appendSequenceNumber(seq);
	  //std::stringstream temp_comp;
	  //temp_comp << m_seq;
	  //interestName->append(temp_comp.str());
    m_nonce = std::stoul(nonce) ;
    /*
	  m_seq++;
    m_nonce++;
    if(m_seq > 2)
    {
      m_seq = 0;
    }
    */
	  shared_ptr<Interest> interest = make_shared<Interest>();
	  //interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
    interest->setNonce(m_nonce);
	  interest->setName(*interestName);
	  //interest->setCanBePrefix(false);
	  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
	  interest->setInterestLifetime(interestLifeTime);

    /*
    if(m_seq == 1 && m_nonce > 2 )
    {
      std::string temp_parameters = "This is a parameter";
      const char* buff = temp_parameters.c_str();
	    size_t buff_size = temp_parameters.length();
      interest->setApplicationParameters(make_shared< ::ndn::Buffer>(buff,buff_size));
      NS_LOG_INFO("This one with a parameter.");
    }
    */

	  NS_LOG_INFO("> Interest for " << *interestName);


	  m_transmittedInterests(interest, this, m_face);
	  m_appLink->onReceiveInterest(*interest);
    m_sendingTime = Simulator::Now().GetMilliSeconds();


	  //ScheduleNextPacket();
}
/*
void
reuseConsumer5tables::ScheduleNextPacket()
{

	  if (!m_sendEvent.IsRunning())
	      m_sendEvent = Simulator::Schedule(Seconds(1.0),&reuseConsumer5tables::SendInterest, this);
}
*/

/*

void
reuseConsumer5tables::OnInterest(shared_ptr<const Interest> interest)
{

  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);

  if (!m_active)
    return;

  Name dataName(interest->getName());
  // dataName.append(m_postfix);
  // dataName.appendVersion();

  auto data = make_shared<Data>();
  data->setName(dataName);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

  data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));

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

*/

void
reuseConsumer5tables::OnData(shared_ptr<const Data> data)
{
  if (!m_active)
    return;

  App::OnData(data); // tracing inside

  NS_LOG_FUNCTION(this << data);

  // NS_LOG_INFO ("Received content object: " << boost::cref(*data));

  // This could be a problem......
  //uint32_t seq = data->getName().at(-1).toSequenceNumber();
  NS_LOG_INFO("< DATA for " << data->getName());

  int acc = 1;
  Block contentBlock = data->getContent();
  std::string content = std::string((char*)contentBlock.value());
  if(image_class != content.substr(6,2))
  {
    acc = 0;
  }
  
  NS_LOG_INFO("Logger:Nonce = " << m_nonce << " Logger:Name = " << image_name << " Logger:Latency = " << (Simulator::Now().GetMilliSeconds() - m_sendingTime) 
              << " Logger:Acc = " << acc << "endline");

  int hopCount = 0;
  auto hopCountTag = data->getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }
  NS_LOG_DEBUG("Hop count: " << hopCount);
  if (_file.eof())
  {
     NS_LOG_DEBUG("Reached the end : " << m_nonce);
    return;
  }
  SendInterest();
	
}

} // namespace ndn
} // namespace ns3
