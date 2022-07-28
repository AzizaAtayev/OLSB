/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Aziza Atayev
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
 * Author: Aziza Atayev <azizaa@post.bgu.ac.il>
 * Kobi lab reference
 * Ben Gurion University (BGU)
 * Department of Electrical Engineering
 * Beer Sheva, Israel.
 *
 */

#include "ns3/test.h"
#include "ns3/mesh-helper.h"
#include "ns3/simulator.h"
#include "ns3/mobility-helper.h"
#include "ns3/olsb-helper.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/v4ping-helper.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/pcap-file.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/olsb-packet.h"
#include "ns3/olsb-rtable.h"

using namespace ns3;

/**
 * \ingroup olsb-test
 * \defgroup olsb-test OLSB module tests
 */


/**
 * \ingroup olsb-test
 * \ingroup tests
 *
 * \brief OLSB test case to verify the OLSB header
 *
 */
class OlsbHeaderTestCase : public TestCase
{
public:
  OlsbHeaderTestCase ();
  ~OlsbHeaderTestCase ();
  virtual void
  DoRun (void);
};
OlsbHeaderTestCase::OlsbHeaderTestCase ()
  : TestCase ("Verifying the OLSB header")
{
}
OlsbHeaderTestCase::~OlsbHeaderTestCase ()
{
}
void
OlsbHeaderTestCase::DoRun ()
{
  Ptr<Packet> packet = Create<Packet> ();

  {
    olsb::OlsbHeader hdr1;
    hdr1.SetDst (Ipv4Address ("10.1.1.2"));
    hdr1.SetDstSeqno (2);
    hdr1.SetHopCount (2);
    packet->AddHeader (hdr1);
    olsb::OlsbHeader hdr2;
    hdr2.SetDst (Ipv4Address ("10.1.1.3"));
    hdr2.SetDstSeqno (4);
    hdr2.SetHopCount (1);
    packet->AddHeader (hdr2);
    NS_TEST_ASSERT_MSG_EQ (packet->GetSize (), 24, "001");
  }

  {
    olsb::OlsbHeader hdr2;
    packet->RemoveHeader (hdr2);
    NS_TEST_ASSERT_MSG_EQ (hdr2.GetSerializedSize (),16,"002");
    NS_TEST_ASSERT_MSG_EQ (hdr2.GetDst (),Ipv4Address ("10.1.1.3"),"003");
    NS_TEST_ASSERT_MSG_EQ (hdr2.GetDstSeqno (),4,"004");
    NS_TEST_ASSERT_MSG_EQ (hdr2.GetHopCount (),1,"005");
    olsb::OlsbHeader hdr1;
    packet->RemoveHeader (hdr1);
    NS_TEST_ASSERT_MSG_EQ (hdr1.GetSerializedSize (),16,"006");
    NS_TEST_ASSERT_MSG_EQ (hdr1.GetDst (),Ipv4Address ("10.1.1.2"),"008");
    NS_TEST_ASSERT_MSG_EQ (hdr1.GetDstSeqno (),2,"009");
    NS_TEST_ASSERT_MSG_EQ (hdr1.GetHopCount (),2,"010");
  }
}

/**
 * \ingroup olsb-test
 * \ingroup tests
 *
 * \brief OLSB routing table tests (adding and looking up routes)
 */
class OlsbTableTestCase : public TestCase
{
public:
  OlsbTableTestCase ();
  ~OlsbTableTestCase ();
  virtual void
  DoRun (void);
};

