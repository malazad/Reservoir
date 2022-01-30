/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2019,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <vector>
#include <algorithm> 
#include <ndn-cxx/link.hpp>
#include <string>  
#include <typeinfo> 
  

#include "reservoir-strategy.hpp"
#include "algorithm.hpp"
#include "common/logger.hpp"
#include<iostream>

namespace nfd {
namespace fw {

NFD_LOG_INIT(ReservoirStrategy);
NFD_REGISTER_STRATEGY(ReservoirStrategy);

const time::milliseconds ReservoirStrategy::RETX_SUPPRESSION_INITIAL(10);
const time::milliseconds ReservoirStrategy::RETX_SUPPRESSION_MAX(250);

ReservoirStrategy::ReservoirStrategy(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder)
  , ProcessNackTraits(this)
  , m_32bit(4294967295)
  , m_NumberOfCN(2)
  , m_retxSuppression(RETX_SUPPRESSION_INITIAL,
                      RetxSuppressionExponential::DEFAULT_MULTIPLIER,
                      RETX_SUPPRESSION_MAX)
{
  m_blockSize = (uint32_t) (m_32bit/m_NumberOfCN);
  ParsedInstanceName parsed = parseInstanceName(name);
  if (!parsed.parameters.empty()) {
    NDN_THROW(std::invalid_argument("ReservoirStrategy does not accept parameters"));
  }
  if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
    NDN_THROW(std::invalid_argument(
      "ReservoirStrategy does not support version " + to_string(*parsed.version)));
  }
  this->setInstanceName(makeInstanceName(name, getStrategyName()));
}

const Name&
ReservoirStrategy::getStrategyName()
{
  static Name strategyName("/localhost/nfd/strategy/reservoir-strategy/%FD%05");
  return strategyName;
}

void
ReservoirStrategy::afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
                                         const shared_ptr<pit::Entry>& pitEntry)
{
  RetxSuppressionResult suppression = m_retxSuppression.decidePerPitEntry(*pitEntry);
  if (suppression == RetxSuppressionResult::SUPPRESS) {
    NFD_LOG_DEBUG(interest << " from=" << ingress << " suppressed");
    return;
  }

  if(!interest.getForwardingHint().empty())
  {
    if (interest.getForwardingHint().at(0).name.getSubName(0,1)  == "/service1")
    {
        if (interest.getForwardingHint().at(1).name.getSubName(0,1) == "/data")
        {
            uint32_t FH_CN = (uint32_t) ((std::stoul(interest.getName().getSubName(2,1).toUri().substr(1)) /m_blockSize));
            std::string forwardingHint;
            forwardingHint = "/cn/" + std::to_string(FH_CN);
            //std::cout << "FHCN = " << FH_CN << "  hash = " << interest.getName().getSubName(2,1).toUri().substr(1) << std::endl;

            ::ndn::Link m_link;
            Interest redirectingInterest(interest.getName());
            //auto redirectingInterest = make_shared<Interest>(interest.getName());
            m_link.addDelegation(0, forwardingHint);
            m_link.addDelegation(10, interest.getForwardingHint().at(1).name);
            redirectingInterest.setForwardingHint(m_link.getDelegationList());
            redirectingInterest.setNonce(interest.getNonce());
            //redirctingInterest.setCanBePrefix(false);
            const fib::Entry& fibEntry = this->lookupFib2(redirectingInterest);

            const fib::NextHopList& nexthops = fibEntry.getNextHops();

            auto it = nexthops.end();
            it = std::find_if(nexthops.begin(), nexthops.end(), [&] (const auto& nexthop) {
              return isNextHopEligible(ingress.face, redirectingInterest, nexthop, pitEntry, true, time::steady_clock::now());
            });

            if (it != nexthops.end()) 
            {
              NFD_LOG_DEBUG( "FH added = " << forwardingHint);
              //std::cout << "FH added = " << forwardingHint << std::endl;
              auto egress = FaceEndpoint(it->getFace(), 0);
              this->sendInterest(pitEntry, egress, redirectingInterest);
              return;
            }


        }
    }
    else
    {
      NFD_LOG_DEBUG("FH = " << interest.getForwardingHint() );
      //std::cout << "No FH "  << std::endl;

      
      const fib::Entry& fibEntry = this->lookupFib2(interest);
      const fib::NextHopList& nexthops = fibEntry.getNextHops();
      auto it = nexthops.end();

      it = std::find_if(nexthops.begin(), nexthops.end(), [&] (const auto& nexthop) {
        return isNextHopEligible(ingress.face, interest, nexthop, pitEntry, true, time::steady_clock::now());
      });
      
      if (it != nexthops.end()) {
        auto egress = FaceEndpoint(it->getFace(), 0);
        this->sendInterest(pitEntry, egress, interest);
        return;
      }


  }


}

}
void
ReservoirStrategy::afterReceiveNack(const FaceEndpoint& ingress, const lp::Nack& nack,
                                     const shared_ptr<pit::Entry>& pitEntry)
{
  this->processNack(ingress.face, nack, pitEntry);
}

} // namespace fw
} // namespace nfd
