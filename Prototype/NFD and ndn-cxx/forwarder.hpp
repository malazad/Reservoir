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

#ifndef NFD_DAEMON_FW_FORWARDER_HPP
#define NFD_DAEMON_FW_FORWARDER_HPP

#include "face-table.hpp"
#include "forwarder-counters.hpp"
#include "unsolicited-data-policy.hpp"
#include "face/face-endpoint.hpp"
#include "table/fib.hpp"
#include "table/pit.hpp"
#include "table/cs.hpp"
#include "table/measurements.hpp"
#include "table/strategy-choice.hpp"
#include "table/dead-nonce-list.hpp"
#include "table/network-region-table.hpp"

#include <fstream>

namespace nfd {

namespace fw {
class Strategy;
} // namespace fw

/** \brief Main class of NFD's forwarding engine.
 *
 *  Forwarder owns all tables and implements the forwarding pipelines.
 */
class Forwarder
{
public:
  explicit
  Forwarder(FaceTable& faceTable);

  VIRTUAL_WITH_TESTS
  ~Forwarder();

  const ForwarderCounters&
  getCounters() const
  {
    return m_counters;
  }

  fw::UnsolicitedDataPolicy&
  getUnsolicitedDataPolicy() const
  {
    return *m_unsolicitedDataPolicy;
  }

  void
  setUnsolicitedDataPolicy(unique_ptr<fw::UnsolicitedDataPolicy> policy)
  {
    BOOST_ASSERT(policy != nullptr);
    m_unsolicitedDataPolicy = std::move(policy);
  }

public: // forwarding entrypoints and tables
  /** \brief start incoming Interest processing
   *  \param ingress face on which Interest is received and endpoint of the sender
   *  \param interest the incoming Interest, must be well-formed and created with make_shared
   */
  void
  startProcessInterest(const FaceEndpoint& ingress, const Interest& interest)
  {
    this->onIncomingInterest(ingress, interest);
  }

  /** \brief start incoming Data processing
   *  \param ingress face on which Data is received and endpoint of the sender
   *  \param data the incoming Data, must be well-formed and created with make_shared
   */
  void
  startProcessData(const FaceEndpoint& ingress, const Data& data)
  {
    this->onIncomingData(ingress, data);
  }

  /** \brief start incoming Nack processing
   *  \param ingress face on which Nack is received and endpoint of the sender
   *  \param nack the incoming Nack, must be well-formed
   */
  void
  startProcessNack(const FaceEndpoint& ingress, const lp::Nack& nack)
  {
    this->onIncomingNack(ingress, nack);
  }

  /** \brief start new nexthop processing
   *  \param prefix the prefix of the FibEntry containing the new nexthop
   *  \param nextHop the new NextHop
   */
  void
  startProcessNewNextHop(const Name& prefix, const fib::NextHop& nextHop)
  {
    this->onNewNextHop(prefix, nextHop);
  }

  NameTree&
  getNameTree()
  {
    return m_nameTree;
  }

  Fib&
  getFib()
  {
    return m_fib;
  }

  Pit&
  getPit()
  {
    return m_pit;
  }

  Cs&
  getCs()
  {
    return m_cs;
  }

  Measurements&
  getMeasurements()
  {
    return m_measurements;
  }

  StrategyChoice&
  getStrategyChoice()
  {
    return m_strategyChoice;
  }

  DeadNonceList&
  getDeadNonceList()
  {
    return m_deadNonceList;
  }

  NetworkRegionTable&
  getNetworkRegionTable()
  {
    return m_networkRegionTable;
  }

PUBLIC_WITH_TESTS_ELSE_PRIVATE: // pipelines
  /** \brief incoming Interest pipeline
   */
  VIRTUAL_WITH_TESTS void
  onIncomingInterest(const FaceEndpoint& ingress, const Interest& interest);

  /** \brief Interest loop pipeline
   */
  VIRTUAL_WITH_TESTS void
  onInterestLoop(const FaceEndpoint& ingress, const Interest& interest);

