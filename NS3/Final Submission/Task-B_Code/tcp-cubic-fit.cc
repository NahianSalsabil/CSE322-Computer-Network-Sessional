/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Natale Patriciello <natale.patriciello@gmail.com>
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
 */

#define NS_LOG_APPEND_CONTEXT \
  { std::clog << Simulator::Now ().GetSeconds () << " "; }

#include "tcp-cubic-fit.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("TcpCubicFit");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpCubicFit);

TypeId
TcpCubicFit::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpCubicFit")
    .SetParent<TcpSocketBase> ()
    .AddConstructor<TcpCubicFit> ()
    .SetGroupName ("Internet")
    .AddAttribute ("FastConvergence", "Enable (true) or disable (false) fast convergence",
                   BooleanValue (true),
                   MakeBooleanAccessor (&TcpCubicFit::m_fastConvergence),
                   MakeBooleanChecker ())
    .AddAttribute ("Beta", "Beta for multiplicative decrease",
                   DoubleValue (0.7),
                   MakeDoubleAccessor (&TcpCubicFit::m_beta),
                   MakeDoubleChecker <double> (0.0))
    .AddAttribute ("HyStart", "Enable (true) or disable (false) hybrid slow start algorithm",
                   BooleanValue (true),
                   MakeBooleanAccessor (&TcpCubicFit::m_hystart),
                   MakeBooleanChecker ())
    .AddAttribute ("HyStartLowWindow", "Lower bound cWnd for hybrid slow start (segments)",
                   UintegerValue (16),
                   MakeUintegerAccessor (&TcpCubicFit::m_hystartLowWindow),
                   MakeUintegerChecker <uint32_t> ())
    .AddAttribute ("HyStartDetect", "Hybrid Slow Start detection mechanisms:" \
                   "1: packet train, 2: delay, 3: both",
                   IntegerValue (3),
                   MakeIntegerAccessor (&TcpCubicFit::m_hystartDetect),
                   MakeIntegerChecker <int> (1,3))
    .AddAttribute ("HyStartMinSamples", "Number of delay samples for detecting the increase of delay",
                   UintegerValue (8),
                   MakeUintegerAccessor (&TcpCubicFit::m_hystartMinSamples),
                   MakeUintegerChecker <uint8_t> ())
    .AddAttribute ("HyStartAckDelta", "Spacing between ack's indicating train",
                   TimeValue (MilliSeconds (2)),
                   MakeTimeAccessor (&TcpCubicFit::m_hystartAckDelta),
                   MakeTimeChecker ())
    .AddAttribute ("HyStartDelayMin", "Minimum time for hystart algorithm",
                   TimeValue (MilliSeconds (4)),
                   MakeTimeAccessor (&TcpCubicFit::m_hystartDelayMin),
                   MakeTimeChecker ())
    .AddAttribute ("HyStartDelayMax", "Maximum time for hystart algorithm",
                   TimeValue (MilliSeconds (1000)),
                   MakeTimeAccessor (&TcpCubicFit::m_hystartDelayMax),
                   MakeTimeChecker ())
    .AddAttribute ("CubicDelta", "Delta Time to wait after fast recovery before adjusting param",
                   TimeValue (MilliSeconds (10)),
                   MakeTimeAccessor (&TcpCubicFit::m_cubicDelta),
                   MakeTimeChecker ())
    .AddAttribute ("CntClamp", "Counter value when no losses are detected (counter is used" \
                   " when incrementing cWnd in congestion avoidance, to avoid" \
                   " floating point arithmetic). It is the modulo of the (avoided)" \
                   " division",
                   UintegerValue (20),
                   MakeUintegerAccessor (&TcpCubicFit::m_cntClamp),
                   MakeUintegerChecker <uint8_t> ())
    .AddAttribute ("C", "Cubic Scaling factor",
                   DoubleValue (0.4),
                   MakeDoubleAccessor (&TcpCubicFit::m_c),
                   MakeDoubleChecker <double> (0.0))
    // Cubic Fit Modification
    .AddAttribute ("N_t", "Number of Flow",
                    DoubleValue (1.0),
                    MakeDoubleAccessor (&TcpCubicFit::N_t),
                   MakeDoubleChecker <double> (0.0))
    .AddAttribute ("Alpha", "janina",
                    DoubleValue (1.0),
                    MakeDoubleAccessor (&TcpCubicFit::m_alpha),
                   MakeDoubleChecker <double> (0.0))
  ;
  return tid;
}

