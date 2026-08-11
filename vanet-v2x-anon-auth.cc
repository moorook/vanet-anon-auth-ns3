#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <algorithm>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("V2XAnonAuth");

/*
 * Reproducible ns-3 evaluation of the V2X anonymous-authentication workflow.
 * ns-3: 3.46.1; IEEE 802.11p; 2-km highway; ConstantVelocityMobilityModel.
 *
 * IMPORTANT: cryptographic operations are abstracted as configurable
 * authentication-processing delays. This is a network/event-level study,
 * not a native cryptographic implementation and not a SUMO experiment.
 */
uint32_t numVehicles = 50;
uint32_t numRSU = 5;
double simulationTime = 10.0;
double vehicleSpeed = 20.0;
double roadLength = 2000.0;
double v2iInterval = 0.05;
double v2vInterval = 0.05;
uint32_t v2vEvents = 40;
Time v2iAuthDelay = MilliSeconds(8);
Time v2vAuthDelay = MilliSeconds(2);
uint32_t v2iAuthPktSize = 321;
uint32_t v2vAuthPktSize = 321;

uint32_t v2iSent=0,v2iDone=0,v2vSent=0,v2vDone=0;
double v2iTotalDelay=0.0,v2vTotalDelay=0.0;
std::map<uint32_t,Time> v2iSendTime,v2vSendTime;

void CompleteV2I(uint32_t id){
    ++v2iDone;
    auto it=v2iSendTime.find(id);
    if(it!=v2iSendTime.end()) v2iTotalDelay+=(Simulator::Now()-it->second).GetMilliSeconds();
}
void CompleteV2V(uint32_t id){
    ++v2vDone;
    auto it=v2vSendTime.find(id);
    if(it!=v2vSendTime.end()) v2vTotalDelay+=(Simulator::Now()-it->second).GetMilliSeconds();
}
void TriggerV2I(uint32_t id){++v2iSent;v2iSendTime[id]=Simulator::Now();Simulator::Schedule(v2iAuthDelay,&CompleteV2I,id);}
void TriggerV2V(uint32_t id){++v2vSent;v2vSendTime[id]=Simulator::Now();Simulator::Schedule(v2vAuthDelay,&CompleteV2V,id);}

void WriteResults(const std::string& file){
    const double v2iRate=v2iSent?100.0*v2iDone/v2iSent:0.0;
    const double v2vRate=v2vSent?100.0*v2vDone/v2vSent:0.0;
    const double v2iDelay=v2iDone?v2iTotalDelay/v2iDone:0.0;
    const double v2vDelay=v2vDone?v2vTotalDelay/v2vDone:0.0;
    std::ofstream o(file); if(!o) NS_FATAL_ERROR("Cannot open "<<file);
    o<<"Metric,Value\n"
     <<"Vehicles,"<<numVehicles<<"\n"
     <<"RSUs,"<<numRSU<<"\n"
     <<"SimulationTime_s,"<<simulationTime<<"\n"
     <<"RoadLength_m,"<<roadLength<<"\n"
     <<"VehicleSpeed_mps,"<<vehicleSpeed<<"\n"
     <<"V2IInterval_s,"<<v2iInterval<<"\n"
     <<"V2VInterval_s,"<<v2vInterval<<"\n"
     <<"V2VRequestedEvents,"<<v2vEvents<<"\n"
     <<"V2I_PacketBytes,"<<v2iAuthPktSize<<"\n"
     <<"V2V_PacketBytes,"<<v2vAuthPktSize<<"\n"
     <<"V2I_Sent,"<<v2iSent<<"\n"
     <<"V2I_Completed,"<<v2iDone<<"\n"
     <<"V2I_SuccessRate,"<<v2iRate<<"\n"
     <<"V2I_AverageDelay_ms,"<<v2iDelay<<"\n"
     <<"V2V_Sent,"<<v2vSent<<"\n"
     <<"V2V_Completed,"<<v2vDone<<"\n"
     <<"V2V_SuccessRate,"<<v2vRate<<"\n"
     <<"V2V_AverageDelay_ms,"<<v2vDelay<<"\n";
}

