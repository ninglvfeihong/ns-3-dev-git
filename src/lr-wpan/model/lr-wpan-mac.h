/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 The Boeing Company
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
 * Authors:
 *  Gary Pei <guangyu.pei@boeing.com>
 *  kwong yin <kwong-sang.yin@boeing.com>
 *  Tom Henderson <thomas.r.henderson@boeing.com>
 *  Sascha Alexander Jopen <jopen@cs.uni-bonn.de>
 */
#ifndef LR_WPAN_MAC_H
#define LR_WPAN_MAC_H

#include <ns3/object.h>
#include <ns3/traced-callback.h>
#include <ns3/traced-value.h>
#include <ns3/mac16-address.h>
#include <ns3/mac64-address.h>
#include <ns3/sequence-number.h>
#include <ns3/lr-wpan-phy.h>
#include <ns3/event-id.h>
#include <deque>


namespace ns3 {

class Packet;
class LrWpanCsmaCa;

/**
 * \defgroup lr-wpan LR-WPAN models
 *
 * This section documents the API of the IEEE 802.15.4-related models.  For a generic functional description, please refer to the ns-3 manual.
 */

/**
 * \ingroup lr-wpan
 *
 * Tx options
 */
typedef enum
{
  TX_OPTION_NONE = 0,    //!< TX_OPTION_NONE
  TX_OPTION_ACK = 1,     //!< TX_OPTION_ACK
  TX_OPTION_GTS = 2,     //!< TX_OPTION_GTS
  TX_OPTION_INDIRECT = 4 //!< TX_OPTION_INDIRECT
} LrWpanTxOption;

/**
 * \ingroup lr-wpan
 *
 * MAC states
 */
typedef enum
{
  MAC_IDLE,              //!< MAC_IDLE
  MAC_CSMA,              //!< MAC_CSMA
  MAC_SENDING,           //!< MAC_SENDING
  MAC_ACK_PENDING,       //!< MAC_ACK_PENDING
  CHANNEL_ACCESS_FAILURE,//!< CHANNEL_ACCESS_FAILURE
  CHANNEL_IDLE,          //!< CHANNEL_IDLE
  SET_PHY_TX_ON          //!< SET_PHY_TX_ON
} LrWpanMacState;

/**
 * \ingroup lr-wpan
 *
 * MAC AN (Access notification Command) states
 */
typedef enum
{
  MAC_AN_NP,                    //!< MAC_AN_NP Mac normal period state
  MAC_AN_GP,                    //!< MAC_AN_GP Mac guaranteed period state
  MAC_AN_SP,                    //!< MAC_AN_SP Mac surpressed period state
  MAC_AN_SENDING,                //!< AN sending state. 
  MAC_AN_PENDING                //!< AN pending state. An waited to be sent
} LrWpanMacAnState;

namespace TracedValueCallback {

/**
 * \ingroup lr-wpan
 * TracedValue callback signature for LrWpanMacState.
 *
 * \param [in] oldValue original value of the traced variable
 * \param [in] newValue new value of the traced variable
 */
  typedef void (* LrWpanMacState)(LrWpanMacState oldValue,
                                  LrWpanMacState newValue);

}  // namespace TracedValueCallback

/**
 * \ingroup lr-wpan
 *
 * table 80 of 802.15.4
 */
typedef enum
{
  NO_PANID_ADDR = 0,
  ADDR_MODE_RESERVED = 1,
  SHORT_ADDR = 2,
  EXT_ADDR = 3
} LrWpanAddressMode;

/**
 * \ingroup lr-wpan
 *
 * table 83 of 802.15.4
 */
typedef enum
{
  ASSOCIATED = 0,
  PAN_AT_CAPACITY = 1,
  PAN_ACCESS_DENIED = 2,
  ASSOCIATED_WITHOUT_ADDRESS = 0xfe,
  DISASSOCIATED = 0xff
} LrWpanAssociationStatus;

/**
 * \ingroup lr-wpan
 *
 * Genreal enumeration of 802.15.4 MAC. Talbe 78
 */
typedef enum
{
  IEEE802154_SUCCESS                  = 0x00,
  IEEE802154_BEACON_LOSS              = 0xe0,
  IEEE802154_CHANNEL_ACCESS_FAILURE   = 0xe1,
  IEEE802154_COUNTER_ERROR            = 0xdb,
  IEEE802154_DENIED                   = 0xe2,
  IEEE802154_DISABLE_TRX_FAILURE      = 0xe3,
  IEEE802154_FRAME_TOO_LONG           = 0xe5,
  IEEE802154_IMPROPER_KEY_TYPE        = 0xdc,
  IEEE802154_IMPROPER_SECURITY_LEVEL  = 0xdd,
  IEEE802154_INVALID_ADDRESS          = 0xf5,
  IEEE802154_INVALID_GTS              = 0xe6,
  IEEE802154_INVALID_HANDLE           = 0xe7,
  IEEE802154_INVALID_INDEX            = 0xf9,
  IEEE802154_INVALID_PARAMETER        = 0xe8,
  IEEE802154_LIMIT_REACHED            = 0xfa,
  IEEE802154_NO_ACK                   = 0xe9,
  IEEE802154_NO_BEACON                = 0xea,
  IEEE802154_NO_DATA                  = 0xeb,
  IEEE802154_NO_SHORT_ADDRESS         = 0xec,
  IEEE802154_ON_TIME_TOO_LONG         = 0xf6,
  IEEE802154_OUT_OF_CAP               = 0xed,
  IEEE802154_PAN_ID_CONFLICT          = 0xee,
  IEEE802154_PAST_TIME                = 0xf7,
  IEEE802154_READ_ONLY                = 0xfb,
  IEEE802154_REALIGNMENT              = 0xef,
  IEEE802154_SCAN_IN_PROGRESS         = 0xfe,
  IEEE802154_SECURITY_ERROR           = 0xe4,
  IEEE802154_SUPERFRAME_OVERLAP       = 0xfd,
  IEEE802154_TRACKING_OFF             = 0xf8,
  IEEE802154_TRANSACTION_EXPIRED      = 0xf0,
  IEEE802154_TRANSACTION_OVERFLOW     = 0xf1,
  IEEE802154_TX_ACTIVE                = 0xf2,
  IEEE802154_UNAVAILABLE_KEY          = 0xf3,
  IEEE802154_UNSUPPORTED_ATTRIBUTE    = 0xf4,
  IEEE802154_UNSUPPORTED_LEGACY       = 0xde,
  IEEE802154_UNSUPPORTED_SECURITY     = 0xdf,
} LrWpanMcpsConfirmStatus;

/**
 * \ingroup lr-wpan
 *
 * Table 42 of 802.15.4-2006
 */
typedef enum
{
  IEEE_802_15_4_SUCCESS                = 0,
  IEEE_802_15_4_TRANSACTION_OVERFLOW   = 1,
  IEEE_802_15_4_TRANSACTION_EXPIRED    = 2,
  IEEE_802_15_4_CHANNEL_ACCESS_FAILURE = 3,
  IEEE_802_15_4_INVALID_ADDRESS        = 4,
  IEEE_802_15_4_INVALID_GTS            = 5,
  IEEE_802_15_4_NO_ACK                 = 6,
  IEEE_802_15_4_COUNTER_ERROR          = 7,
  IEEE_802_15_4_FRAME_TOO_LONG         = 8,
  IEEE_802_15_4_UNAVAILABLE_KEY        = 9,
  IEEE_802_15_4_UNSUPPORTED_SECURITY   = 10,
  IEEE_802_15_4_INVALID_PARAMETER      = 11
} LrWpanMcpsDataConfirmStatus;


/**
 * \ingroup lr-wpan
 *
 * MCPS-DATA.request params. See 7.1.1.1
 */
struct McpsDataRequestParams
{
  McpsDataRequestParams ()
    : m_srcAddrMode (SHORT_ADDR),
      m_dstAddrMode (SHORT_ADDR),
      m_dstPanId (0),
      m_dstAddr (),
      m_msduHandle (0),
      m_txOptions (0)
  {
  }
  LrWpanAddressMode m_srcAddrMode; //!< Source address mode
  LrWpanAddressMode m_dstAddrMode; //!< Destination address mode
  uint16_t m_dstPanId;             //!< Destination PAN identifier
  Mac16Address m_dstAddr;          //!< Destination address
  Mac64Address m_dstExtAddr;       //!< Destination extended address
  uint8_t m_msduHandle;            //!< MSDU handle
  uint8_t m_txOptions;             //!< Tx Options (bitfield)
};

/**
 * \ingroup lr-wpan
 *
 * MCPS-AN.request params. See Xiao Wang's PhD thesis proposal
 */
struct McpsAnRequestParams
{
  McpsAnRequestParams ()
    : m_GpExpire (0),
      m_SPF (0)
  {
  }
  uint16_t m_GpExpire;             //!< Guaranteed period expire time -- in symbols
  uint16_t m_SPF;                   //!< Surpressed period field -- per 64 symbols
};

/**
 * \ingroup lr-wpan
 *
 * AN request processing data, help to process the state machine. See Xiao Wang's PhD thesis proposal
 */
struct McpsAnRequestProcessData
{
  // the Packet isn't included, only forward declared, thus 
  /*orward-declaring a class is a promise to the compiler to provide a definition at some later point. 
  In return the compiler lets you use your forward-declared class in other declarations, 
  where the content of the class is not required, such as declaring a pointer or a reference.
  */
  // McpsAnRequestProcessData ()
  //   : m_AN(),
  //     m_tx (0),
  //     startTime()
  // {
  // }
  struct McpsAnRequestParams m_AN; //!< AN parameter
  Ptr<Packet> m_tx;         //!< the packet that is being sent
  ns3::Time startTime;      //!< record when the MCPS-AP.request is called to recaculate GFP
  ns3::Time GpExpireTime;   //!< record when the MCPS-AP.request is called to recaculate GFP
};


/**
 * \ingroup lr-wpan
 *
 * MCPS-DATA.confirm params. See 7.1.1.2
 */
struct McpsDataConfirmParams
{
  uint8_t m_msduHandle; //!< MSDU handle
  LrWpanMcpsDataConfirmStatus m_status; //!< The status of the last MSDU transmission
};

/**
 * \ingroup lr-wpan
 *
 * MCPS-AN.confirm params. See Xiao Wang's proposal
 */
struct McpsAnConfirmParams
{
  LrWpanMcpsConfirmStatus m_status; //!< The status of the last MSDU transmission
};

/**
 * \ingroup lr-wpan
 *
 * MCPS-DATA.indication params. See 7.1.1.3
 */
struct McpsDataIndicationParams
{
  uint8_t m_srcAddrMode;  //!< Source address mode
  uint16_t m_srcPanId;    //!< Source PAN identifier
  Mac16Address m_srcAddr; //!< Source address
  Mac64Address m_srcExtAddr;  //!< Source extended address
  uint8_t m_dstAddrMode;  //!< Destination address mode
  uint16_t m_dstPanId;    //!< Destination PAN identifier
  Mac16Address m_dstAddr; //!< Destination address
  Mac64Address m_dstExtAddr;  //!< Destination extended address
  uint8_t m_mpduLinkQuality;  //!< LQI value measured during reception of the MPDU
  uint8_t m_dsn;          //!< The DSN of the received data frame
};

/**
 * \ingroup lr-wpan
 *
 * This callback is called after a McpsDataRequest has been called from
 * the higher layer.  It returns a status of the outcome of the
 * transmission request
 */
typedef Callback<void, McpsDataConfirmParams> McpsDataConfirmCallback;

/**
 * \ingroup lr-wpan
 *
 * This callback is called after a McpsAnRequest has been called from
 * the higher layer.  It returns a status of the outcome of the
 * AN request
 */
typedef Callback<void, McpsAnConfirmParams> McpsAnConfirmCallback;

/**
 * \ingroup lr-wpan
 *
 * This callback is called after a Mcps has successfully received a
 *  frame and wants to deliver it to the higher layer.
 *
 *  \todo for now, we do not deliver all of the parameters in section
 *  7.1.1.3.1 but just send up the packet.
 */
typedef Callback<void, McpsDataIndicationParams, Ptr<Packet> > McpsDataIndicationCallback;


/**
 * \ingroup lr-wpan
 *
 * Class that implements the LR-WPAN Mac state machine
 */
class LrWpanMac : public Object
{
public:
  /**
   * Get the type ID.
   *
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * The minimum number of octets added by the MAC sublayer to the PSDU.
   * See IEEE 802.15.4-2006, section 7.4.1, Table 85
   */
  static const uint32_t aMinMPDUOverhead;