TcpCubicFit::TcpCubicFit ()
  : TcpCongestionOps (),
    m_cWndCnt (0),
    m_lastMaxCwnd (0),
    m_bicOriginPoint (0),
    m_bicK (0.0),
    m_delayMin (Time::Min ()),
    m_epochStart (Time::Min ()),
    m_found (false),
    m_roundStart (Time::Min ()),
    m_endSeq (0),
    m_lastAck (Time::Min ()),
    m_cubicDelta (Time::Min ()),
    m_currRtt (Time::Min ()),
    m_minRtt (Time::Max()),
    m_maxRtt (Time::Min()),
    m_sampleCnt (0)
{
  NS_LOG_FUNCTION (this);
}

TcpCubicFit::TcpCubicFit (const TcpCubicFit &sock)
  : TcpCongestionOps (sock),
    m_fastConvergence (sock.m_fastConvergence),
    m_beta (sock.m_beta),
    m_hystart (sock.m_hystart),
    m_hystartDetect (sock.m_hystartDetect),
    m_hystartLowWindow (sock.m_hystartLowWindow),
    m_hystartAckDelta (sock.m_hystartAckDelta),
    m_hystartDelayMin (sock.m_hystartDelayMin),
    m_hystartDelayMax (sock.m_hystartDelayMax),
    m_hystartMinSamples (sock.m_hystartMinSamples),
    m_initialCwnd (sock.m_initialCwnd),
    m_cntClamp (sock.m_cntClamp),
    m_c (sock.m_c),
    m_cWndCnt (sock.m_cWndCnt),
    m_lastMaxCwnd (sock.m_lastMaxCwnd),
    m_bicOriginPoint (sock.m_bicOriginPoint),
    m_bicK (sock.m_bicK),
    m_delayMin (sock.m_delayMin),
    m_epochStart (sock.m_epochStart),
    m_found (sock.m_found),
    m_roundStart (sock.m_roundStart),
    m_endSeq (sock.m_endSeq),
    m_lastAck (sock.m_lastAck),
    m_cubicDelta (sock.m_cubicDelta),
    m_currRtt (sock.m_currRtt),
    m_minRtt (sock.m_minRtt),
    m_maxRtt (sock.m_maxRtt),
    m_sampleCnt (sock.m_sampleCnt)
{
  NS_LOG_FUNCTION (this);
}

std::string
TcpCubicFit::GetName () const
{
  return "TcpCubicFit";
}

void
TcpCubicFit::HystartReset (Ptr<const TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this);

  m_roundStart = m_lastAck = Simulator::Now ();
  m_endSeq = tcb->m_highTxMark;
  m_currRtt = Time::Min ();
  m_minRtt = Time::Max();
  m_maxRtt = Time::Min();
  m_sampleCnt = 0;
}

void
TcpCubicFit::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (tcb->m_cWnd < tcb->m_ssThresh)
    {

      if (m_hystart && tcb->m_lastAckedSeq > m_endSeq)
        {
          HystartReset (tcb);
        }

      // In Linux, the QUICKACK socket option enables the receiver to send
      // immediate acks initially (during slow start) and then transition
      // to delayed acks.  ns-3 does not implement QUICKACK, and if ack
      // counting instead of byte counting is used during slow start window
      // growth, when TcpSocket::DelAckCount==2, then the slow start will
      // not reach as large of an initial window as in Linux.  Therefore,
      // we can approximate the effect of QUICKACK by making this slow
      // start phase perform Appropriate Byte Counting (RFC 3465)
      tcb->m_cWnd += segmentsAcked * tcb->m_segmentSize;
      segmentsAcked = 0;

      NS_LOG_INFO ("In SlowStart, updated to cwnd " << tcb->m_cWnd <<
                   " ssthresh " << tcb->m_ssThresh);
    }

  if (tcb->m_cWnd >= tcb->m_ssThresh && segmentsAcked > 0)
    {
      m_cWndCnt += segmentsAcked;
      uint32_t cnt = Update (tcb);

      /* According to RFC 6356 even once the new cwnd is
       * calculated you must compare this to the number of ACKs received since
       * the last cwnd update. If not enough ACKs have been received then cwnd
       * cannot be updated.
       */
      if (m_cWndCnt >= cnt)
        {
          tcb->m_cWnd += tcb->m_segmentSize;
          m_cWndCnt -= cnt;
          NS_LOG_INFO ("In CongAvoid, updated to cwnd " << tcb->m_cWnd);
        }
      else
        {
          NS_LOG_INFO ("Not enough segments have been ACKed to increment cwnd."
                       "Until now " << m_cWndCnt << " cnd " << cnt);
        }
    }
}

