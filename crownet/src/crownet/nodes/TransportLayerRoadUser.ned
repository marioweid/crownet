package crownet.nodes;

import crownet.artery.PedestrianMiddleware;
import crownet.artery.IDispatchRadioDriver;
import crownet.aid.IAidLayer;
import crownet.neighbourhood.INeighborhoodTable;
import artery.networking.Vanetza;
import artery.networking.IVanetza;
import artery.inet.PlainVehicle;
import artery.nic.IRadioDriver;
import artery.application.IMiddleware;
import inet.node.inet.INetworkNode;
import inet.linklayer.contract.IWirelessInterface;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.configurator.ipv4.HostAutoConfigurator;
import inet.applications.contract.IApp;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.contract.INetworkLayer;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;
import inet.transportlayer.contract.ISctp;
import inet.common.MessageDispatcher;
import simu5g.stack.ICellularNic;

module TransportLayerRoadUser like INetworkNode
{
    parameters:
        @networkNode();
        @labels(node,ethernet-node,wireless-node);
        
        @figure[transportLayer](type=rectangle; pos=250,306; size=600,130; fillColor=#ff0000; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[transportLayer.title](type=text; pos=845,311; anchor=ne; text="transport layer");

        @figure[networkLayer](type=rectangle; pos=250,456; size=600,130; fillColor=#00ff00; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[networkLayer.title](type=text; pos=845,461; anchor=ne; text="network layer");

        @figure[linkLayer](type=rectangle; pos=250,606; size=600,130; fillColor=#0000ff; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[linkLayer.title](type=text; pos=845,611; anchor=ne; text="link layer");
        @figure[submodules];

        @statistic[posX](source="xCoord(mobilityPos(mobilityStateChanged))"; record=vector?);
        @statistic[posY](source="yCoord(mobilityPos(mobilityStateChanged))"; record=vector?);
        string osgModel = default(""); // 3D model for OSG visualization, no 3D model by default
        string osgModelColor = default(""); // tint color, no colorization by default
        string canvasImage = default(""); // image for canvas visualization, no image by default
        string canvasImageColor = default(""); // tint color, no colorization by default
        
        int numRadios = default(1);
        bool withAntennaMobility = default(false);
        *.mobility.visualRepresentation = "^";

        //# Artery
        bool useArtery = default(false);
        string middlewareType = ifTrueOrElse(useArtery, "PedestrianMiddleware", "PedestrianMiddlewareEmpty");

        //# Transport layer
        bool hasUdp = default(firstAvailableOrEmpty("Udp") != "");
        bool hasTcp = default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc") != "");
        bool hasSctp = default(false);

        //# Node specs
        string nodeType = "UE";     // DO NOT CHANGE
        int masterId = default(0);
        int macNodeId = default(0); // TODO: this is not a real parameter
        int macCellId = default(0); // TODO: this is not a real parameter

        //# D2D capability
        string nicType = default("LteNicUe");

        //# Network Layer specs
        bool hasIpv4 = default(true);
        bool hasIpv6 = default(false);

        //# Link Layer specs
        bool hasCellular = default(true);
        bool hasWlan = default(false);
        int numWlanNic = default(0);

        **.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default("^.^.ipv4.routingTable");
        **.mobilityModule = default(absPath(".mobility"));
        **.neighborhoodTableMobdule = default(absPath(".nTable"));
        **.multicastInterface = default("cellular");

        //# Ext Interface
        bool enableExtInterface = default(false);
        string extHostAddress = default("");

        int numPcapRecorders = default(0); // no of PcapRecorders
    gates:
        input lteRadioIn @directIn;
        input radioIn[numWlanNic] @directIn;     // connection to master

    submodules:
        interfaceTable: InterfaceTable {
            @display("p=84.672,147.392;is=s");
        }
        mobility: <default("crownet.artery.traci.InetVaderePersonMobility")> like IMobility {
            parameters:
                @display("p=83.104004,54.88;is=s");
        }
        nTable: <default("NeighborhoodTable")> like INeighborhoodTable if typename!="" {
            @display("p=216.384,147.392;i=block/table2;is=s");
        }
        middleware: <default(middlewareType)> like IMiddleware if typename!="" {
            parameters:
                @display("p=109.76,296.352");
        }
        vanetza: <default("Vanetza")> like IVanetza if useArtery {
            parameters:
                @display("p=109.76,393.568");
                *.middlewareModule = absPath("^.middleware");
                *.mobilityModule = absPath("^.mobility");
                *.radioDriverModule = absPath("^.radioDriver[0]");
                runtime.datetime = middleware.datetime;
        }
        radioDriver: <default("LteRadioDriver")> like IDispatchRadioDriver if useArtery {
            parameters:
                @display("p=109.76,553.504,row,100");
        }
        udp: <default(firstAvailableOrEmpty("Udp"))> like IUdp if hasUdp {
            parameters:
                @display("p=346.69598,370.856");
        }
        tcp: <default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc"))> like ITcp if hasTcp {
            parameters:
                @display("p=473.53598,370.856");
        }
        sctp: <default(firstAvailableOrEmpty("Sctp"))> like ISctp if hasSctp {
            parameters:
                @display("p=710.304,370.856");
        }
        ipv4: <default("Ipv4NetworkLayer")> like INetworkLayer if hasIpv4 {
            parameters:
                @display("p=425,507;q=queue");
                //todo: hostAutoConfigurator with wlan and cellular
                //set HostAutoConfigurator (deactivated)
                //use standalone configurator
                configurator.typename = "HostAutoConfigurator";
                configurator.interfaces = default("cellular");
        }
        ipv6: <default("Ipv6NetworkLayer")> like INetworkLayer if hasIpv6 {
            parameters:
                @display("p=681.312,526.688;q=queue");
        }
        cellularNic: <nicType> like ICellularNic if hasCellular {
            nodeType = nodeType;
            ip2nic.typename = "Geo2Nic";
            //d2dCapable = d2dCapable;
            @display("p=710.304,672.672");
        }

        wlanNic[numWlanNic]: <default("Ieee80211Interface")> like IWirelessInterface if hasWlan {
            parameters:
                @display("p=407.68,672.672,row,150;q=queue");
        }
        // message dispatcher for SAP between transport and network layer
        tn: MessageDispatcher {
            parameters:
                @display("p=550,446;b=600,5,,,,1");
        }
        // message dispatcher for SAP to link layer
        nl: MessageDispatcher {
            parameters:
                @display("p=550,596;b=600,5,,,,1");
        }
        configurator: HostAutoConfigurator {
            @display("p=216.384,54.88;is=s");
        }

    connections allowunconnected:
        //# LTE Nic
        cellularNic.radioIn <-- lteRadioIn if exists(cellularNic);

        cellularNic.upperLayerOut --> nl.in++ if exists(cellularNic);
        cellularNic.upperLayerIn <-- nl.out++ if exists(cellularNic);

        //# Wlan Nic
        for i=0..sizeof(radioIn)-1 {
            wlanNic[i].upperLayerOut --> nl.in++ if exists(wlanNic);
            wlanNic[i].upperLayerIn <-- nl.out++ if exists(wlanNic);
            wlanNic[i].radioIn <-- radioIn[i] if exists(wlanNic);
        }

        //# Artery
        radioDriver.upperLayer <--> vanetza.radioDriverData if useArtery;
        radioDriver.properties --> vanetza.radioDriverProperties if useArtery;


        //#
		//# Internal transport layer connections to network layer
		//#

        udp.ipOut --> tn.in++ if hasUdp;
        udp.ipIn <-- tn.out++ if hasUdp;

        tcp.ipOut --> tn.in++ if hasTcp;
        tcp.ipIn <-- tn.out++ if hasTcp;

        sctp.ipOut --> tn.in++ if hasSctp;
        tn.out++ --> sctp.ipIn if hasSctp;

        ipv4.ifIn <-- nl.out++ if hasIpv4;
        ipv4.ifOut --> nl.in++ if hasIpv4;

        radioDriver.lowerLayerIn <-- nl.out++ if useArtery;
        radioDriver.lowerLayerOut --> nl.in++ if useArtery;

        ipv4.transportIn <-- tn.out++ if hasIpv4;
        ipv4.transportOut --> tn.in++ if hasIpv4;

        ipv6.ifIn <-- nl.out++ if hasIpv6;
        ipv6.ifOut --> nl.in++ if hasIpv6;

        ipv6.transportIn <-- tn.out++ if hasIpv6;
        ipv6.transportOut --> tn.in++ if hasIpv6;

        tn.out++ --> nl.in++;
        tn.in++ <-- nl.out++;
}