  /**
   * Default constructor.
   */
  LrWpanMac (void);
  virtual ~LrWpanMac (void);

  /**
   * Check if the receiver will be enabled when the MAC is idle.
   *
   * \return true, if the receiver is enabled during idle periods, false otherwise
   */
  bool GetRxOnWhenIdle (void);

  /**
   * Set if the receiver should be enabled when the MAC is idle.
   *
   * \param rxOnWhenIdle set to true to enable the receiver during idle periods
   */
  void SetRxOnWhenIdle (bool rxOnWhenIdle);

  // XXX these setters will become obsolete if we use the attribute system
  /**
   * Set the short address of this MAC.
   *
   * \param address the new address
   */
  void SetShortAddress (Mac16Address address);

  /**
   * Get the short address of this MAC.
   *
   * \return the short address
   */
  Mac16Address GetShortAddress (void) const;

  /**
   * Set the extended address of this MAC.
   *
   * \param address the new address
   */
  void SetExtendedAddress (Mac64Address address);

  /**
   * Get the extended address of this MAC.
   *
   * \return the extended address
   */
  Mac64Address GetExtendedAddress (void) const;

  /**
   * Set the PAN id used by this MAC.
   *
   * \param panId the new PAN id.
   */
  void SetPanId (uint16_t panId);

