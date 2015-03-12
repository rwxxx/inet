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

#ifndef __INET_IEEE80211HTDATACODE_H
#define __INET_IEEE80211HTDATACODE_H

#include "inet/physicallayer/contract/layered/ICode.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaving.h"
#include "inet/physicallayer/common/layered/ConvolutionalCode.h"
#include "inet/physicallayer/common/layered/AdditiveScrambling.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211HTDataCode
{
    protected:
        const IForwardErrorCorrection *forwardErrorCorrection;
        const Ieee80211Interleaving *interleaving1;
        const Ieee80211Interleaving *interleaving2;
        const Ieee80211Interleaving *interleaving3;
        const Ieee80211Interleaving *interleaving4;
        const AdditiveScrambling *scrambling;

    public:
        Ieee80211HTDataCode(const IForwardErrorCorrection *forwardErrorCorrection, const Ieee80211Interleaving *interleaving1, const Ieee80211Interleaving *interleaving2, const Ieee80211Interleaving *interleaving3, const Ieee80211Interleaving *interleaving4,  const AdditiveScrambling *scrambling);

        const IForwardErrorCorrection* getForwardErrorCorrection() const { return forwardErrorCorrection; }
        const Ieee80211Interleaving* getInterleaving1() const { return interleaving1; }
        const Ieee80211Interleaving* getInterleaving2() const { return interleaving2; }
        const Ieee80211Interleaving* getInterleaving3() const { return interleaving3; }
        const Ieee80211Interleaving* getInterleaving4() const { return interleaving4; }
        const AdditiveScrambling* getScrambling() const { return scrambling; }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211HTDATACODE_H
