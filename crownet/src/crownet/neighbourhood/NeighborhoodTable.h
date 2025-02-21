/*
 * NeighborhoodTable.h
 *
 *  Created on: Jan 23, 2021
 *      Author: sts
 */

#pragma once

#include <omnetpp/cobject.h>

#include "crownet/applications/beacon/BeaconReceptionInfo.h"
#include "inet/common/InitStages.h"


using namespace omnetpp;
using namespace inet;

namespace crownet {

class NeighborhoodTable : public omnetpp::cSimpleModule {
public:
    using nTable = std::map<int, BeaconReceptionInfo*>;
    virtual ~NeighborhoodTable();

    // cSimpleModule
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    BeaconReceptionInfo* getOrCreateEntry(const int sourceId);
    virtual void checkTimeToLive();
    const int getNeighbourCount();

    //getter
    const simtime_t& getMaxAge() const { return maxAge; }
    const nTable& getTable() const { return _table; }
    const cMessage* getTitleMessage() const { return ttl_msg;}


    //setter
    void setMaxAge(const simtime_t& _maxAge) { maxAge = _maxAge; }
    void setTable(const std::map<int, BeaconReceptionInfo*>& _nTable){ _table = _nTable;}
    void setTitleMessage(cMessage *msg){ttl_msg = msg;}

    // iterators and visitors
    typename nTable::iterator begin() { return _table.begin(); }
    typename nTable::iterator end() { return _table.end(); }
    typename nTable::const_iterator cbegin() {return _table.cbegin();}
    typename nTable::const_iterator cend() {return _table.cend();}



protected:
    nTable _table;
    simtime_t maxAge;
    cMessage *ttl_msg = nullptr;
    simtime_t lastCheck;
};

} /* namespace crownet */