  /**
   * Get the PAN id used by this MAC.
   *
   * \return the PAN id.
   */
  uint16_t GetPanId (void) const;

  /**
   *  IEEE 802.15.4-2006, section 7.1.1.1
   *  MCPS-DATA.request
   *  Request to transfer a MSDU.
   *
   *  \param params the request parameters
   *  \param p the packet to be transmitted
   */
  void McpsDataRequest (McpsDataRequestParams params, Ptr<Packet> p);
 
 /**
   *  IEEE 802.15.4-2006, section 7.1.1.1
   *  MCPS-AN.request
   *  Request to Access Notification command, proposed by Xiao Wang.
   *
   *  \param params the request parameters
   */
  void McpsAnRequest (McpsAnRequestParams params);

  /**
   * Set the CSMA/CA implementation to be used by the MAC.
   *
   * \param csmaCa the CSMA/CA implementation
   */
  void SetCsmaCa (Ptr<LrWpanCsmaCa> csmaCa);

  /**
   * Set the underlying PHY for the MAC.
   *
   * \param phy the PHY
   */
  void SetPhy (Ptr<LrWpanPhy> phy);

  /**
   * Get the underlying PHY of the MAC.
   *
   * \return the PHY
   */
  Ptr<LrWpanPhy> GetPhy (void);

