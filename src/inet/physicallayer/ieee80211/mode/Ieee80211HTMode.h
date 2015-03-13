//
// Copyright (C) 2015 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IEEE80211HTMODE_H
#define __INET_IEEE80211HTMODE_H

#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"
#include "inet/physicallayer/ieee80211/Ieee80211HTCode.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211HTTimingRelatedParametersBase
{
    public:
        const simtime_t getDFTPeriod() const { return 3.2E-6; } // DFT
        const simtime_t getGIDuration() const { return getDFTPeriod() / 4; } // GI
        const simtime_t getShortGIDuration() const { return getDFTPeriod() / 8; } // GIS
        const simtime_t getSymbolInterval() const { return getDFTPeriod() + getGIDuration(); } // SYM
        const simtime_t getShortGISymbolInterval() const { return getDFTPeriod() + getShortGIDuration(); } // SYMS
};

class INET_API Ieee80211HTModeBase
{
    public:
        enum GuardIntervalType
        {
            HT_GUARD_INTERVAL_SHORT, // 400 ns
            HT_GUARD_INTERVAL_LONG // 800 ns
        };

    protected:
        const Hz bandwidth;
        const GuardIntervalType guardIntervalType;
        const unsigned int modulationAndCodingScheme; // MCS
        const unsigned int numberOfSpatialStreams; // N_SS

        mutable bps netBitrate; // cached
        mutable bps grossBitrate; // cached

    protected:
        virtual bps computeGrossBitrate() const = 0;
        virtual bps computeNetBitrate() const = 0;

    public:
        Ieee80211HTModeBase(unsigned int modulationAndCodingScheme, unsigned int numberOfSpatialStreams, const Hz bandwidth, GuardIntervalType guardIntervalType);

        int getNumberOfDataSubcarriers() const;
        int getNumberOfPilotSubcarriers() const;
        int getNumberOfTotalSubcarriers() const { return getNumberOfDataSubcarriers() + getNumberOfPilotSubcarriers(); }
        const GuardIntervalType getGuardIntervalType() const { return guardIntervalType; }
        const unsigned int getNumberOfSpatialStreams() const { return numberOfSpatialStreams; }
        unsigned int getModulationAndCodingScheme() const { return modulationAndCodingScheme; }

        Hz getBandwidth() const { return bandwidth; }
        bps getNetBitrate() const;
        bps getGrossBitrate() const;

};

class INET_API Ieee80211HTSignalMode : public IIeee80211HeaderMode, public Ieee80211HTModeBase, public Ieee80211HTTimingRelatedParametersBase
{
    protected:
        const Ieee80211HTCode *code;
        const Ieee80211OFDMModulation *modulation;

    protected:
        bps computeGrossBitrate() const override;
        bps computeNetBitrate() const override;

    public:
        Ieee80211HTSignalMode(unsigned int modulationAndCodingScheme, const Hz bandwidth, GuardIntervalType guardIntervalType);

        /* Table 20-11—HT-SIG fields, 1699p */

        // HT-SIG_1 (24 bits)
        inline int getMCSLength() const { return 7; }
        inline int getCBWLength() const { return 1; }
        inline int getHTLengthLength() const { return 16; }

        // HT-SIG_2 (24 bits)
        inline int getSmoothingLength() const { return 1; }
        inline int getNotSoundingLength() const { return 1; }
        inline int getReservedLength() const { return 1; }
        inline int getAggregationLength() const { return 1; }
        inline int getSTBCLength() const { return 2; }
        inline int getFECCodingLength() const { return 1; }
        inline int getShortGILength() const { return 1; }
        inline int getNumOfExtensionSpatialStreamsLength() const { return 2; }
        inline int getCRCLength() const { return 8; }
        inline int getTailBitsLength() const { return 6; }
        unsigned int getSTBC() const { return 0; } // Limitation: We assume that STBC is not used

        const inline simtime_t getHTSIGDuration() const { return 2 * getSymbolInterval(); } // HT-SIG

        unsigned int getModulationAndCodingScheme() const { return modulationAndCodingScheme; }
        virtual const simtime_t getDuration() const override { return getHTSIGDuration(); }
        virtual int getBitLength() const override;
};

/*
 * The HT preambles are defined in HT-mixed format and in HT-greenfield format to carry the required
 * information to operate in a system with multiple transmit and multiple receive antennas. (20.3.9 HT preamble)
 */
class INET_API Ieee80211HTPreambleMode : public IIeee80211PreambleMode, public Ieee80211HTTimingRelatedParametersBase
{
    public:
        enum HighTroughputPreambleFormat
        {
            HT_PREAMBLE_MIXED,      // can be received by non-HT STAs compliant with Clause 18 or Clause 19
            HT_PREAMBLE_GREENFIELD  // all of the non-HT fields are omitted
        };

    protected:
        const Ieee80211HTSignalMode *highThroughputSignalMode; // In HT-terminology the HT-SIG (signal field) and L-SIG are part of the preamble
        const Ieee80211OFDMSignalMode *legacySignalMode; // L-SIG
        const HighTroughputPreambleFormat preambleFormat;
        const unsigned int numberOfHTLongTrainings; // N_LTF, 20.3.9.4.6 HT-LTF definition

    protected:
        unsigned int computeNumberOfSpaceTimeStreams(unsigned int numberOfSpatialStreams) const;
        unsigned int computeNumberOfHTLongTrainings(unsigned int numberOfSpaceTimeStreams) const;

    public:
        Ieee80211HTPreambleMode(const Ieee80211HTSignalMode* highThroughputSignalMode, const Ieee80211OFDMSignalMode *legacySignalMode, HighTroughputPreambleFormat preambleFormat, unsigned int numberOfSpatialStream);

