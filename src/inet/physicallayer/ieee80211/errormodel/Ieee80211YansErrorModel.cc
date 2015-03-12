//
// Copyright (c) 2005, 2006 INRIA
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
// Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
//

#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211DSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HRDSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"
#include "inet/physicallayer/ieee80211/errormodel/Ieee80211YansErrorModel.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211YansErrorModel);

Ieee80211YansErrorModel::Ieee80211YansErrorModel()
{
}

double Ieee80211YansErrorModel::GetBpskBer(double snr, Hz signalSpread, bps phyRate) const
{
    double EbNo = snr * signalSpread.get() / phyRate.get();
    double z = sqrt(EbNo);
    double ber = 0.5 * erfc(z);
    EV << "bpsk snr=" << snr << " ber=" << ber << endl;
    return ber;
}

double Ieee80211YansErrorModel::GetQamBer(double snr, unsigned int m, Hz signalSpread, bps phyRate) const
{
    double EbNo = snr * signalSpread.get() / phyRate.get();
    double z = sqrt((1.5 * log2(m) * EbNo) / (m - 1.0));
    double z1 = ((1.0 - 1.0 / sqrt((double)m)) * erfc(z));
    double z2 = 1 - pow((1 - z1), 2.0);
    double ber = z2 / log2(m);
    EV << "Qam m=" << m << " rate=" << phyRate << " snr=" << snr << " ber=" << ber << endl;
    return ber;
}

uint32_t Ieee80211YansErrorModel::Factorial(uint32_t k) const
{
    uint32_t fact = 1;
    while (k > 0) {
        fact *= k;
        k--;
    }
    return fact;
}

double Ieee80211YansErrorModel::Binomial(uint32_t k, double p, uint32_t n) const
{
    double retval = Factorial(n) / (Factorial(k) * Factorial(n - k)) * pow(p, (int)k) * pow(1 - p, (int)(n - k));
    return retval;
}

double Ieee80211YansErrorModel::CalculatePdOdd(double ber, unsigned int d) const
{
    ASSERT((d % 2) == 1);
    unsigned int dstart = (d + 1) / 2;
    unsigned int dend = d;
    double pd = 0;

    for (unsigned int i = dstart; i < dend; i++) {
        pd += Binomial(i, ber, d);
    }
    return pd;
}

double Ieee80211YansErrorModel::CalculatePdEven(double ber, unsigned int d) const
{
    ASSERT((d % 2) == 0);
    unsigned int dstart = d / 2 + 1;
    unsigned int dend = d;
    double pd = 0;

    for (unsigned int i = dstart; i < dend; i++) {
        pd += Binomial(i, ber, d);
    }
    pd += 0.5 * Binomial(d / 2, ber, d);

    return pd;
}

double Ieee80211YansErrorModel::CalculatePd(double ber, unsigned int d) const
{
    double pd;
    if ((d % 2) == 0) {
        pd = CalculatePdEven(ber, d);
    }
    else {
        pd = CalculatePdOdd(ber, d);
    }
    return pd;
}

double Ieee80211YansErrorModel::GetFecBpskBer(double snr, double nbits, Hz signalSpread, bps phyRate, uint32_t dFree, uint32_t adFree) const
{
    double ber = GetBpskBer(snr, signalSpread, phyRate);
    if (ber == 0.0) {
        return 1.0;
    }
    double pd = CalculatePd(ber, dFree);
    double pmu = adFree * pd;
    pmu = std::min(pmu, 1.0);
    double pms = pow(1 - pmu, nbits);
    return pms;
}

double Ieee80211YansErrorModel::GetFecQamBer(double snr, uint32_t nbits, Hz signalSpread, bps phyRate, uint32_t m, uint32_t dFree, uint32_t adFree, uint32_t adFreePlusOne) const
{
    double ber = GetQamBer(snr, m, signalSpread, phyRate);
    if (ber == 0.0) {
        return 1.0;
    }
    /* first term */
    double pd = CalculatePd(ber, dFree);
    double pmu = adFree * pd;
    /* second term */
    pd = CalculatePd(ber, dFree + 1);
    pmu += adFreePlusOne * pd;
    pmu = std::min(pmu, 1.0);
    double pms = pow(1 - pmu, (int)nbits);
    return pms;
}