  /**
   * Set the callback for the indication of an incoming data packet.
   * The callback implements MCPS-DATA.indication SAP of IEEE 802.15.4-2006,
   * section 7.1.1.3.
   *
   * \param c the callback
   */
  void SetMcpsDataIndicationCallback (McpsDataIndicationCallback c);

  /**
   * Set the callback for the confirmation of a data transmission request.
   * The callback implements MCPS-DATA.confirm SAP of IEEE 802.15.4-2006,
   * section 7.1.1.2.
   *
   * \param c the callback
   */
  void SetMcpsDataConfirmCallback (McpsDataConfirmCallback c);
  /**
   * Set the callback for the confirmation of a AN request.
   * The callback implements MCPS-AN.confirm SAP, proposed by Xiao Wang
   *
   * \param c the callback
   */
  void SetMcpsAnConfirmCallback (McpsAnConfirmCallback c);

  // interfaces between MAC and PHY
  /**
   *  IEEE 802.15.4-2006 section 6.2.1.3
   *  PD-DATA.indication
   *  Indicates the transfer of an MPDU from PHY to MAC (receiving)
   *  @param psduLength number of bytes in the PSDU
   *  @param p the packet to be transmitted
   *  @param lqi Link quality (LQI) value measured during reception of the PPDU
   */
  void PdDataIndication (uint32_t psduLength, Ptr<Packet> p, uint8_t lqi);

