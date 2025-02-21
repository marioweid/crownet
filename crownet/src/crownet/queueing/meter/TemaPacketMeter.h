//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef CROWNET_QUEUEING_METER_TEMAPACKETMETER_H_
#define CROWNET_QUEUEING_METER_TEMAPACKETMETER_H_


#include "inet/queueing/contract/IPacketMeter.h"
#include "crownet/queueing/meter/meter_m.h"

namespace crownet {


class TemaPacketMeter : public TemaPacketMeter_Base {

protected:
    simtime_t lastUpdate = simtime_t::ZERO;
    bool t_0 = true;

public:
    virtual void meterPacket(Packet *packet) override;
    virtual TemaPacketMeter *dup() const override {
        return new TemaPacketMeter(*this);
    }

};
}

#endif /* CROWNET_QUEUEING_METER_TEMAPACKETMETER_H_ */