  /** \brief Content Store miss pipeline
  */
  VIRTUAL_WITH_TESTS void
  onContentStoreMiss(const FaceEndpoint& ingress,
                     const shared_ptr<pit::Entry>& pitEntry, const Interest& interest);

  /** \brief Content Store hit pipeline
  */
  VIRTUAL_WITH_TESTS void
  onContentStoreHit(const FaceEndpoint& ingress, const shared_ptr<pit::Entry>& pitEntry,
                    const Interest& interest, const Data& data);

  /** \brief outgoing Interest pipeline
   */
  VIRTUAL_WITH_TESTS void
  onOutgoingInterest(const shared_ptr<pit::Entry>& pitEntry,
                     const FaceEndpoint& egress, const Interest& interest);

  /** \brief Interest finalize pipeline
   */
  VIRTUAL_WITH_TESTS void
  onInterestFinalize(const shared_ptr<pit::Entry>& pitEntry);

  /** \brief incoming Data pipeline
   */
  VIRTUAL_WITH_TESTS void
  onIncomingData(const FaceEndpoint& ingress, const Data& data);

  /** \brief Data unsolicited pipeline
   */
  VIRTUAL_WITH_TESTS void
  onDataUnsolicited(const FaceEndpoint& ingress, const Data& data);

  /** \brief outgoing Data pipeline
   */
  VIRTUAL_WITH_TESTS void
  onOutgoingData(const Data& data, const FaceEndpoint& egress);

  /** \brief incoming Nack pipeline
   */
  VIRTUAL_WITH_TESTS void
  onIncomingNack(const FaceEndpoint& ingress, const lp::Nack& nack);

  /** \brief outgoing Nack pipeline
   */
  VIRTUAL_WITH_TESTS void
  onOutgoingNack(const shared_ptr<pit::Entry>& pitEntry,
                 const FaceEndpoint& egress, const lp::NackHeader& nack);

  VIRTUAL_WITH_TESTS void
  onDroppedInterest(const FaceEndpoint& egress, const Interest& interest);

  VIRTUAL_WITH_TESTS void
  onNewNextHop(const Name& prefix, const fib::NextHop& nextHop);

PROTECTED_WITH_TESTS_ELSE_PRIVATE:
  /** \brief set a new expiry timer (now + \p duration) on a PIT entry
   */
  void
  setExpiryTimer(const shared_ptr<pit::Entry>& pitEntry, time::milliseconds duration);

  /** \brief insert Nonce to Dead Nonce List if necessary
   *  \param upstream if null, insert Nonces from all out-records;
   *                  if not null, insert Nonce only on the out-records of this face
   */
  VIRTUAL_WITH_TESTS void
  insertDeadNonceList(pit::Entry& pitEntry, Face* upstream);

  /** \brief call trigger (method) on the effective strategy of pitEntry
   */
#ifdef WITH_TESTS
  virtual void
  dispatchToStrategy(pit::Entry& pitEntry, std::function<void(fw::Strategy&)> trigger)
#else
  template<class Function>
  void
  dispatchToStrategy(pit::Entry& pitEntry, Function trigger)
#endif
  {
    trigger(m_strategyChoice.findEffectiveStrategy(pitEntry));
  }

private:
  ForwarderCounters m_counters;

  FaceTable& m_faceTable;
  unique_ptr<fw::UnsolicitedDataPolicy> m_unsolicitedDataPolicy;

  NameTree           m_nameTree;
  Fib                m_fib;
  Pit                m_pit;
  Cs                 m_cs;
  Measurements       m_measurements;
  StrategyChoice     m_strategyChoice;
  DeadNonceList      m_deadNonceList;
  NetworkRegionTable m_networkRegionTable;
  double             m_totalTime;
  std::ofstream incoming;
  std::ofstream outgoing;

  // allow Strategy (base class) to enter pipelines
  friend class fw::Strategy;
};

} // namespace nfd

#endif // NFD_DAEMON_FW_FORWARDER_HPP