  /**
   *  IEEE 802.15.4-2006 section 6.2.1.2
   *  Confirm the end of transmission of an MPDU to MAC
   *  @param status to report to MAC
   *  PHY PD-DATA.confirm status
   */
  void PdDataConfirm (LrWpanPhyEnumeration status);

  /**
   *  IEEE 802.15.4-2006 section 6.2.2.2
   *  PLME-CCA.confirm status
   *  @param status TRX_OFF, BUSY or IDLE
   */
  void PlmeCcaConfirm (LrWpanPhyEnumeration status);

  /**
   *  IEEE 802.15.4-2006 section 6.2.2.4
   *  PLME-ED.confirm status and energy level
   *  @param status SUCCESS, TRX_OFF or TX_ON
   *  @param energyLevel 0x00-0xff ED level for the channel
   */
  void PlmeEdConfirm (LrWpanPhyEnumeration status, uint8_t energyLevel);

  /**
   *  IEEE 802.15.4-2006 section 6.2.2.6
   *  PLME-GET.confirm
   *  Get attributes per definition from Table 23 in section 6.4.2
   *  @param status SUCCESS or UNSUPPORTED_ATTRIBUTE
   *  @param id the attributed identifier
   *  @param attribute the attribute value
   */
  void PlmeGetAttributeConfirm (LrWpanPhyEnumeration status,
                                LrWpanPibAttributeIdentifier id,
                                LrWpanPhyPibAttributes* attribute);

  /**
   *  IEEE 802.15.4-2006 section 6.2.2.8
   *  PLME-SET-TRX-STATE.confirm
   *  Set PHY state
   *  @param status in RX_ON,TRX_OFF,FORCE_TRX_OFF,TX_ON
   */
  void PlmeSetTRXStateConfirm (LrWpanPhyEnumeration status);

  /**
   *  IEEE 802.15.4-2006 section 6.2.2.10
   *  PLME-SET.confirm
   *  Set attributes per definition from Table 23 in section 6.4.2
   *  @param status SUCCESS, UNSUPPORTED_ATTRIBUTE, INVALID_PARAMETER, or READ_ONLY
   *  @param id the attributed identifier
   */
  void PlmeSetAttributeConfirm (LrWpanPhyEnumeration status,
                                LrWpanPibAttributeIdentifier id);

  /**
   * CSMA-CA algorithm calls back the MAC after executing channel assessment.
   *
   * \param macState indicate BUSY or IDLE channel condition
   */
  void SetLrWpanMacState (LrWpanMacState macState);

  /**
   * Get the current association status.
   *
   * \return current association status
   */
  LrWpanAssociationStatus GetAssociationStatus (void) const;

  /**
   * Set the current association status.
   *
   * \param status new association status
   */
  void SetAssociationStatus (LrWpanAssociationStatus status);

  //MAC sublayer constants
  /**
   * Length of a superframe slot in symbols. Defaults to 60 symbols in each
   * superframe slot.
   * See IEEE 802.15.4-2006, section 7.4.1, Table 85.
   */
  uint64_t m_aBaseSlotDuration;

  /**
   * Number of a superframe slots per superframe. Defaults to 16.
   * See IEEE 802.15.4-2006, section 7.4.1, Table 85.
   */
  uint64_t m_aNumSuperframeSlots;

