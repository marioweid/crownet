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

#include "EmaPacketMeter.h"

namespace crownet {

Register_Class(EmaPacketMeter);

void EmaPacketMeter::meterPacket(Packet *packet){

    auto now = simTime();
    stats.incrData(packet->getTotalLength());
    stats.incrPacket();
    auto elapsedTime = (now - lastUpdate).dbl();
    if (stats.getPacketCount() == 1) {
        // first call
        if (elapsedTime <= 0.0){
            stats.setPacketRate(1.0);
            stats.setDataRate(bps(packet->getDataLength()/ s(1)));
        } else {
            stats.setPacketRate(1.0 / elapsedTime);
            stats.setDataRate(bps(packet->getDataLength()/ s(elapsedTime)));
        }
    } else if( now != lastUpdate ) {
        auto packetrateChange = (1 / elapsedTime) - stats.getPacketRate();
        stats.setPacketRate(stats.getPacketRate() + packetrateChange * alpha);

        double datarateChange = (double)(packet->getTotalLength().get() / elapsedTime) - (double)stats.getDataRate().get();
        double dRate = stats.getDataRate().get() + alpha* datarateChange;
        stats.setDataRate(bps(dRate));

        lastUpdate = now;

    } else {
        throw cRuntimeError("multiple packets at the same time not supported");
    }
}

}

