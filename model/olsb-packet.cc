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

#include "olsb-packet.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"

namespace ns3 {
namespace olsb {

NS_OBJECT_ENSURE_REGISTERED (OlsbHeader);

OlsbHeader::OlsbHeader (Ipv4Address dst, uint32_t hopCount, uint32_t dstSeqNo, uint32_t queueSize)
  : m_dst (dst),
    m_hopCount (hopCount),
    m_dstSeqNo (dstSeqNo),
    m_queuesize (queueSize)
{
}

OlsbHeader::~OlsbHeader ()
{
}

TypeId
OlsbHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::olsb::OlsbHeader")
    .SetParent<Header> ()
    .SetGroupName ("Olsb")
    .AddConstructor<OlsbHeader> ();
  return tid;
}

TypeId
OlsbHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
OlsbHeader::GetSerializedSize () const
{
  return 16;
}

void
OlsbHeader::Serialize (Buffer::Iterator i) const
{
  WriteTo (i, m_dst);
  i.WriteHtonU32 (m_hopCount);
  i.WriteHtonU32 (m_dstSeqNo);
  i.WriteHtonU32 (m_queuesize);
}

uint32_t
OlsbHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  ReadFrom (i, m_dst);
  m_hopCount = i.ReadNtohU32 ();
  m_dstSeqNo = i.ReadNtohU32 ();
  m_queuesize = i.ReadNtohU32 ();

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
OlsbHeader::Print (std::ostream &os) const
{
  os << "DestinationIpv4: " << m_dst
     << " Hopcount: " << m_hopCount
     << " SequenceNumber: " << m_dstSeqNo
     << " QueueSize: " << m_queuesize;
}
}
}