  /**
   * Length of a superframe in symbols. Defaults to
   * aBaseSlotDuration * aNumSuperframeSlots in symbols.
   * See IEEE 802.15.4-2006, section 7.4.1, Table 85.
   */
  uint64_t m_aBaseSuperframeDuration;

  //MAC PIB attributes
  /**
   * The time that the device transmitted its last beacon frame, in symbol
   * periods. Only 24 bits used.
   * See IEEE 802.15.4-2006, section 7.4.2, Table 86.
   */
  uint64_t m_macBeaconTxTime;

  /**
   * Symbol boundary is same as m_macBeaconTxTime.
   * See IEEE 802.15.4-2006, section 7.4.2, Table 86.
   */
  uint64_t m_macSyncSymbolOffset;

  /**
   * Specification of how often the coordinator transmits its beacon.
   * 0 - 15 with 15 means no beacons are being sent.
   * See IEEE 802.15.4-2006, section 7.4.2, Table 86.
   */
  uint64_t m_macBeaconOrder;

  /**
   * The length of the active portion of the outgoing superframe, including the
   * beacon frame.
   * 0 - 15 with 15 means the superframe will not be active after the beacon.
   * See IEEE 802.15.4-2006, section 7.4.2, Table 86.
   */
  uint64_t m_macSuperframeOrder;

  /**
   * Indicates if MAC sublayer is in receive all mode. True mean accept all
   * frames from PHY.
   * See IEEE 802.15.4-2006, section 7.4.2, Table 86.
   */
  bool m_macPromiscuousMode;

  /**
   * 16 bits id of PAN on which this device is operating. 0xffff means not
   * asscoiated.
   * See IEEE 802.15.4-2006, section 7.4.2, Table 86.
   */
  uint16_t m_macPanId;


  /**
   * Attrube for enable Access Notification or not
   * See Xiao Wang's PhD thesis proposal of Access notification for co-existing with wifi.
   */
  bool m_macEnableAN;

  /**
   * Sequence number added to transmitted data or MAC command frame, 00-ff.
   * See IEEE 802.15.4-2006, section 7.4.2, Table 86.
   */
  SequenceNumber8 m_macDsn;

  /**
   * The maximum number of retries allowed after a transmission failure.
   * See IEEE 802.15.4-2006, section 7.4.2, Table 86.
   */
  uint8_t m_macMaxFrameRetries;

  /**
   * Indication of whether the MAC sublayer is to enable its receiver during
   * idle periods.
   * See IEEE 802.15.4-2006, section 7.4.2, Table 86.
   */
  bool m_macRxOnWhenIdle;

  /**
   * Get the macAckWaitDuration attribute value.
   *
   * \return the maximum number symbols to wait for an acknowledgment frame
   */
  uint64_t GetMacAckWaitDuration (void) const;

  /**
   * Get the macMaxFrameRetries attribute value.
   *
   * \return the maximum number of retries
   */
  uint8_t GetMacMaxFrameRetries (void) const;

  /**
   * Set the macMaxFrameRetries attribute value.
   *
   * \param retries the maximum number of retries
   */
  void SetMacMaxFrameRetries (uint8_t retries);

  /**
   * TracedCallback signature for sent packets.
   *
   * \param [in] packet The packet.
   * \param [in] retries The number of retries.
   * \param [in] backoffs The number of CSMA backoffs.
   */
  typedef void (* SentTracedCallback)
    (Ptr<const Packet> packet, uint8_t retries, uint8_t backoffs);

  /**
   * TracedCallback signature for LrWpanMacState change events.
   *
   * \param [in] oldValue The original state value.
   * \param [in] newValue The new state value.
   * \deprecated The LrWpanMacState is now accessible as the
   * TracedValue \c MacStateValue. The \c MacState TracedCallback will
   * be removed in a future release.
   */
  typedef void (* StateTracedCallback)
    (LrWpanMacState oldState, LrWpanMacState newState);
  
protected:
  // Inherited from Object.
  virtual void DoInitialize (void);
  virtual void DoDispose (void);

private:
  /**
   * Helper structure for managing transmission queue elements.
   */
  struct TxQueueElement
  {
    uint8_t txQMsduHandle; //!< MSDU Handle
    Ptr<Packet> txQPkt;    //!< Queued packet
  };

