//
// Copyright (c) 2010 The Boeing Company
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
// Author: Gary Pei <guangyu.pei@boeing.com>
//

#ifndef __INET_IEEE80211NISTERRORMODEL_H
#define __INET_IEEE80211NISTERRORMODEL_H

#include "inet/physicallayer/ieee80211/errormodel/Ieee80211ErrorModelBase.h"
#include "inet/physicallayer/ieee80211/errormodel/dsss-error-rate-model.h"
#include "inet/physicallayer/common/layered/ConvolutionalCode.h"
#include "inet/physicallayer/base/APSKModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * A model for the error rate for different modulations.  For OFDM modulation,
 * the model description and validation can be found in
 * http://www.nsnam.org/~pei/80211ofdm.pdf.  For DSSS modulations (802.11b),
 * the model uses the DsssErrorRateModel.
 */
class INET_API Ieee80211NistErrorModel : public Ieee80211ErrorModelBase
{
  protected:
    double CalculatePe(double p, uint32_t bValue) const;
    double GetBpskBer(double snr) const;
    double GetQpskBer(double snr) const;
    double Get16QamBer(double snr) const;
    double Get64QamBer(double snr) const;
    double GetFecBpskBer(double snr, double nbits, uint32_t bValue) const;
    double GetFecQpskBer(double snr, double nbits, uint32_t bValue) const;
    double GetFec16QamBer(double snr, uint32_t nbits, uint32_t bValue) const;
    double GetFec64QamBer(double snr, uint32_t nbits, uint32_t bValue) const;

    virtual double getOFDMAndERPOFDMChunkSuccessRate(const APSKModulationBase *subcarrierModulation, const ConvolutionalCode *convolutionalCode, unsigned int bitLength, double snr) const;
    virtual double getDSSSAndHrDSSSChunkSuccessRate(bps bitrate, unsigned int bitLength, double snr) const;

  public:
    virtual ~Ieee80211NistErrorModel();

    virtual void printToStream(std::ostream& stream) const override { stream << "Ieee80211NistErrorModel"; }
    virtual double getSuccessRate(const IIeee80211Mode *mode, unsigned int headerBitLength, unsigned int payloadBitLength, double snr) const override;

};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_NISTERRORRATEMODEL_H

