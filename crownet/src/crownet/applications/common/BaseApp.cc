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

#include "BaseApp.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/common/Simsignals.h"

using namespace inet;

namespace crownet {
BaseApp::~BaseApp() {
  if (appLifeTime) cancelAndDelete(appLifeTime);
  delete localInfo;
}

void BaseApp::initialize(int stage) {
  crownet::queueing::CrownetActivePacketSourceBase::initialize(stage);
  if (stage == INITSTAGE_LOCAL) {

    startTime = par("startTime").doubleValue();
    stopTime = par("stopTime").doubleValue();

    hostId = getContainingNode(this)->getId();
    WATCH(hostId);

    localInfo = (AppInfoLocal*)(par("localInfo").objectValue()->dup());
    take(localInfo);
    initLocalAppInfo();


    if (stopTime > SIMTIME_ZERO && stopTime < startTime){
      throw cRuntimeError("Invalid startTime/stopTime parameters");
    }
    appLifeTime = new cMessage("applicationTimer");

    socketProvider = inet::getModuleFromPar<SocketProvider>(par("socketModule"), this);
    scheduler = inet::getModuleFromPar<IAppScheduler>(par("schedulerModule"), this);
  } else if (stage == INITSTAGE_APPLICATION_LAYER){
      handleStartOperation(nullptr);
  }
}

void BaseApp::initLocalAppInfo(){
    localInfo->setNodeId(getHostId());
    localInfo->setSequencenumber(0);
    localInfo->setWalltimeStart(startTime);
}

void BaseApp::setFsmResult(const FsmState &state) { socketFsmResult = state; }

void BaseApp::finish() {
  crownet::queueing::CrownetActivePacketSourceBase::finish();
}

void BaseApp::scheduleNextAppMainEvent(simtime_t time) {
    // do nothing
}

void BaseApp::overrideSocketDest(Packet *packet, L3Address addr, int port){
    packet->addTagIfAbsent<L3AddressReq>()->setDestAddress(addr);
    packet->addTagIfAbsent<L4PortReq>()->setDestPort(port);
}


void BaseApp::handleStartOperation(LifecycleOperation *operation) {
  simtime_t start = std::max(startTime, simTime());
  if ((stopTime == SIMTIME_ZERO) || (start <= stopTime) ||
      (start == stopTime && startTime == stopTime)) {
    appLifeTime->setKind(FsmRootStates::SETUP);
    scheduleAt(start, appLifeTime);
  } else {
    EV << "Lifecycle not started for BaseApp" << endl;
  }
}

/**
 * The root Finite State Machine handles basic lifecycle operations and
 * delegate selfMessage and Packet handling to fsmHandleSelfMsg and
 * fsmHandleWithSocket
 */
void BaseApp::handleMessage(cMessage *msg) {
  socketFsmResult = FsmRootStates::ERR;
  // fsmRoot == current state
  FSM_Switch(fsmRoot) {
    // Init state ...
    case FSM_Exit(FsmRootStates::INIT):
      FSM_Goto(fsmRoot, FsmRootStates::SETUP);
      break;
    //
    // FSM_Steady
    case FSM_Exit(FsmRootStates::WAIT_ACTIVE):
      if (msg->isSelfMessage()) {
        FsmState state = msg->getKind();
        if (std::abs(state) >= FsmRootStates::SUBSTATE) {
          // Handle sub states in fsmHandleSelfMsg
          FSM_Goto(fsmRoot, fsmHandleSubState(msg));
        } else {
          FSM_Goto(fsmRoot, msg->getKind());
        }
      } else if (msg->arrivedOn("in")) {
          FSM_Goto(fsmRoot, fsmDataArrived(msg));
      } else if (msg->arrivedOn("scheduleIn")){
          if (msg->isPacket()){
              // scheduler sends an already assembled packet. Just send it on its way.
              FSM_Goto(fsmRoot, fsmSendPacket(check_and_cast<Packet *>(msg)));
          } else {
              // scheduler event to be handled by the application logic
              FSM_Goto(fsmRoot, fsmHandleSubState(msg));
          }
      } else {
          throw cRuntimeError("Unkonwn Packet");
      }
      break;
    case FSM_Exit(FsmRootStates::WAIT_INACTIVE):
      EV_INFO << "Application stop time reached! WAIT_INACTIVE" << std::endl;
      break;
    case FSM_Enter(FsmRootStates::ERR):  // error! if state is entered
      throw cRuntimeError("Reached Error state");
      break;
    //
    // FSM_Transient
    case FSM_Exit(FsmRootStates::SETUP):
      FSM_Goto(fsmRoot, fsmSetup(msg));
      break;
    case FSM_Exit(FsmRootStates::APP_MAIN):
      FSM_Goto(fsmRoot, fsmAppMain(msg));
      break;
    case FSM_Exit(FsmRootStates::TEARDOWN):
      FSM_Goto(fsmRoot, fsmTeardown(msg));
      break;
    case FSM_Exit(FsmRootStates::DESTROY):
      FSM_Goto(fsmRoot, fsmDestroy(msg));
      break;
  }
}

void BaseApp::setupTimers() { }


void BaseApp::handlePacketProcessed(Packet *packet)
{
    // called before pushOrSendPacket.
    // Packet count already updated during packet creation
    emit(packetSentSignal, packet);
}

Packet *BaseApp::buildPacket(Ptr<Chunk> content, Ptr<Chunk> header){

    applyContentTags(content);

    auto packetName = createPacketName(content);
    auto packet = new Packet(packetName, content);


    if (header != nullptr){
        packet->insertAtFront(header);
    }
    applyPacketTags(packet);

    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    emit(packetCreatedSignal, packet);
    return packet;
}

// FSM
FsmState BaseApp::fsmHandleSubState(cMessage *msg) {
    throw cRuntimeError("No Substate on BaseApp");
}

FsmState BaseApp::fsmSetup(cMessage *msg) {
  socketProvider->setupSocket();

  if (stopTime > SIMTIME_ZERO) {
    appLifeTime->setKind(FsmRootStates::TEARDOWN);
    scheduleAt(stopTime, appLifeTime);
  }

  if ( !par("allEmptyDestAddress").boolValue() && !socketProvider->hasDestAddress()) {
      cRuntimeError("No destination address found.");
  }

  setupTimers();
  return FsmRootStates::WAIT_ACTIVE;
}

FsmState BaseApp::fsmDataArrived(cMessage *msg){
    Packet* pk = check_and_cast<Packet *>(msg);
    emit(packetReceivedSignal, pk);
    FsmState next = handleDataArrived(pk);
    delete pk;
    return next;
}

FsmState BaseApp::fsmSendPacket(Packet *pkt){
    handlePacketProcessed(pkt);
    pushOrSendPacket(pkt, outputGate, consumer);
    return FsmRootStates::WAIT_ACTIVE;
}

FsmState BaseApp::fsmAppMain(cMessage *msg) {
  // do nothing
  EV_WARN << "BaseApp has no AppMain. Implement for active message generate. App will still receive messages!"
          << std::endl;
  return FsmRootStates::WAIT_ACTIVE;
}

FsmState BaseApp::fsmTeardown(cMessage *msg) {
    socketProvider->close();
  cancelAndDelete(appLifeTime);
  appLifeTime = nullptr;

  return FsmRootStates::WAIT_INACTIVE;
}

FsmState BaseApp::fsmDestroy(cMessage *msg) {
  socketProvider->destroy();
  // TODO  in real operating systems, program crash detected
  // by OS and OS closes sockets of crashed programs.
  cancelAndDelete(appLifeTime);
  appLifeTime = nullptr;

  return FsmRootStates::WAIT_INACTIVE;
}

}  // namespace crownet