  /**
   * Send an acknowledgment packet for the given sequence number.
   *
   * \param seqno the sequence number for the ACK
   */
  void SendAck (uint8_t seqno);

  /**
   * Remove the tip of the transmission queue, including clean up related to the
   * last packet transmission.
   */
  void RemoveFirstTxQElement ();

  /**
   * Change the current MAC state to the given new state.
   *
   * \param newState the new state
   */
  void ChangeMacState (LrWpanMacState newState);

  /**
   * Handle an ACK timeout with a packet retransmission, if there are
   * retransmission left, or a packet drop.
   */
  void AckWaitTimeout (void);

  /**
   * Check for remaining retransmissions for the packet currently being sent.
   * Drop the packet, if there are no retransmissions left.
   *
   * \return true, if the packet should be retransmitted, false otherwise.
   */
  bool PrepareRetransmission (void);

  /**
   * Check the transmission queue. If there are packets in the transmission
   * queue and the MAC is idle, pick the first one and initiate a packet
   * transmission.
   */
  void CheckQueue (void);

  /**
   *get the time duration by Symbol Numbers. 
   * 
   */
  inline Time SymbolToTime(uint32_t symbolsNumber);
  
  /**
   *get Symbol Numbers by the time duration, round up method. 
   * 
   */
  inline uint32_t TimeToSymbol(Time time);

  /**
   *get the time duration of AN frame in Symbols, including SHR PHR pluse given psdusize
   * 
   */
  inline uint32_t GetFrameDuration(uint32_t psduSize);
  
  /**
   *get the time duration of AN frame in Symbols, including CCA, RX2TX, SHR, PHR pluse given psdusize
   * 
   */
  inline uint32_t GetSendingDurationMin(uint32_t psduSize);
  /**
   * assemble a An Packet according to current time and given parameter
   * 
   */
  Ptr<Packet> AnPacketAssemble(void);

  /**
   * AN GP and SP manage
   *
   */
  void ChangeWpanMacAnState (LrWpanMacAnState anState);
  /**
   * The trace source fired when packets are considered as successfully sent
   * or the transmission has been given up.
   * Only non-broadcast packets are traced.
   *
   * The data should represent:
   * packet, number of retries, total number of csma backoffs
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet>, uint8_t, uint8_t > m_sentPktTrace;

  /**
   * The trace source fired when packets come into the "top" of the device
   * at the L3/L2 transition, when being queued for transmission.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxEnqueueTrace;

  /**
   * The trace source fired when packets are dequeued from the
   * L3/l2 transmission queue.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxDequeueTrace;

  /**
   * The trace source fired when packets are being sent down to L1.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxTrace;

  /**
   * The trace source fired when packets where successfully transmitted, that is
   * an acknowledgment was received, if requested, or the packet was
   * successfully sent by L1, if no ACK was requested.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxOkTrace;

  /**
   * The trace source fired when packets are dropped due to missing ACKs or
   * because of transmission failures in L1.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxDropTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3
   * transition).  This is a promiscuous trace.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macPromiscRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3
   * transition).  This is a non-promiscuous trace.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * but dropped before being forwarded up to higher layers (at the L2/L3
   * transition).
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macRxDropTrace;

  /**
   * A trace source that emulates a non-promiscuous protocol sniffer connected
   * to the device.  Unlike your average everyday sniffer, this trace source
   * will not fire on PACKET_OTHERHOST events.
   *
   * On the transmit size, this trace hook will fire after a packet is dequeued
   * from the device queue for transmission.  In Linux, for example, this would
   * correspond to the point just before a device hard_start_xmit where
   * dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET
   * ETH_P_ALL handlers.
   *
   * On the receive side, this trace hook will fire when a packet is received,
   * just before the receive callback is executed.  In Linux, for example,
   * this would correspond to the point at which the packet is dispatched to
   * packet sniffers in netif_receive_skb.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_snifferTrace;

  /**
   * A trace source that emulates a promiscuous mode protocol sniffer connected
   * to the device.  This trace source fire on packets destined for any host
   * just like your average everyday packet sniffer.
   *
   * On the transmit size, this trace hook will fire after a packet is dequeued
   * from the device queue for transmission.  In Linux, for example, this would
   * correspond to the point just before a device hard_start_xmit where
   * dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET
   * ETH_P_ALL handlers.
   *
   * On the receive side, this trace hook will fire when a packet is received,
   * just before the receive callback is executed.  In Linux, for example,
   * this would correspond to the point at which the packet is dispatched to
   * packet sniffers in netif_receive_skb.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_promiscSnifferTrace;

  /**
   * A trace source that fires when the LrWpanMac changes states.
   * Parameters are the old mac state and the new mac state.
   *
   * \deprecated This TracedCallback is deprecated and will be
   * removed in a future release,  Instead, use the \c MacStateValue
   * TracedValue.
   */
  TracedCallback<LrWpanMacState, LrWpanMacState> m_macStateLogger;

