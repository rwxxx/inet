//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_ISYNCHRONIZATIONDECISION_H
#define __INET_ISYNCHRONIZATIONDECISION_H

#include "IReception.h"
#include "RadioControlInfo_m.h"

namespace inet {

namespace physicallayer {

/**
 * This interface represents the result of a receiver's synchronization process.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API ISynchronizationDecision : public IPrintableObject
{
  public:
    /**
     * Returns the corresponding reception that also specifies the receiver
     * and the received transmission. This function never returns NULL.
     */
    virtual const IReception *getReception() const = 0;

    /**
     * Returns the physical properties of the synchronization. This function
     * never returns NULL.
     */
    virtual const SynchronizationIndication *getIndication() const = 0;

    /**
     * Returns whether synchronization is possible according to the physical
     * properties of the received radio signal.
     */
    virtual bool isPossible() const = 0;

    /**
     * Returns whether the receiver decided to attempt the synchronization or
     * it decided to ignore it.
     */
    virtual bool isAttempted() const = 0;

    /**
     * Returns whether the synchronization was completely successful or not.
     */
    virtual bool isSuccessful() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_ISYNCHRONIZATIONDECISION_H