uint32_t
TcpCubicFit::Update (Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this);
  Time t;
  uint32_t delta, bicTarget, cnt = 0;
  double offs;
  uint32_t segCwnd = tcb->GetCwndInSegments ();

  if (m_epochStart == Time::Min ())
    {
      m_epochStart = Simulator::Now ();   // record the beginning of an epoch

      if (m_lastMaxCwnd <= segCwnd)
        {
          NS_LOG_DEBUG ("lastMaxCwnd <= m_cWnd. K=0 and origin=" << segCwnd);
          m_bicK = 0.0;
          m_bicOriginPoint = segCwnd;
        }
      else
        {
          // Cubic Fit Modification
          m_bicK = std::pow ((10*m_lastMaxCwnd) / (19*N_t + 1), 1 / 3.);
          m_bicOriginPoint = m_lastMaxCwnd;
          NS_LOG_DEBUG ("lastMaxCwnd > m_cWnd. K=" << m_bicK <<
                        " and origin=" << m_lastMaxCwnd);
        }
    }

  t = Simulator::Now () + m_delayMin - m_epochStart;

  //Cubic Fit Modification
  m_alpha = std::min(1/10.0, (m_maxRtt.GetSeconds() - m_minRtt.GetSeconds())/(2*m_maxRtt.GetSeconds()));
  N_t = std::max(1.0, (N_t + 1 - ((m_currRtt.GetSeconds() - m_minRtt.GetSeconds())/(m_alpha*m_currRtt.GetSeconds())*N_t)));


  if (t.GetSeconds () < m_bicK)       /* t - I */
    {
      offs = m_bicK - (N_t * t.GetSeconds ());     // Cubic Fit Modification   (I-Nt)
      NS_LOG_DEBUG ("t=" << t.GetSeconds () << " <k: offs=" << offs);
    }
  else
    {
      offs = (N_t * t.GetSeconds ()) - m_bicK;    //  // Cubic Fit Modification  (Nt-I)
      NS_LOG_DEBUG ("t=" << t.GetSeconds () << " >= k: offs=" << offs);
    }


  /* Constant value taken from Experimental Evaluation of Cubic Tcp, available at
   * eprints.nuim.ie/1716/1/Hamiltonpfldnet2007_cubic_final.pdf */

  // Cubic Fit Modification
  delta = m_c * std::pow (offs, 3);    //   0.4(Nt-I)^3

  NS_LOG_DEBUG ("delta: " << delta);

  if (t.GetSeconds () < m_bicK)
    {
      // below origin
      bicTarget = m_bicOriginPoint - delta;
      NS_LOG_DEBUG ("t < k: Bic Target: " << bicTarget);
    }
  else
    {
      // above origin
      bicTarget = m_bicOriginPoint + delta;
      NS_LOG_DEBUG ("t >= k: Bic Target: " << bicTarget);
    }

  // Next the window target is converted into a cnt or count value. CUBIC will
  // wait until enough new ACKs have arrived that a counter meets or exceeds
  // this cnt value. This is how the CUBIC implementation simulates growing
  // cwnd by values other than 1 segment size.
  if (bicTarget > segCwnd)
    {
      cnt = segCwnd / (bicTarget - segCwnd);
      NS_LOG_DEBUG ("target>cwnd. cnt=" << cnt);
    }
  else
    {
      cnt = 100 * segCwnd;
    }

  if (m_lastMaxCwnd == 0 && cnt > m_cntClamp)
    {
      cnt = m_cntClamp;
    }


  // The maximum rate of cwnd increase CUBIC allows is 1 packet per
  // 2 packets ACKed, meaning cwnd grows at 1.5x per RTT.
  return std::max (cnt, 2U);
}