  /**
   * The PHY associated with this MAC.
   */
  Ptr<LrWpanPhy> m_phy;

  /**
   * The CSMA/CA implementation used by this MAC.
   */
  Ptr<LrWpanCsmaCa> m_csmaCa;

  /**
   * This callback is used to notify incoming packets to the upper layers.
   * See IEEE 802.15.4-2006, section 7.1.1.3.
   */
  McpsDataIndicationCallback m_mcpsDataIndicationCallback;

  /**
   * This callback is used to report data transmission request status to the
   * upper layers.
   * See IEEE 802.15.4-2006, section 7.1.1.2.
   */
  McpsDataConfirmCallback m_mcpsDataConfirmCallback;

  /**
   * This callback is used to report Access Notification request status
   * upper layers.
   * See Xiao Wang' PhD thesis proposal
   */
  McpsAnConfirmCallback m_mcpsAnConfirmCallback;

  /**
   * The current state of the MAC layer.
   */
  TracedValue<LrWpanMacState> m_lrWpanMacState;

  /**
   * The current An Command state of the MAC layer.
   */
  TracedValue<LrWpanMacAnState> m_lrWpanMacAnState;

  /**
   * The current association status of the MAC layer.
   */
  LrWpanAssociationStatus m_associationStatus;

  /**
   * The packet which is currently being sent by the MAC layer.
   */
  Ptr<Packet> m_txPkt;  // XXX need packet buffer instead of single packet

  /**
   * The AN command processing data which is currently being sent by the MAC layer.
   */
  struct McpsAnRequestProcessData m_anProcessData; 
  
  /**
   * The AN command minimum GP period.
   */
  ns3::Time m_minGP; 

  /**
   * The short address used by this MAC. Currently we do not have complete
   * extended address support in the MAC, nor do we have the association
   * primitives, so this address has to be configured manually.
   */
  Mac16Address m_shortAddress;

  /**
   * The extended address used by this MAC. Extended addresses are currently not
   * really supported.
   */
  Mac64Address m_selfExt;

  /**
   * The transmit queue used by the MAC.
   */
  std::deque<TxQueueElement*> m_txQueue;

  /**
   * The number of already used retransmission for the currently transmitted
   * packet.
   */
  uint8_t m_retransmission;

  /**
   * The number of CSMA/CA retries used for sending the current packet.
   */
  uint8_t m_numCsmacaRetry;

  /**
   * Scheduler event for the ACK timeout of the currently transmitted data
   * packet.
   */
  EventId m_ackWaitTimeout;

  /**
   * Scheduler event for a deferred MAC state change.
   */
  EventId m_setMacState;
  /**
   * Scheduler event for a MAC AN state change.
   */
  EventId m_macAnStateEvent;
};


} // namespace ns3

#endif /* LR_WPAN_MAC_H */