double Ieee80211YansErrorModel::getOFDMAndERPOFDMChunkSuccessRate(const APSKModulationBase* subcarrierModulation, const ConvolutionalCode* convolutionalCode, unsigned int bitLength, bps grossBitrate, Hz bandwidth, double snr) const
{
    if (subcarrierModulation == &BPSKModulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return GetFecBpskBer(snr, bitLength, bandwidth, grossBitrate, 10, 11);
        else
            return GetFecBpskBer(snr, bitLength, bandwidth, grossBitrate, 5, 8 );
    }
    else if (subcarrierModulation == &QPSKModulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return GetFecQamBer(snr, bitLength, bandwidth, grossBitrate, 4, 10, 11, 0 );
        else
            return GetFecQamBer(snr, bitLength, bandwidth, grossBitrate, 4, 5, 8, 31);
    }
    else if (subcarrierModulation == &QAM16Modulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return GetFecQamBer(snr, bitLength, bandwidth, grossBitrate, 16, 10, 11, 0);
        else
            return GetFecQamBer(snr, bitLength, bandwidth, grossBitrate, 16, 5, 8, 31);
    }
    else if (subcarrierModulation == &QAM64Modulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 2 && convolutionalCode->getCodeRatePuncturingN() == 3)
            return GetFecQamBer(snr, bitLength, bandwidth, grossBitrate, 64, 6, 1, 16);
        else
            return GetFecQamBer(snr, bitLength, bandwidth, grossBitrate, 64, 5, 8, 31);
    }
    else
        throw cRuntimeError("Unknown modulation");
}

double Ieee80211YansErrorModel::getDSSSAndHrDSSSChunkSuccessRate(bps bitrate, unsigned int bitLength, double snr) const
{
    switch ((int)bitrate.get()) {
        case 1000000:
            return DsssErrorRateModel::GetDsssDbpskSuccessRate(snr, bitLength);
        case 2000000:
            return DsssErrorRateModel::GetDsssDqpskSuccessRate(snr, bitLength);
        case 5500000:
            return DsssErrorRateModel::GetDsssDqpskCck5_5SuccessRate(snr, bitLength);
        case 11000000:
            return DsssErrorRateModel::GetDsssDqpskCck11SuccessRate(snr, bitLength);
    }
    throw cRuntimeError("Unsupported bitrate");
}

double Ieee80211YansErrorModel::getSuccessRate(const IIeee80211Mode* mode, unsigned int headerBitLength, unsigned int payloadBitLength, double snr) const
{
    double headerSuccessRate = 0;
    double payloadSuccessRate = 0;
    if (dynamic_cast<const Ieee80211OFDMMode *>(mode))
    {
        const Ieee80211OFDMMode *ofdmMode = dynamic_cast<const Ieee80211OFDMMode *>(mode);
        headerSuccessRate = getOFDMAndERPOFDMChunkSuccessRate(ofdmMode->getHeaderMode()->getModulation()->getSubcarrierModulation(),
                                                              ofdmMode->getHeaderMode()->getCode()->getConvolutionalCode(),
                                                              headerBitLength,
                                                              ofdmMode->getHeaderMode()->getGrossBitrate(),
                                                              ofdmMode->getHeaderMode()->getBandwidth(),
                                                              snr);
        payloadSuccessRate = getOFDMAndERPOFDMChunkSuccessRate(ofdmMode->getDataMode()->getModulation()->getSubcarrierModulation(),
                                                              ofdmMode->getDataMode()->getCode()->getConvolutionalCode(),
                                                              payloadBitLength,
                                                              ofdmMode->getDataMode()->getGrossBitrate(),
                                                              ofdmMode->getHeaderMode()->getBandwidth(),
                                                              snr);
    }
    else if (dynamic_cast<const Ieee80211DsssMode *>(mode))
    {
        const Ieee80211DsssMode *dsssMode = dynamic_cast<const Ieee80211DsssMode *>(mode);
        headerSuccessRate = getDSSSAndHrDSSSChunkSuccessRate(dsssMode->getHeaderMode()->getNetBitrate(), headerBitLength, snr);
        payloadSuccessRate = getDSSSAndHrDSSSChunkSuccessRate(dsssMode->getDataMode()->getNetBitrate(), payloadBitLength, snr);
    }
    else if (dynamic_cast<const Ieee80211HrDsssMode *>(mode))
    {
        const Ieee80211HrDsssMode *hrDsssMode = dynamic_cast<const Ieee80211HrDsssMode *>(mode);
        headerSuccessRate = getDSSSAndHrDSSSChunkSuccessRate(hrDsssMode->getHeaderMode()->getNetBitrate(), headerBitLength, snr);
        payloadSuccessRate = getDSSSAndHrDSSSChunkSuccessRate(hrDsssMode->getDataMode()->getNetBitrate(), payloadBitLength, snr);
    }
//    else
//        throw cRuntimeError("Unsupported 802.11 mode");
    EV_DEBUG << "Min SNIR = " << snr << ", bit length = " << payloadBitLength << ", header error rate = " << 1 - headerSuccessRate << ", payload error rate = " << 1 - payloadSuccessRate << endl;
    if (headerSuccessRate >= 1)
        headerSuccessRate = 1;
    if (payloadSuccessRate >= 1)
        payloadSuccessRate = 1;
    return headerSuccessRate * payloadSuccessRate;
}

} // namespace physicallayer

} // namespace inet