        HighTroughputPreambleFormat getPreambleFormat() const { return preambleFormat; }
        const Ieee80211HTSignalMode *getSignalMode() const { return highThroughputSignalMode; }
        const Ieee80211OFDMSignalMode *getLegacySignalMode() const { return legacySignalMode; }
        const Ieee80211HTSignalMode* getHighThroughputSignalMode() const { return highThroughputSignalMode; }
        inline unsigned int getNumberOfHTLongTrainings() const { return numberOfHTLongTrainings; }

        const inline simtime_t getDoubleGIDuration() const { return 2 * getGIDuration(); } // GI2
        const inline simtime_t getLSIGDuration() const { return getSymbolInterval(); } // L-SIG
        const inline simtime_t getNonHTShortTrainingSequenceDuration() const { return 10 * getDFTPeriod() / 4;  } // L-STF
        const inline simtime_t getHTGreenfieldShortTrainingFieldDuration() const { return 10 * getDFTPeriod() / 4; } // HT-GF-STF
        const inline simtime_t getNonHTLongTrainingFieldDuration() const { return 2 * getDFTPeriod() + getDoubleGIDuration(); } // L-LTF
        const inline simtime_t getHTShortTrainingFieldDuration() const { return 4E-6; } // HT-STF
        const simtime_t getFirstHTLongTrainingFieldDuration() const;
        const inline simtime_t getSecondAndSubsequentHTLongTrainingFielDuration() const { return 4E-6; } // HT-LTFs, s = 2,3,..,n
        const inline unsigned int getNumberOfHtLongTrainings() const { return numberOfHTLongTrainings; }

        virtual const simtime_t getDuration() const override;

};

class INET_API Ieee80211HTDataMode : public IIeee80211DataMode, public Ieee80211HTModeBase, public Ieee80211HTTimingRelatedParametersBase
{
    protected:
        const Ieee80211HTCode *code;
        const Ieee80211OFDMModulation *stream1Modulation;
        const Ieee80211OFDMModulation *stream2Modulation;
        const Ieee80211OFDMModulation *stream3Modulation;
        const Ieee80211OFDMModulation *stream4Modulation;

        unsigned int numberOfBCCEncoders;

    protected:
        bps computeGrossBitrate() const override;
        bps computeNetBitrate() const override;
        unsigned int computeNumberOfSpatialStreams(const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation) const;

    public:
        Ieee80211HTDataMode(unsigned int modulationAndCodingScheme, unsigned int numberOfBCCEncoders, const Ieee80211HTCode *code, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211OFDMModulation *stream2Modulation, const Ieee80211OFDMModulation *stream3Modulation, const Ieee80211OFDMModulation *stream4Modulation, const Hz bandwidth, GuardIntervalType guardIntervalType);

        const Ieee80211HTCode* getCode() const { return code; }
        const Ieee80211OFDMModulation* getStream1Modulation() const { return stream1Modulation; }
        const Ieee80211OFDMModulation* getStream2Modulation() const { return stream2Modulation; }
        const Ieee80211OFDMModulation* getStream3Modulation() const { return stream3Modulation; }
        const Ieee80211OFDMModulation* getStream4Modulation() const { return stream4Modulation; }

        inline int getServiceBitLength() const { return 16; }
        inline int getTailBitLength() const { return 6 * numberOfBCCEncoders; }
        inline unsigned int getNumberOfBccEncoders() const { return numberOfBCCEncoders; }

        virtual int getBitLength(int dataBitLength) const override;
        virtual const simtime_t getDuration(int dataBitLength) const override;

};

class INET_API Ieee80211HTMode : public IIeee80211Mode
{
    protected:
        const Ieee80211HTPreambleMode *preambleMode;
        const Ieee80211HTDataMode *dataMode;
        const Hz carrierFrequency; // TODO: revise

    public:
        Ieee80211HTMode(const Ieee80211HTPreambleMode *preambleMode, const Ieee80211HTDataMode *dataMode, const Hz carrierFrequency);

        const Ieee80211HTDataMode* getDataMode() const override { return dataMode; }
        const Ieee80211HTPreambleMode* getPreambleMode() const override { return preambleMode; }
        const Ieee80211HTSignalMode *getHeaderMode() const override { return preambleMode->getSignalMode(); }
        const Ieee80211OFDMSignalMode *getLegacySignalMode() const { return preambleMode->getLegacySignalMode(); }

        // Table 20-25—MIMO PHY characteristics
        virtual inline const simtime_t getRifsTime() const { return 2E-6; }
        virtual const simtime_t getSlotTime() const override;
        virtual const simtime_t getShortSlotTime() const;
        virtual inline const simtime_t getSifsTime() const override;
        virtual inline const simtime_t getCcaTime() const override { return 4E-6; } // < 4
        virtual inline const simtime_t getPhyRxStartDelay() const override { return 33E-6; }
        virtual inline const simtime_t getRxTxTurnaroundTime() const override { return 2E-6; } // < 2
        virtual inline const simtime_t getPreambleLength() const override { return 16E-6; }
        virtual inline const simtime_t getPlcpHeaderLength() const override { return 4E-6; }
        virtual inline int getCwMin() const override { return 15; }
        virtual inline int getCwMax() const override { return 1023; }
        virtual inline int getMpduMaxLength() const override { return 65535; } // in octets
        const Hz getCarrierFrequency() const { return carrierFrequency; }

        virtual const simtime_t getDuration(int dataBitLength) const override { return preambleMode->getDuration() + dataMode->getDuration(dataBitLength); }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211HTMODE_H