OlsbTableTestCase::OlsbTableTestCase ()
  : TestCase ("Olsb Routing Table test case")
{
}
OlsbTableTestCase::~OlsbTableTestCase ()
{
}
void
OlsbTableTestCase::DoRun ()
{
  olsb::RoutingTable rtable;
  Ptr<NetDevice> dev;
  {
    olsb::RoutingTableEntry rEntry1 (
      /*device=*/ dev, /*dst=*/
      Ipv4Address ("10.1.1.4"), /*seqno=*/ 2,
      /*iface=*/ Ipv4InterfaceAddress (Ipv4Address ("10.1.1.1"), Ipv4Mask ("255.255.255.0")),
      /*hops=*/ 2, 
      /*queue size*/ 0,
      /*next hop=*/ Ipv4Address ("10.1.1.2"),
      /*lifetime=*/ Seconds (10));
    NS_TEST_EXPECT_MSG_EQ (rtable.AddRoute (rEntry1),true,"add route");
    olsb::RoutingTableEntry rEntry2 (
      /*device=*/ dev, /*dst=*/
      Ipv4Address ("10.1.1.2"), /*seqno=*/ 4,
      /*iface=*/ Ipv4InterfaceAddress (Ipv4Address ("10.1.1.1"), Ipv4Mask ("255.255.255.0")),
      /*hops=*/ 1, 
      /*queue size*/ 2,
      /*next hop=*/ Ipv4Address ("10.1.1.2"),
      /*lifetime=*/ Seconds (10));
    NS_TEST_EXPECT_MSG_EQ (rtable.AddRoute (rEntry2),true,"add route");
    olsb::RoutingTableEntry rEntry3 (
      /*device=*/ dev, /*dst=*/
      Ipv4Address ("10.1.1.3"), /*seqno=*/ 4,
      /*iface=*/ Ipv4InterfaceAddress (Ipv4Address ("10.1.1.1"), Ipv4Mask ("255.255.255.0")),
      /*hops=*/ 1, 
      /*queue size*/ 1,
      /*next hop=*/ Ipv4Address ("10.1.1.3"),
      /*lifetime=*/ Seconds (10));
    NS_TEST_EXPECT_MSG_EQ (rtable.AddRoute (rEntry3),true,"add route");
    olsb::RoutingTableEntry rEntry4 (
      /*device=*/ dev, /*dst=*/
      Ipv4Address ("10.1.1.255"), /*seqno=*/ 0,
      /*iface=*/ Ipv4InterfaceAddress (Ipv4Address ("10.1.1.1"), Ipv4Mask ("255.255.255.0")),
      /*hops=*/ 0, 
      /*queue size*/ 2,
      /*next hop=*/ Ipv4Address ("10.1.1.255"),
      /*lifetime=*/ Seconds (10));
    NS_TEST_EXPECT_MSG_EQ (rtable.AddRoute (rEntry4),true,"add route");
  }
  {
    olsb::RoutingTableEntry rEntry;
    if (rtable.LookupRoute (Ipv4Address ("10.1.1.4"), rEntry))
      {
        NS_TEST_ASSERT_MSG_EQ (rEntry.GetDestination (),Ipv4Address ("10.1.1.4"),"100");
        NS_TEST_ASSERT_MSG_EQ (rEntry.GetSeqNo (),2,"101");
        NS_TEST_ASSERT_MSG_EQ (rEntry.GetHop (),2,"102");
        NS_TEST_ASSERT_MSG_EQ (rEntry.GetQueueSize (),0,"112");
      }
    if (rtable.LookupRoute (Ipv4Address ("10.1.1.2"), rEntry))
      {
        NS_TEST_ASSERT_MSG_EQ (rEntry.GetDestination (),Ipv4Address ("10.1.1.2"),"103");
        NS_TEST_ASSERT_MSG_EQ (rEntry.GetSeqNo (),4,"104");
        NS_TEST_ASSERT_MSG_EQ (rEntry.GetHop (),1,"105");
        NS_TEST_ASSERT_MSG_EQ (rEntry.GetQueueSize (),2,"113");
      }
    if (rtable.LookupRoute (Ipv4Address ("10.1.1.3"), rEntry))
      {
        NS_TEST_ASSERT_MSG_EQ (rEntry.GetDestination (),Ipv4Address ("10.1.1.3"),"106");
        NS_TEST_ASSERT_MSG_EQ (rEntry.GetSeqNo (),4,"107");
        NS_TEST_ASSERT_MSG_EQ (rEntry.GetHop (),1,"108");
        NS_TEST_ASSERT_MSG_EQ (rEntry.GetQueueSize (),1,"114");
      }
    if (rtable.LookupRoute (Ipv4Address ("10.1.1.255"), rEntry))
      {
        NS_TEST_ASSERT_MSG_EQ (rEntry.GetDestination (),Ipv4Address ("10.1.1.255"),"109");
      }
    NS_TEST_ASSERT_MSG_EQ (rEntry.GetInterface ().GetLocal (),Ipv4Address ("10.1.1.1"),"110");
    NS_TEST_ASSERT_MSG_EQ (rEntry.GetInterface ().GetBroadcast (),Ipv4Address ("10.1.1.255"),"111");
    NS_TEST_ASSERT_MSG_EQ (rtable.RoutingTableSize (),4,"Rtable size incorrect");
  }
  Simulator::Destroy ();
}

/**
 * \ingroup olsb-test
 * \ingroup tests
 *
 * \brief OLSB test suite
 */
class OlsbTestSuite : public TestSuite
{
public:
  OlsbTestSuite () : TestSuite ("routing-olsb", UNIT)
  {
    AddTestCase (new OlsbHeaderTestCase (), TestCase::QUICK);
    AddTestCase (new OlsbTableTestCase (), TestCase::QUICK);
  }
} g_olsbTestSuite; ///< the test suite