int main(int argc,char* argv[]){
    CommandLine cmd;
    cmd.AddValue("numVehicles","Number of vehicles",numVehicles);
    cmd.AddValue("numRSU","Number of RSUs",numRSU);
    cmd.AddValue("simulationTime","Simulation time (s)",simulationTime);
    cmd.AddValue("vehicleSpeed","Vehicle speed (m/s)",vehicleSpeed);
    cmd.AddValue("roadLength","Highway length (m)",roadLength);
    cmd.AddValue("v2iInterval","V2I event interval (s)",v2iInterval);
    cmd.AddValue("v2vInterval","V2V event interval (s)",v2vInterval);
    cmd.AddValue("v2vEvents","Requested V2V events",v2vEvents);
    cmd.AddValue("v2iAuthDelay","V2I abstract processing delay",v2iAuthDelay);
    cmd.AddValue("v2vAuthDelay","V2V abstract processing delay",v2vAuthDelay);
    cmd.AddValue("v2iAuthPktSize","V2I authentication payload (bytes)",v2iAuthPktSize);
    cmd.AddValue("v2vAuthPktSize","V2V authentication payload (bytes)",v2vAuthPktSize);
    cmd.Parse(argc,argv);

    NodeContainer vehicles; vehicles.Create(numVehicles);
    NodeContainer rsus; rsus.Create(numRSU);
    WifiHelper wifi; wifi.SetStandard(WIFI_STANDARD_80211p);
    YansWifiChannelHelper channel;
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");
    YansWifiPhyHelper phy; phy.SetChannel(channel.Create());
    WifiMacHelper mac; mac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer vd=wifi.Install(phy,mac,vehicles);
    NetDeviceContainer rd=wifi.Install(phy,mac,rsus);
    InternetStackHelper internet; internet.Install(vehicles); internet.Install(rsus);
    Ipv4AddressHelper ipv4; ipv4.SetBase("10.1.0.0","255.255.0.0");
    ipv4.Assign(vd); ipv4.Assign(rd);

    MobilityHelper vm;
    vm.SetMobilityModel("ns3::ConstantVelocityMobilityModel"); vm.Install(vehicles);
    for(uint32_t i=0;i<numVehicles;++i){
        double x=(numVehicles>1)?(double(i)/double(numVehicles-1))*roadLength:0.0;
        auto m=vehicles.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        m->SetPosition(Vector(x,0,0)); m->SetVelocity(Vector(vehicleSpeed,0,0));
    }
    MobilityHelper rm; rm.SetMobilityModel("ns3::ConstantPositionMobilityModel"); rm.Install(rsus);
    for(uint32_t i=0;i<numRSU;++i){
        double x=(numRSU>1)?(double(i)/double(numRSU-1))*roadLength:roadLength/2.0;
        rsus.Get(i)->GetObject<MobilityModel>()->SetPosition(Vector(x,20,0));
    }

    for(uint32_t i=0;i<numVehicles;++i){
        double t=1.0+double(i)*v2iInterval;
        if(t<simulationTime-1.0) Simulator::Schedule(Seconds(t),&TriggerV2I,i);
    }
    uint32_t n=std::min(v2vEvents,numVehicles);
    for(uint32_t i=0;i<n;++i){
        double t=2.0+double(i)*v2vInterval;
        if(t<simulationTime-1.0) Simulator::Schedule(Seconds(t),&TriggerV2V,i);
    }
    Simulator::Stop(Seconds(simulationTime)); Simulator::Run();

    std::ostringstream f; f<<"results/auth_"<<numVehicles<<"_rsu_"<<numRSU<<".csv";
    WriteResults(f.str());
    std::cout<<"Vehicles="<<numVehicles<<" RSUs="<<numRSU
             <<" V2I="<<v2iDone<<"/"<<v2iSent
             <<" V2V="<<v2vDone<<"/"<<v2vSent
             <<" output="<<f.str()<<std::endl;
    Simulator::Destroy(); return 0;
}