void
TcpCubicFit::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                     const Time &rtt)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked << rtt);

  /* Discard delay samples right after fast recovery */
  if (m_epochStart != Time::Min ()
      && (Simulator::Now () - m_epochStart) < m_cubicDelta)
    {
      return;
    }

  /* first time call or link delay decreases */
  if (m_delayMin == Time::Min () || m_delayMin > rtt)
    {
      m_delayMin = rtt;
    }

  // Cubic Fit Modification
  if (rtt.IsZero ())
    {
      return;
    }

  // Keep track of maximum RTT
  m_maxRtt = std::max (rtt, m_maxRtt);

  //keep track of minimum RTT
  m_minRtt = std::min(rtt, m_minRtt);


  /* hystart triggers when cwnd is larger than some threshold */
  if (m_hystart
      && tcb->m_cWnd <= tcb->m_ssThresh
      && tcb->m_cWnd >= m_hystartLowWindow * tcb->m_segmentSize)
    {
      HystartUpdate (tcb, rtt);
    }

}

void
TcpCubicFit::HystartUpdate (Ptr<TcpSocketState> tcb, const Time& delay)
{
  NS_LOG_FUNCTION (this << delay);

  if (!(m_found & m_hystartDetect))
    {
      Time now = Simulator::Now ();

      /* first detection parameter - ack-train detection */
      if ((now - m_lastAck) <= m_hystartAckDelta)
        {
          m_lastAck = now;

          if ((now - m_roundStart) > m_delayMin)
            {
              m_found |= PACKET_TRAIN;
            }
        }

      /* obtain the minimum delay of more than sampling packets */
      if (m_sampleCnt < m_hystartMinSamples)
        {
          if (m_currRtt == Time::Min () || m_currRtt > delay)
            {
              m_currRtt = delay;
            }

          ++m_sampleCnt;
        }
      else
        {
          if (m_currRtt > m_delayMin +
              HystartDelayThresh (m_delayMin))
            {
              m_found |= DELAY;
            }
        }
      /*
       * Either one of two conditions are met,
       * we exit from slow start immediately.
       */
      if (m_found & m_hystartDetect)
        {
          NS_LOG_DEBUG ("Exit from SS, immediately :-)");
          tcb->m_ssThresh = tcb->m_cWnd;
        }
    }
}

Time
TcpCubicFit::HystartDelayThresh (const Time& t) const
{
  NS_LOG_FUNCTION (this << t);

  Time ret = t;
  if (t > m_hystartDelayMax)
    {
      ret = m_hystartDelayMax;
    }
  else if (t < m_hystartDelayMin)
    {
      ret = m_hystartDelayMin;
    }

  return ret;
}

uint32_t
TcpCubicFit::GetSsThresh (Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight)
{
  NS_LOG_FUNCTION (this << tcb << bytesInFlight);

  uint32_t segCwnd = tcb->GetCwndInSegments ();
  NS_LOG_DEBUG ("Loss at cWnd=" << segCwnd << " segments in flight=" << bytesInFlight / tcb->m_segmentSize);

  /* Wmax and fast convergence */
  if (segCwnd < m_lastMaxCwnd && m_fastConvergence)
    {
      m_lastMaxCwnd = (segCwnd * (1 + m_beta)) / 2; // Section 4.6 in RFC 8312
    }
  else
    {
      m_lastMaxCwnd = segCwnd;
    }

  m_epochStart = Time::Min ();    // end of epoch

  /* Formula taken from the Linux kernel */
  uint32_t ssThresh = std::max (static_cast<uint32_t> (segCwnd * m_beta ), 2U) * tcb->m_segmentSize;

  NS_LOG_DEBUG ("SsThresh = " << ssThresh);

  return ssThresh;
}

void
TcpCubicFit::CongestionStateSet (Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCongState_t newState)
{
  NS_LOG_FUNCTION (this << tcb << newState);

  if (newState == TcpSocketState::CA_LOSS)
    {
      CubicReset (tcb);
      HystartReset (tcb);
    }
}

void
TcpCubicFit::CubicReset (Ptr<const TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);

  m_lastMaxCwnd = 0;
  m_bicOriginPoint = 0;
  m_bicK = 0;
  m_delayMin = Time::Min ();
  m_found = false;
}

Ptr<TcpCongestionOps>
TcpCubicFit::Fork (void)
{
  NS_LOG_FUNCTION (this);
  return CopyObject<TcpCubicFit> (this);
}

}
