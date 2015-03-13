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

#include "inet/physicallayer/ieee80211/Ieee80211HTDataCode.h"

namespace inet {
namespace physicallayer {

Ieee80211HTDataCode::Ieee80211HTDataCode(
        const IForwardErrorCorrection* forwardErrorCorrection,
        const Ieee80211OFDMInterleaving* interleaving1,
        const Ieee80211OFDMInterleaving* interleaving2,
        const Ieee80211OFDMInterleaving* interleaving3,
        const Ieee80211OFDMInterleaving* interleaving4,
        const AdditiveScrambling* scrambling) :
                forwardErrorCorrection(forwardErrorCorrection),
                interleaving1(interleaving1),
                interleaving2(interleaving2),
                interleaving3(interleaving3),
                interleaving4(interleaving4),
                scrambling(scrambling)
{
}

} /* namespace physicallayer */
} /* namespace inet */

