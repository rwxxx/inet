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

#include "inet/physicallayer/ieee80211/mode/Ieee80211HTMode.h"

namespace inet {
namespace physicallayer {

Ieee80211HTMode::Ieee80211HTMode(const Ieee80211HTPreambleMode* preambleMode, const Ieee80211HTDataMode* dataMode, const Hz carrierFrequency) :
        preambleMode(preambleMode),
        dataMode(dataMode),
        carrierFrequency(carrierFrequency)
{
}

Ieee80211HTModeBase::Ieee80211HTModeBase(unsigned int modulationAndCodingScheme, unsigned int numberOfSpatialStreams, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        bandwidth(bandwidth),
        guardIntervalType(guardIntervalType),
        modulationAndCodingScheme(modulationAndCodingScheme),
        numberOfSpatialStreams(numberOfSpatialStreams),
        netBitrate(bps(NaN)),
        grossBitrate(bps(NaN))
{
}

Ieee80211HTPreambleMode::Ieee80211HTPreambleMode(const Ieee80211HTSignalMode* highThroughputSignalMode, const Ieee80211OFDMSignalMode *legacySignalMode, HighTroughputPreambleFormat preambleFormat, unsigned int numberOfSpatialStream) :
        highThroughputSignalMode(highThroughputSignalMode),
        legacySignalMode(legacySignalMode),
        preambleFormat(preambleFormat),
        numberOfHTLongTrainings(computeNumberOfHTLongTrainings(computeNumberOfSpaceTimeStreams(numberOfSpatialStream)))
{
}

Ieee80211HTSignalMode::Ieee80211HTSignalMode(unsigned int modulationAndCodingScheme, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        Ieee80211HTModeBase(modulationAndCodingScheme, 1, bandwidth, guardIntervalType) // TODO: Should HT signal mode support spatial extensions in non-compliant mode?
{
}

Ieee80211HTDataMode::Ieee80211HTDataMode(unsigned int modulationAndCodingScheme, unsigned int numberOfBCCEncoders, const Ieee80211HTDataCode* code, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        Ieee80211HTModeBase(modulationAndCodingScheme, computeNumberOfSpatialStreams(stream1Modulation,stream2Modulation, stream3Modulation, stream4Modulation), bandwidth, guardIntervalType),
        code(code),
        stream1Modulation(stream1Modulation),
        stream2Modulation(stream2Modulation),
        stream3Modulation(stream3Modulation),
        stream4Modulation(stream4Modulation),
        numberOfBCCEncoders(numberOfBCCEncoders)
{
}

bps Ieee80211HTSignalMode::computeGrossBitrate() const
{
    unsigned int numberOfCodedBitsPerSymbol = modulation->getSubcarrierModulation()->getCodeWordSize() * getNumberOfDataSubcarriers();
    if (guardIntervalType == HT_GUARD_INTERVAL_LONG)
        return bps(numberOfCodedBitsPerSymbol / getSymbolInterval());
    else if (guardIntervalType == HT_GUARD_INTERVAL_SHORT)
        return bps(numberOfCodedBitsPerSymbol / getShortGISymbolInterval());
    else
        throw cRuntimeError("Unknown guard interval type");
}

bps Ieee80211HTSignalMode::computeNetBitrate() const
{
    return computeGrossBitrate() * code->getForwardErrorCorrection()->getCodeRate();
}

bps Ieee80211HTDataMode::computeGrossBitrate() const
{
    unsigned int numberOfCodedBitsPerSubcarrierSum = stream1Modulation ? stream1Modulation->getSubcarrierModulation()->getCodeWordSize() : 0 +
                                                     stream2Modulation ? stream2Modulation->getSubcarrierModulation()->getCodeWordSize() : 0 +
                                                     stream3Modulation ? stream3Modulation->getSubcarrierModulation()->getCodeWordSize() : 0 +
                                                     stream4Modulation ? stream4Modulation->getSubcarrierModulation()->getCodeWordSize() : 0;
    unsigned int numberOfCodedBitsPerSymbol = numberOfCodedBitsPerSubcarrierSum * getNumberOfDataSubcarriers();
    if (guardIntervalType == HT_GUARD_INTERVAL_LONG)
        return bps(numberOfCodedBitsPerSymbol / getSymbolInterval());
    else if (guardIntervalType == HT_GUARD_INTERVAL_SHORT)
        return bps(numberOfCodedBitsPerSymbol / getShortGISymbolInterval());
    else
        throw cRuntimeError("Unknown guard interval type");
}

bps Ieee80211HTDataMode::computeNetBitrate() const
{
    return computeGrossBitrate() * code->getForwardErrorCorrection()->getCodeRate();
}

bps Ieee80211HTModeBase::getNetBitrate() const
{
    if (isNaN(netBitrate.get()))
        netBitrate = computeNetBitrate();
    return netBitrate;
}

bps Ieee80211HTModeBase::getGrossBitrate() const
{
    if (isNaN(grossBitrate.get()))
        grossBitrate = computeGrossBitrate();
    return grossBitrate;
}

int Ieee80211HTSignalMode::getBitLength() const
{
    return
        getMCSLength() +
        getCBWLength() +
        getHTLengthLength() +
        getSmoothingLength() +
        getNotSoundingLength() +
        getReservedLength() +
        getAggregationLength() +
        getSTBCLength() +
        getFECCodingLength() +
        getShortGILength() +
        getNumOfExtensionSpatialStreamsLength() +
        getCRCLength() +
        getTailBitsLength();
}

unsigned int Ieee80211HTPreambleMode::computeNumberOfSpaceTimeStreams(unsigned int numberOfSpatialStreams) const
{
    // Table 20-12—Determining the number of space-time streams
    return numberOfSpatialStreams + highThroughputSignalMode->getSTBC();
}

unsigned int Ieee80211HTPreambleMode::computeNumberOfHTLongTrainings(unsigned int numberOfSpaceTimeStreams) const
{
    // If the transmitter is providing training for exactly the space-time
    // streams (spatial mapper inputs) used for the transmission of the PSDU,
    // the number of training symbols, N_LTF, is equal to the number of space-time
    // streams, N STS, except that for three space-time streams, four training symbols
    // are required.
    return numberOfSpaceTimeStreams == 3 ? 4 : numberOfSpaceTimeStreams;
}

const simtime_t Ieee80211HTPreambleMode::getDuration() const
{
    // 20.3.7 Mathematical description of signals
    simtime_t sumOfHTLTFs = getFirstHTLongTrainingFieldDuration() + getSecondAndSubsequentHTLongTrainingFielDuration() * (numberOfHTLongTrainings - 1);
    if (preambleFormat == HT_PREAMBLE_MIXED)
        // L-STF -> L-LTF -> L-SIG -> HT-SIG -> HT-STF -> HT-LTF1 -> HT-LTF2 -> ... -> HT_LTFn
        return getNonHTShortTrainingSequenceDuration() + getNonHTLongTrainingFieldDuration() + legacySignalMode->getDuration() + highThroughputSignalMode->getDuration() + getHTShortTrainingFieldDuration() + sumOfHTLTFs;
    else if (preambleFormat == HT_PREAMBLE_GREENFIELD)
        // HT-GF-STF -> HT-LTF1 -> HT-SIG -> HT-LTF2 -> ... -> HT-LTFn
        return getHTGreenfieldShortTrainingFieldDuration() + highThroughputSignalMode->getDuration() + sumOfHTLTFs;
    else
        throw cRuntimeError("Unknown preamble format");
}

int Ieee80211HTModeBase::getNumberOfDataSubcarriers() const
{
    if (bandwidth == MHz(20))
        return 52;
    else if (bandwidth == MHz(40))
        // It is a special case, see Table 20-38—MCS parameters for
        // optional 40 MHz MCS 32 format, N SS = 1, N ES = 1
        return modulationAndCodingScheme == 32 ? 48 : 108;
    else
        throw cRuntimeError("Unsupported bandwidth");
}

int Ieee80211HTModeBase::getNumberOfPilotSubcarriers() const
{
    if (bandwidth == MHz(20))
        return 4;
    else if (bandwidth == MHz(40))
        // It is a spacial case, see the comment above.
        return modulationAndCodingScheme == 32 ? 4 : 6;
    else
        throw cRuntimeError("Unsupported bandwidth");
}

const simtime_t Ieee80211HTPreambleMode::getFirstHTLongTrainingFieldDuration() const
{
    if (preambleFormat == HT_PREAMBLE_MIXED)
        return simtime_t(4E-6);
    else if (preambleFormat == HT_PREAMBLE_GREENFIELD)
        return simtime_t(8E-6);
    else
        throw cRuntimeError("Unknown preamble format");
}

int Ieee80211HTDataMode::getBitLength(int dataBitLength) const
{
    return getServiceBitLength() + getTailBitLength() + dataBitLength; // TODO: padding?
}

unsigned int Ieee80211HTDataMode::computeNumberOfSpatialStreams(const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation) const
{
    return stream1Modulation ? 1 : 0 + stream2Modulation ? 1 : 0 +
           stream3Modulation ? 1 : 0 + stream4Modulation ? 1 : 0;
}

const simtime_t Ieee80211HTDataMode::getDuration(int dataBitLength) const
{
    unsigned int numberOfCodedBitsPerSubcarrierSum = stream1Modulation ? stream1Modulation->getSubcarrierModulation()->getCodeWordSize() : 0 +
                                                         stream2Modulation ? stream2Modulation->getSubcarrierModulation()->getCodeWordSize() : 0 +
                                                         stream3Modulation ? stream3Modulation->getSubcarrierModulation()->getCodeWordSize() : 0 +
                                                         stream4Modulation ? stream4Modulation->getSubcarrierModulation()->getCodeWordSize() : 0;
    unsigned int numberOfCodedBitsPerSymbol = numberOfCodedBitsPerSubcarrierSum * getNumberOfDataSubcarriers();
    const IForwardErrorCorrection *forwardErrorCorrection = code ? code->getForwardErrorCorrection() : nullptr;
    unsigned int dataBitsPerSymbol = forwardErrorCorrection ? forwardErrorCorrection->getDecodedLength(numberOfCodedBitsPerSymbol) : numberOfCodedBitsPerSymbol;
    int numberOfSymbols = lrint(ceil((double)getBitLength(dataBitLength) / dataBitsPerSymbol));
    return numberOfSymbols * getSymbolInterval();
}

const simtime_t Ieee80211HTMode::getSlotTime() const
{
    if (carrierFrequency == Hz(2400))
        return 20E-6;
    else if (carrierFrequency == Hz(5000))
        return 9E-6;
    else
        throw cRuntimeError("Unsupported carrier frequency");
}

inline const simtime_t Ieee80211HTMode::getSifsTime() const
{
    // 2.4: 10, 5: 16
    if (carrierFrequency == Hz(2400))
        return 10E-6;
    else if (carrierFrequency == Hz(5000))
        return 16E-6;
    else
        throw cRuntimeError("Sifs time is not defined for %f Hz carrier frequency", carrierFrequency.get());
}

const simtime_t Ieee80211HTMode::getShortSlotTime() const
{
    if (carrierFrequency == Hz(2400))
        return 9E-6;
    else
        throw cRuntimeError("Short slot time is not defined for %f Hz carrier frequency", carrierFrequency.get());
}

} /* namespace physicallayer */
} /* namespace inet */

