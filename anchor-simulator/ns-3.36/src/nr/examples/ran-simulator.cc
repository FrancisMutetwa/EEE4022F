#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/nr-module.h"
#include "ns3/antenna-module.h"
#include "ns3/lte-module.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/log.h"
#include "ns3/netanim-module.h"
#include "ns3/spectrum-value.h"
#include "ns3/nr-spectrum-value-helper.h"
#include "ns3/nr-phy-mac-common.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/udp-client.h"

#include <fstream>
#include <map>
#include <limits>
#include <sstream>
#include <jsoncpp/json/json.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ran-simulator");

void PrintSimulationStatus(uint32_t simulationDuration);

// Add these near the top of the file after includes
std::ofstream positionLogFile;
std::string positionLogPath;

// Add this function before main()
void LogNodePosition(uint32_t nodeId, Vector position) {
  if (!positionLogFile.is_open()) {
    positionLogFile.open(positionLogPath);
    positionLogFile << "Time,NodeId,X,Y,Z" << std::endl;
  }
  positionLogFile << Simulator::Now().GetSeconds() << "," 
                 << nodeId << "," 
                 << position.x << "," 
                 << position.y << "," 
                 << position.z << std::endl;
}

class PositionLogger {
public:
  static void Log(uint32_t nodeId, Vector position) {
    if (!m_file.is_open()) {
      m_file.open("node_positions.csv");
      m_file << "Time,NodeId,X,Y,Z" << std::endl;
    }
    m_file << Simulator::Now().GetSeconds() << "," 
           << nodeId << "," 
           << position.x << "," 
           << position.y << "," 
           << position.z << std::endl;
  }

  static void Close() {
    if (m_file.is_open()) {
      m_file.close();
    }
  }

private:
  static std::ofstream m_file;
};

std::ofstream PositionLogger::m_file;

int
main (int argc, char *argv[])
{
  uint64_t startTime = time(NULL);
  uint32_t simRoundDurationMs = 10000;	// round simulation duration in ms. Don't use double to indicate s! Use ms and integer to avoid representation errors
  uint32_t simRoundDurationS = simRoundDurationMs / 1000;	// round duration in seconds
  uint32_t simDurationS = 600;	// simulation duration in seconds
  uint32_t simRoundIndex = 0;	// indicates the round Index
  string environment = "RMa_LoS";  // default environment type
  CommandLine cmd;
  // Allows including these variables in the simulation round launch command to set values different from the default ones
  cmd.AddValue ("simRoundDurationMs", "Simulation round duration in millisec", simRoundDurationMs);
  cmd.AddValue ("simDurationS", "Simulation duration in sec", simDurationS);
  cmd.AddValue ("simRoundIndex", "Index of the currently running round", simRoundIndex);
  cmd.AddValue ("environment", "Environment type (RMa_LoS, UMa_LoS, UMi_StreetCanyon)", environment);
  cmd.Parse (argc, argv);
  // Create a log file to store possible errors occuring in runtime
  stringstream logFileName;
  logFileName << outputPath << "logFile.txt";
  ofstream logFile(logFileName.str().c_str(), ofstream::out);

  // Read the network configuration input file
  stringstream netConfFileName;
  netConfFileName << inputPath <<   "networkConfiguration.txt";
  ifstream netConfFile(netConfFileName.str().c_str());
  if (!netConfFile)
    {
	  cerr << "Network configuration file: Unable to open file!";
	  logFile << "Network configuration file: Unable to open file!";
      exit(1);   // call system to stop
    }
  cout << "Start round " << simRoundIndex << "\n";
  logFile << "Start round " << simRoundIndex << "\n";
  cout << "Load Network configuration!\n";
  logFile << "Load Network configuration!\n";
  string keyword;
  // Scenario parameters configuration
  uint32_t numgNB, numTerrestrialGNBs, numTrafficProfile, numTotalUE = 0;
  netConfFile >> keyword;
  if (keyword == "numgNBs")
	  netConfFile >> numgNB;
  else
    {
	  cerr << "Network configuration file: Missing number of gNB information\n";
	  logFile << "Network configuration file: Missing number of gNB information\n";
      exit(1);
    }

  netConfFile >> keyword;
  if (keyword == "numTerrestrialGNBs")
	  netConfFile >> numTerrestrialGNBs;
  else
    {
	  cerr << "Network configuration file: Missing number of terrestrial gNB information\n";
	  logFile << "Network configuration file: Missing number of terrestrial gNB information\n";
      exit(1);
    }  
  netConfFile >> keyword;
  if (keyword == "numTrafficProfiles")
	  netConfFile >> numTrafficProfile;
  else
    {
  	  cerr << "Network configuration file: Missing number of traffic profile information\n";
  	  logFile << "Network configuration file: Missing number of traffic profile information\n";
      exit(1);
    }
  uint32_t numUEPerTrafProf[numTrafficProfile];
  netConfFile >> keyword;
  if (keyword == "numUEsPerTrafProf")
	  for (uint32_t i = 0; i < numTrafficProfile; i++)
	    {
		  netConfFile >> numUEPerTrafProf[i];
		  numTotalUE += numUEPerTrafProf[i];
	    }
  else
    {
  	  cerr << "Network configuration file: Missing number of UE per traffic profile information\n";
  	  logFile << "Network configuration file: Missing number of UE per traffic profile information\n";
      exit(1);
    }
  // NR parameters configuration
  uint32_t numberActiveFrequencyBands;
  netConfFile >> keyword;
  if (keyword == "numActiveFrequencyBands") {
	  netConfFile >> numberActiveFrequencyBands;
  }
  else
    {
  	  cerr << "Network configuration file: Missing number of active frequency bands information\n";
  	  logFile << "Network configuration file: Missing number of active frequency bands information\n";
      exit(1);
    }
  double centralFrequenciesBands[numberActiveFrequencyBands], bandwidthBands[numberActiveFrequencyBands];
  netConfFile >> keyword;
  if (keyword == "centralFrequencyBands")
	  for (uint32_t i = 0; i < numberActiveFrequencyBands; i++)
		  netConfFile >> centralFrequenciesBands[i];
  else
    {
  	  cerr << "Network configuration file: Missing central frequencies of active frequency bands information\n";
      logFile << "Network configuration file: Missing central frequencies of active frequency bands information\n";
      exit(1);
    }
  netConfFile >> keyword;
  if (keyword == "bandwidthBands")
	  for (uint32_t i = 0; i < numberActiveFrequencyBands; i++)
		  netConfFile >> bandwidthBands[i];
  else
    {
  	  cerr << "Network configuration file: Missing bandwidth of active frequency bands information\n";
  	  logFile << "Network configuration file: Missing bandwidth of active frequency bands information\n";
      exit(1);
    }
  uint32_t numCcPerBand[numberActiveFrequencyBands], numTotalCC = 0;
  netConfFile >> keyword;
  if (keyword == "numCCsPerBand")
  	  for (uint32_t i = 0; i < numberActiveFrequencyBands; i++)
  	    {
  		  netConfFile >> numCcPerBand[i];
  		  numTotalCC += numCcPerBand[i];
  	    }
  else
    {
      cerr << "Network configuration file: Missing number of CC of each active frequency band information\n";
      logFile << "Network configuration file: Missing number of CC of each active frequency band information\n";
      exit(1);
    }
  double centralFrequenciesCCs[numTotalCC], bandwidthCCs[numTotalCC];
  netConfFile >> keyword;
  if (keyword == "centralFrequencyCCs")
	  for (uint32_t i = 0; i < numTotalCC; i++)
		  netConfFile >> centralFrequenciesCCs[i];
  else
    {
  	  cerr << "Network configuration file: Missing central frequencies of active CCs information\n";
  	  logFile << "Network configuration file: Missing central frequencies of active CCs information\n";
      exit(1);
    }
  netConfFile >> keyword;
  if (keyword == "bandwidthCCs")
	  for (uint32_t i = 0; i < numTotalCC; i++)
		  netConfFile >> bandwidthCCs[i];
  else
    {
  	  cerr << "Network configuration file: Missing bandwidth of active CCs information\n";
  	  logFile << "Network configuration file: Missing bandwidth of active CCs information\n";
      exit(1);
    }
  uint32_t numBwpPerCC[numTotalCC], numTotalBWP = 0;
  netConfFile >> keyword;
  if (keyword == "numBWPsPerCC")
  	  for (uint32_t i = 0; i < numTotalCC; i++)
  	    {
  		  netConfFile >> numBwpPerCC[i];
  		  numTotalBWP += numBwpPerCC[i];
  	    }
  else
    {
      cerr << "Network configuration file: Missing number of BWP of each CC information\n";
      logFile << "Network configuration file: Missing number of BWP of each CC information\n";
      exit(1);
    }
  double centralFrequenciesBWPs[numTotalBWP], bandwidthBWPs[numTotalBWP], numerologyBWPs[numTotalBWP];
  netConfFile >> keyword;
  if (keyword == "centralFrequencyBWPs")
	  for (uint32_t i = 0; i < numTotalBWP; i++)
		  netConfFile >> centralFrequenciesBWPs[i];
  else
    {
  	  cerr << "Network configuration file: Missing central frequencies of active BWPs information\n";
  	  logFile << "Network configuration file: Missing central frequencies of active BWPs information\n";
      exit(1);
    }
  netConfFile >> keyword;
  if (keyword == "bandwidthBWPs")
	  for (uint32_t i = 0; i < numTotalBWP; i++)
		  netConfFile >> bandwidthBWPs[i];
  else
    {
  	  cerr << "Network configuration file: Missing bandwidth of active BWPs information\n";
  	  logFile << "Network configuration file: Missing bandwidth of active BWPs information\n";
      exit(1);
    }
  netConfFile >> keyword;
  if (keyword == "numerologyBWPs")
  	  for (uint32_t i = 0; i < numTotalBWP; i++)
  		  netConfFile >> numerologyBWPs[i];
  else
    {
      cerr << "Network configuration file: Missing numerology of each BWP information\n";
      logFile << "Network configuration file: Missing numerology of each BWP information\n";
      exit(1);
    }
  netConfFile.close();

  // Check if the frequencies are in the allowed range
  for (uint32_t i = 0; i < numberActiveFrequencyBands; i++)
	  NS_ABORT_IF (centralFrequenciesBands[i] > 100e9);

  /*
  * Instantiation of the radio link spectrum
  * The configured spectrum division is one band with one CC with one BWP per Traffic Profile (TP)
  * For example, if numTrafficProfile = 4:
  * |---------Band0--------|----------Band1--------|----------Band2--------|----------Band3--------|
  * |---------CC0----------|----------CC1----------|----------CC2----------|----------CC3----------|
  * |---------BWP0---------|----------BWP1---------|----------BWP2---------|----------BWP3---------|
  */

  /*
   * ATTENTION!
   * The number of CC has to be lower than 6 (compatibility with 3GPP 5G NR specification).
   * The maximum number of TP is set to 10. To increase this number (up to 21) add elements to the if-else at line 567
   */

  CcBwpCreator ccBwpCreator;
  OperationBandInfo bands[numberActiveFrequencyBands];

  // Manually configure bands, CCs, and BWPs
  uint32_t ccIndex = 0, bwpIndex = 0;
  for (uint32_t i = 0; i < numberActiveFrequencyBands; i++)
    {
	  bands[i].m_bandId = i;
      bands[i].m_centralFrequency  = centralFrequenciesBands[i];
      bands[i].m_channelBandwidth = bandwidthBands[i];
      bands[i].m_lowerFrequency = bands[i].m_centralFrequency - bands[i].m_channelBandwidth / 2;
      bands[i].m_higherFrequency = bands[i].m_centralFrequency + bands[i].m_channelBandwidth / 2;
      for (uint32_t j = 0; j < numCcPerBand[i]; j++)
        {
    	  unique_ptr<ComponentCarrierInfo> cc (new ComponentCarrierInfo ());
          cc->m_ccId = ccIndex;
          cc->m_centralFrequency = centralFrequenciesCCs[ccIndex];
          cc->m_channelBandwidth = bandwidthCCs[ccIndex];
          cc->m_lowerFrequency = cc->m_centralFrequency - cc->m_channelBandwidth / 2;
          cc->m_higherFrequency = cc->m_centralFrequency + cc->m_channelBandwidth / 2;
          for (uint32_t k = 0; k < numBwpPerCC[ccIndex]; k++)
            {
        	  unique_ptr<BandwidthPartInfo> bwp (new BandwidthPartInfo ());
              bwp->m_bwpId = bwpIndex;
              bwp->m_centralFrequency = centralFrequenciesBWPs[bwpIndex];
              bwp->m_channelBandwidth = bandwidthBWPs[bwpIndex];
              bwp->m_lowerFrequency = bwp->m_centralFrequency - bwp->m_channelBandwidth / 2;
              bwp->m_higherFrequency = bwp->m_centralFrequency + bwp->m_channelBandwidth / 2;
              bwp->m_scenario = environment == "RMa_LoS" ? BandwidthPartInfo::RMa_LoS :
                               environment == "UMa_LoS" ? BandwidthPartInfo::UMa_LoS :
                               environment == "UMi_StreetCanyon" ? BandwidthPartInfo::UMi_StreetCanyon :
                               BandwidthPartInfo::RMa_LoS;  // default to RMa_LoS
              cc->AddBwp (move(bwp));
        	  bwpIndex++;
            }
          bands[i].AddCc (move(cc));
          ccIndex++;
        }
    }

  // Instantiation of the gNB and UE nodes
  //NodeContainer allGnbContainer; For combining the terrestrial gNBs and the satellite gNBs. 
  NodeContainer gNbContainer;    //Satellite gNBs
  NodeContainer ueContainers[numTrafficProfile];
  NodeContainer ueGlobalContainer;
  MobilityHelper mobility;
  gNbContainer.Create (numgNB);
  mobility.Install(gNbContainer);
  
  for (uint32_t i = 0; i < numTrafficProfile; i++)
	{
	  ueContainers[i].Create(numUEPerTrafProf[i]);
	  mobility.Install(ueContainers[i]);
	  ueGlobalContainer.Add(ueContainers[i]);
	}


  //Creation of terrestrial gNBs
  NodeContainer terrestrialGnbContainer;   //Terrestrial gNBs 
  terrestrialGnbContainer.Create (numTerrestrialGNBs);

  // Calculate and set fixed positions for terrestrial gNBs
  NodePositions terrestrialGnbPositions[numTerrestrialGNBs];
  CalculateTerrestrialGnbPosition(numTerrestrialGNBs, simDurationS, terrestrialGnbPositions);

  // Set up mobility model for terrestrial gNBs
  mobility.Install(terrestrialGnbContainer);

  // Set the initial positions from the calculated positions
  SetTerrestrialGnbPosition(terrestrialGnbContainer, terrestrialGnbPositions);

  // Calculate and set Satellite gNB and UE positions
  uint32_t roundStartTime = simRoundIndex * simRoundDurationS;
  NodePositions gNBPositions[numgNB];
  double satelliteAltitudes[numgNB];
  CalculategNBPosition(numgNB, simDurationS, gNBPositions, satelliteAltitudes);
  SetgNBPosition(gNbContainer, roundStartTime, gNBPositions);
  double* maximumSatelliteAltitude = max_element(satelliteAltitudes, satelliteAltitudes + numgNB);
  double maximumContactDistances[numgNB];
  for (uint32_t i = 0; i < numgNB; i++)
	  maximumContactDistances[i] = sqrt(pow(satelliteAltitudes[i], 2) + 2 * satelliteAltitudes[i] * EARTH_RADIUS + pow(EARTH_RADIUS, 2) * pow(sin(MINIMUM_ELEVATION_ANGLE),2)) - EARTH_RADIUS * sin(MINIMUM_ELEVATION_ANGLE);
  double* maximumContactDistance = max_element(maximumContactDistances, maximumContactDistances + numgNB);
  NodePositions uePositions[numTotalUE];
  CalculateUEPosition(numTotalUE, simDurationS, uePositions);
  SetUEPosition(ueGlobalContainer, roundStartTime, uePositions);
  

  // Setup of the NR module
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));		// we need it for legacy code (LTE)
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);
  double terrestrialPropagationDelay = 1; // in milliseconds


// Initialize channel and pathloss
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds(0)));
  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));
  nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerTdmaRR"));
  //Essentially a calculated propagation delay between the Satellite gNB and the Core Network
  //uint32_t meanCoreLatency = floor(*maximumSatelliteAltitude + (*maximumContactDistance - *maximumSatelliteAltitude) / 2) / LIGHT_SPEED; // Core latency (between gNB e CN). Currently fixed as a mean value between the minimum contact distance (satellite altitude) and the maximum one()
  //epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (meanCoreLatency)));
  
  //Propagation delay between the terrestrial gNB and the Core Network also the propagation delay between the satellite gNB 
  //and the core network calculated dynamically.
  for (uint32_t i = 0; i < gNbContainer.GetN(); i++) {
    Ptr<Node> gnbNode = gNbContainer.Get(i);
    Ptr<Node> terrestrialgnbNode = terrestrialGnbContainer.Get(i);
    bool isSatelliteGnb = gNbContainer.Contains(gnbNode->GetId()); // Use GetId() to get the node ID
    bool isTerrestrialGnb = terrestrialGnbContainer.Contains(terrestrialgnbNode->GetId());
    if (isSatelliteGnb) {
        uint32_t meanCoreLatency = floor(*maximumSatelliteAltitude + (*maximumContactDistance - *maximumSatelliteAltitude) / 2) / LIGHT_SPEED;
         epcHelper->SetAttribute("S1uLinkDelay", TimeValue(MilliSeconds(meanCoreLatency)));}
    else if (isTerrestrialGnb) {
        epcHelper->SetAttribute("S1uLinkDelay", TimeValue(MilliSeconds(terrestrialPropagationDelay)));
      }
}





  epcHelper->SetAttribute ("S1uLinkDataRate", DataRateValue (DataRate ("1Gb/s")));		// Core data rate (between gNB e CN)
  idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));		// Beamforming method

  // Initialize frequency bands
  for (uint32_t i = 0; i < numberActiveFrequencyBands; i++)
	  nrHelper->InitializeOperationBand (&bands[i]);

  // Antennas for all the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Antennas for all the Satellite gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Antennas for all the Terrestrial gNbs
  // Configure terrestrial gNB antennas
  //tnHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(2));
  //tnHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(2));
  //tnHelper->SetGnbAntennaAttribute("AntennaElement", PointerValue(CreateObject<ThreeGppAntennaModel>()));



  // define a bwpId for each TP and set gNb and UE routing between Bearer and BWP
  // ATTENTION: a couple of lines per defined TP. Add or reduce these lines accordingly
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (0));
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (0));
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_MC_DATA", UintegerValue (1));
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_MC_DATA", UintegerValue (1));
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (2));
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (2));
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_PREMIUM", UintegerValue (3));
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_PREMIUM", UintegerValue (3));

  cout << "NR parameters configured!\n";
  logFile << "NR parameters configured!\n";

  // Read the resource allocation input file
  // Create gNB net devices and allocate the related BWPs depending on the information on the input resource configuration text file
  NetDeviceContainer gnbNetDevContainer;   //satellite gNBsDevContainers
  NetDeviceContainer terrestrialGnbNetDevContainer;   //Terrestrial gNBsDevContainers
  stringstream resAllFileName;
  resAllFileName << inputPath << "resourceAllocation.txt";
  ifstream resAllFile(resAllFileName.str().c_str());
  if (!resAllFile)
    {
	  cerr << "gNB resource configuration: Unable to open file!";
	  logFile << "gNB resource configuration: Unable to open file!";
	  exit(1);
    }
  

  //Terrestrial and Satellite gNB resource allocation 
  uint32_t numUEPergNB[numgNB][numTrafficProfile], numActiveBwpPergNB[numgNB]; // for satellites 
  vector<reference_wrapper<BandwidthPartInfoPtr>> bwpVector[numgNB]; //for satellites
  vector<uint32_t> bwpIDs[numgNB];  //for satellites 
  uint32_t numUEPerTerrestrialGNB[numTerrestrialGNBs][numTrafficProfile], numActiveBwpPerTerrestrialGNB[numTerrestrialGNBs]; //for terrestrial network
  vector<reference_wrapper<BandwidthPartInfoPtr>> bwpVectorTerrestrial[numTerrestrialGNBs]; //for terrestrial network
  vector<uint32_t> bwpIDsTerrestrial[numTerrestrialGNBs];
  bool activegNB[numgNB]; //for satellites
  bool activeTerrestrialGNB[numTerrestrialGNBs]; //for satellites
  resAllFile >> keyword; 
  if (keyword == "satellite"){
    resAllFile >> keyword;  
    if (keyword == "gNB")
      {
	    resAllFile >> keyword;
	    resAllFile >> keyword;
	    for (uint32_t i = 0; i < numgNB; i++)
	      {
		    uint32_t gNBIndex;
		    resAllFile >> gNBIndex;
		    numActiveBwpPergNB[i] = 0;
		    for (uint32_t j = 0; j < numTrafficProfile; j++)
		      {
			      resAllFile >> numUEPergNB[i][j];
			      if (numUEPergNB[i][j] != 0)
				      {
				        auto bwps = bands[j].GetBwps ();
				        bwpVector[i].insert (bwpVector[i].end (), make_move_iterator(bwps.begin ()), make_move_iterator(bwps.end ()));
				        bwpIDs[i].push_back(j);
				        numActiveBwpPergNB[i]++;
				      }  
			    }
		      NetDeviceContainer gnbNetDevContainerTemp;
		      gnbNetDevContainerTemp = nrHelper->InstallGnbDevice (gNbContainer.Get(i), bwpVector[i], bwpIDs[i]);
		      if (gnbNetDevContainerTemp.Get(0) != NULL)
		        {
			      gnbNetDevContainer.Add(gnbNetDevContainerTemp);
			      activegNB[i] = true;
		        } 
		      else{
			      activegNB[i] = false;
          }
        }
        
      }
    resAllFile >> keyword;
    if (keyword == "terrestrial"){
      resAllFile >> keyword;  
      if (keyword == "gNB")
        {
          resAllFile >> keyword;
          resAllFile >> keyword;
          for (uint32_t i = 0; i < numTerrestrialGNBs; i++)
            {
              uint32_t gNBIndex;
              resAllFile >> gNBIndex;
              numActiveBwpPerTerrestrialGNB[i] = 0;
              for (uint32_t j = 0; j < numTrafficProfile; j++)
                {
                  resAllFile >> numUEPerTerrestrialGNB[i][j];
                  if (numUEPerTerrestrialGNB[i][j] != 0)
                    {
                      auto bwps = bands[j].GetBwps ();
                      bwpVectorTerrestrial[i].insert (bwpVectorTerrestrial[i].end (), make_move_iterator(bwps.begin ()), make_move_iterator(bwps.end ()));
                      bwpIDsTerrestrial[i].push_back(j);
                      numActiveBwpPerTerrestrialGNB[i]++;
                    }
                }
              NetDeviceContainer terrestrialgnbNetDevContainerTemp;
              terrestrialgnbNetDevContainerTemp = nrHelper->InstallGnbDevice (terrestrialGnbContainer.Get(i), bwpVectorTerrestrial[i], bwpIDsTerrestrial[i]);
              if (terrestrialgnbNetDevContainerTemp.Get(0) != NULL)
                {
                  terrestrialGnbNetDevContainer.Add(terrestrialgnbNetDevContainerTemp);
                  activeTerrestrialGNB[i] = true;
                }
              else{
                  activeTerrestrialGNB[i] = false;
            }
          }
        }
        else 
          {
            cerr << "gNB resource configuration: Wrong line format!\n";
            logFile << "gNB resource configuration: Wrong line format!\n";
            exit(1);
          }
      }
  else
      {
	    cerr << "gNB resource configuration: Wrong line format!\n";
	    logFile << "gNB resource configuration: Wrong line format!\n";
      exit(1);
      }   
  }




  // attach each UE to the gNB indicated in the resource configuration file
  //string RatType = "terrestrial"; // Change this to "terrestrial" for terrestrial gNBs
  int gNBToAttachWithIndex[numTotalUE];
  uint32_t UETrafficProfileIndex[numTotalUE];
  int terrestrialgNBToAttachWithIndex[numTotalUE];
  //if (RatType == "satellite"){
  resAllFile >> keyword;
  if (keyword == "satellite"){
    resAllFile >> keyword;
      if (keyword == "UE")
      	{
	      resAllFile >> keyword;
	      resAllFile >> keyword;
	      resAllFile >> keyword;
      	}
      else
         {
	      cerr << "1UE resource configuration: Wrong line format!\n";
	      logFile << "UE resource configuration: Wrong line format!\n";
        exit(1);
       }
  //int gNBToAttachWithIndex[numTotalUE];
  //uint32_t UETrafficProfileIndex[numTotalUE];
  for (uint32_t i = 0; i < numTotalUE; i++)
     {
	  uint32_t UEIndex;
	  resAllFile >> UEIndex;
	  if ((UEIndex > (numTotalUE-1)) || (UEIndex < 0))
	    {
		  cerr << "Wrong UE index!\n";
		  logFile << "Wrong UE index!\n";
		  cout << UEIndex << "\n";
		  exit(1);
		}
	  resAllFile >> UETrafficProfileIndex[i];
	  if ((UETrafficProfileIndex[i] > (numTrafficProfile-1)) || (UETrafficProfileIndex[i] < 0))
	    {
		  cerr << "Wrong Traffic profile index!\n";
		  logFile << "Wrong Traffic profile index!\n";
		  cout << UETrafficProfileIndex[i] << "\n";
		  exit(1);
		}
	  resAllFile >> gNBToAttachWithIndex[i];
	  if ((gNBToAttachWithIndex[i] > (int(numgNB)-1)) || (gNBToAttachWithIndex[i] < -1))
		{
		  cerr << "Wrong gNB index!\n";
		  logFile << "Wrong gNB index!\n";
		  cout << gNBToAttachWithIndex[i] << "\n";
		  exit(1);
		}
     }
  }
  else
    {
      cerr << "UE resource configuration: Wrong line format!\n";
      logFile << "UE resource configuration: Wrong line format!\n";
      exit(1);
    }

resAllFile >> keyword;
  if (keyword == "terrestrial")
    {
      cout << "To attach UE to Terrestrial gNBs\n";
      resAllFile >> keyword;
      if (keyword == "UE")
        {
          resAllFile >> keyword;
          resAllFile >> keyword;
          resAllFile >> keyword;
        }
      else
         {
          cerr << "UE resource configuration: Wrong line format!\n";
          logFile << "UE resource configuration: Wrong line format!\n";
          exit(1);
         }
      //int tttachWithIndex[numTotalUE];
      //uint32_t UETrafficProfileIndex[numTotalUE];
      for (uint32_t i = 0; i < numTotalUE; i++)
        {
          uint32_t UEIndex;
          resAllFile >> UEIndex;
          if ((UEIndex > (numTotalUE-1)) || (UEIndex < 0))
            {
              cerr << "Wrong UE index!\n";
              logFile << "Wrong UE index!\n";
              cout << UEIndex << "\n";
              exit(1);
            }
          resAllFile >> UETrafficProfileIndex[i];
          if ((UETrafficProfileIndex[i] > (numTrafficProfile-1)) || (UETrafficProfileIndex[i] < 0))
            {
              cerr << "Wrong Traffic profile index!\n";
              logFile << "Wrong Traffic profile index!\n";
              cout << UETrafficProfileIndex[i] << "\n";
              exit(1);
            }
          resAllFile >> terrestrialgNBToAttachWithIndex[i];
          if ((terrestrialgNBToAttachWithIndex[i] > (int(numTerrestrialGNBs)-1)) || (terrestrialgNBToAttachWithIndex[i] < -1))
            {
              cerr << "Wrong gNB index!\n";
              logFile << "Wrong gNB index!\n";
              cout << terrestrialgNBToAttachWithIndex[i] << "\n";
              exit(1);
            }
        }
      //resAllFile.close();
    }
    else 
      {
        cerr << "UE resource configuration: Wrong line format!\n";
        logFile << "UE resource configuration: Wrong line format!\n";
        exit(1);
      }
    resAllFile.close();



  // Create UE net devices and allocate the related BWPs depending on their TP
  NetDeviceContainer ueNetDevContainers[numTrafficProfile];
  NetDeviceContainer ueNetDevGlobalContainer;
  BandwidthPartInfoPtrVector ueBwpVectors[numTrafficProfile];
  
  // Set the net device attributes
  string pattern = "F|F|F|F|F|F|F|F|F|F|"; // Pattern can be e.g. "DL|S|UL|UL|DL|DL|S|UL|UL|DL|"
  double txPower = 40;
  uint32_t activegNBIndex = 0;
  for (uint32_t i = 0; i < numgNB; i++)
    {
	  uint32_t bwpIndex = 0;
	  bool gNBActive = false;
	  for (uint32_t j = 0; j < numTotalBWP; j++)
	    {
		  if ((numUEPergNB[i][j] != 0) && activegNB[i])
		    {

			  //nrHelper->GetGnbPhy (gnbNetDevContainer.Get (activegNBIndex), j)->SetAttribute ("Numerology", UintegerValue (numerologyBWPs[j]));
        nrHelper->GetGnbPhy(gnbNetDevContainer.Get(activegNBIndex), j)->SetAttribute("Numerology", UintegerValue(1));  // or 0, 2, etc. (just be consistent)
			  nrHelper->GetGnbPhy (gnbNetDevContainer.Get (activegNBIndex), j)->SetAttribute ("TxPower", DoubleValue (txPower));
			  nrHelper->GetGnbPhy (gnbNetDevContainer.Get (activegNBIndex), j)->SetAttribute ("Pattern", StringValue (pattern));
			  bwpIndex++;
			  gNBActive = true;
		    }
		}
	  if (gNBActive){
      std::cout << ">>>Satellite gNB " << i << " is active with " << bwpIndex << " BWPs configured.\n";
      activegNBIndex++;
    }
    else{
      //std::cout << ">>> Satellite gNB " << i << " is inactive.\n";
     // exit(1);
    }
  }
    

  cout << "Satellite gNB and UE net devices configured!\n";
  logFile << "Satellite gNB and UE net devices configured!\n";
  
// Configuring the net device attributes for terrestrial gNBs
string terrestrialPattern = "F|F|F|F|F|F|F|F|F|F|"; // Pattern for terrestrial gNBs
double terrestrialTxPower = 43; // Transmission power for terrestrial gNBs
uint32_t activeTerrestrialGNBIndex = 0;


for (uint32_t i = 0; i < numTrafficProfile; i++)
{
ueBwpVectors[i] = CcBwpCreator::GetAllBwps ({bands[i]});
vector<uint32_t> bwpIDsTerrestrial;
bwpIDsTerrestrial.push_back(i);
ueNetDevContainers[i] = nrHelper->InstallUeDevice (ueContainers[i], ueBwpVectors[i], bwpIDsTerrestrial);
ueNetDevGlobalContainer.Add(ueNetDevContainers[i]);
}




for (uint32_t i = 0; i < numTerrestrialGNBs; i++)
{
    uint32_t bwpIndex = 0;
    bool terrestrialGNBActive = false;

    for (uint32_t j = 0; j < numTotalBWP; j++)
    {
        if ((numUEPerTerrestrialGNB[i][j] != 0) && activeTerrestrialGNB[i])
        {
            nrHelper->GetGnbPhy(terrestrialGnbNetDevContainer.Get(activeTerrestrialGNBIndex), j)
                ->SetAttribute("Numerology", UintegerValue(numerologyBWPs[j]));
            nrHelper->GetGnbPhy(terrestrialGnbNetDevContainer.Get(activeTerrestrialGNBIndex), j)
                ->SetAttribute("TxPower", DoubleValue(terrestrialTxPower));
            nrHelper->GetGnbPhy(terrestrialGnbNetDevContainer.Get(activeTerrestrialGNBIndex), j)
                ->SetAttribute("Pattern", StringValue(terrestrialPattern));
            bwpIndex++;
            terrestrialGNBActive = true;
        }
    }
    if (terrestrialGNBActive)
    {
        std::cout << ">>>Terrestrial gNB " << i << " is active with " << bwpIndex << " BWPs configured.\n";
        activeTerrestrialGNBIndex++;
    }
    else
    {
        //std::cout << ">>> Terrestrial gNB " << i << " is inactive.\n";
        //exit(1);
    }
}

cout << "Terrestrial gNB and UE net devices configured!\n";
logFile << "Terrestrial gNB and UE net devices configured!\n";



  // When all the configuration is done, explicitly call UpdateConfig ()
  //for (auto it = gnbNetDevContainer.Begin(); it != gnbNetDevContainer.End(); ++it)
  //    DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
  //for (uint32_t i = 0; i < numTrafficProfile; i++)
	 // for (auto it = ueNetDevContainers[i].Begin(); it != ueNetDevContainers[i].End(); ++it)
		//  DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();



  // When all the configuration is done, explicitly call UpdateConfig ()
// Update satellite gNBs
for (auto it = gnbNetDevContainer.Begin(); it != gnbNetDevContainer.End(); ++it)
DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();

// Update terrestrial gNBs
for (auto it = terrestrialGnbNetDevContainer.Begin(); it != terrestrialGnbNetDevContainer.End(); ++it)
DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();

// Update UEs
for (uint32_t i = 0; i < numTrafficProfile; i++)
for (auto it = ueNetDevContainers[i].Begin(); it != ueNetDevContainers[i].End(); ++it)
    DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();



  // create the Internet and install the IP stack on the UEs
  // get SGW/PGW and create a single host
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  NodeContainer hostContainer;
  hostContainer.Create (1);
  Ptr<Node> host = hostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (hostContainer);

  // connect a host to pgw. Setup routing too
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, host);
  Ipv4AddressHelper ipv4h;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ptr<Ipv4StaticRouting> hostStaticRouting = ipv4RoutingHelper.GetStaticRouting (host->GetObject<Ipv4> ());
  hostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueGlobalContainer);
  Ipv4InterfaceContainer ueIpInterfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDevGlobalContainer));

  // Set the default gateway for each UE
  for (uint32_t j = 0; j < numTrafficProfile; j++)
    {
	  for (uint32_t i = 0; i < ueContainers[j].GetN(); i++)
   	    {
		  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueContainers[j].Get(i)->GetObject<Ipv4> ());
		  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  	    }
 	}

  
// The code block below attaches each UE to the best gNB (either satellite or terrestrial) based on network evaluation results
for (uint32_t i = 0; i < numTotalUE; i++) {
    bool attached = false;
    
    if (simRoundIndex == 0) {
        // For round 0, prioritize terrestrial gNBs
        if (terrestrialgNBToAttachWithIndex[i] != -1) {
            uint32_t gNBIndexChosen = 0, gNBIndex = terrestrialgNBToAttachWithIndex[i];
            for (uint32_t j = 0; j < numTerrestrialGNBs; j++) {
                if (activeTerrestrialGNB[j]) {
                    if (gNBIndex == j) {
                        nrHelper->AttachToEnb(ueNetDevGlobalContainer.Get(i), terrestrialGnbNetDevContainer.Get(gNBIndexChosen), UETrafficProfileIndex[i]);
                        std::cout << ">>>UE " << i << " attached to Terrestrial gNB " << gNBIndex << "\n";
                        attached = true;
                        break;
                    }
                    gNBIndexChosen++;
                }
            }
        }
        // Try satellite if terrestrial attachment failed
        if (!attached && gNBToAttachWithIndex[i] != -1) {
            uint32_t gNBIndexChosen = 0, gNBIndex = gNBToAttachWithIndex[i];
            for (uint32_t j = 0; j < numgNB; j++) {
                if (activegNB[j]) {
                    if (gNBIndex == j) {
                        nrHelper->AttachToEnb(ueNetDevGlobalContainer.Get(i), gnbNetDevContainer.Get(gNBIndexChosen), UETrafficProfileIndex[i]);
                        std::cout << ">>>UE " << i << " attached to Satellite gNB " << gNBIndex << "\n";
                        attached = true;
                        break;
                    }
                    gNBIndexChosen++;
                }
            }
        }
    } else {
        // For rounds > 0, follow network evaluation results
        if (gNBToAttachWithIndex[i] != -1) {
            uint32_t gNBIndexChosen = 0;
            for (uint32_t j = 0; j < numgNB; j++) {
                if (activegNB[j]) {
                    if (gNBToAttachWithIndex[i] == j) {
                        nrHelper->AttachToEnb(ueNetDevGlobalContainer.Get(i), gnbNetDevContainer.Get(gNBIndexChosen), UETrafficProfileIndex[i]);
                        std::cout << ">>>UE " << i << " attached to Satellite gNB " << j << "\n";
                        attached = true;
                        break;
                    }
                    gNBIndexChosen++;
                }
            }
        }
        // Try terrestrial if satellite attachment failed
        if (!attached && terrestrialgNBToAttachWithIndex[i] != -1) {
            uint32_t gNBIndexChosen = 0;
            for (uint32_t j = 0; j < numTerrestrialGNBs; j++) {
                if (activeTerrestrialGNB[j]) {
                    if (terrestrialgNBToAttachWithIndex[i] == j) {
                        nrHelper->AttachToEnb(ueNetDevGlobalContainer.Get(i), terrestrialGnbNetDevContainer.Get(gNBIndexChosen), UETrafficProfileIndex[i]);
                        std::cout << ">>>UE " << i << " attached to Terrestrial gNB " << j << "\n";
                        attached = true;
                        break;
                    }
                    gNBIndexChosen++;
                }
            }
        }
    }
    
    if (!attached) {
        std::cout << "❌ UE " << i << " could not be attached to any gNB.\n";
    }
}

























cout << "UE-gNB attachments done!\n";
  logFile << "UE-gNB attachments done!\n";
  
  // Instantiation of the data traffic flows
  // define numTrafficProfile classes of CBR traffic flows with different packet generation rate
  cout << "Traffic flow application configuration!\n";
  logFile << "Traffic flow application configuration!\n";
  ApplicationContainer serverApps, clientApps;
  uint16_t dlPort;
  uint32_t packetSizeBit, packetInterGenerationTimeMs;
  // Read the devices per UE input file
  stringstream devicesPerUEFileName;
  devicesPerUEFileName << inputPath << "devicesPerUE.txt";
  ifstream devicesPerUEFile(devicesPerUEFileName.str().c_str());
  if (!devicesPerUEFile)
    {
	  cerr << "Devices per UE file: Unable to open file!";
	  logFile << "Devices per UE file: Unable to open file!";
      exit(1);
    }
  uint32_t numDevicesPerUE[numTotalUE];
  for (uint32_t i = 0; i < numTotalUE; i++)
	  devicesPerUEFile >> numDevicesPerUE[i];
  // Read the traffic flow statistics input file
  stringstream trafficFlowFileName;
  trafficFlowFileName << inputPath << "trafficFlowParameters.txt";
  ifstream trafficFlowFile(trafficFlowFileName.str().c_str());
  if (!trafficFlowFile)
    {
	  cerr << "Traffic flow parameter file: Unable to open file!";
	  logFile << "Traffic flow parameter file: Unable to open file!";
      exit(1);
    }
  trafficFlowFile >> keyword;
  if (keyword == "TFIndex") {
	  trafficFlowFile >> keyword;
	  trafficFlowFile >> keyword;
  	  for (uint32_t i = 0; i < numTrafficProfile; i++)
  	    {
  		  trafficFlowFile >> keyword;
  		  trafficFlowFile >> packetSizeBit;
  		  trafficFlowFile >> packetInterGenerationTimeMs;
  		  enum EpsBearer::Qci bearer;	// The bearer that will carry the i-th TP flows
  		  switch (i) {	// set a different port, packet size, and packet inter-generation time per TF
  		  case 0:
  			  dlPort = 1000;
  			  bearer = EpsBearer::GBR_NMC_PUSH_TO_TALK;
  			  break;
  		  case 1:
  			  dlPort = 2000;
  			  bearer = EpsBearer::NGBR_MC_DELAY_SIGNAL;
  			  break;
  		  case 2:
  			  dlPort = 3000;
  			  bearer = EpsBearer::NGBR_MC_DATA;
  			  break;
  		  case 3:
  			  dlPort = 4000;
  			  bearer = EpsBearer::GBR_V2X;
  			  break;
  		  case 4:
  			  dlPort = 5000;
  			  bearer = EpsBearer::GBR_NON_CONV_VIDEO;
  			  break;
  		  }
  		  UdpServerHelper dlServerApp(dlPort);
  		  serverApps.Add (dlServerApp.Install (host));
  		  UdpClientHelper dlClientApp;
  		  dlClientApp.SetAttribute ("RemotePort", UintegerValue (dlPort));
  		  dlClientApp.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  		  dlClientApp.SetAttribute ("PacketSize", UintegerValue (packetSizeBit));
  		  dlClientApp.SetAttribute ("Interval", TimeValue (MilliSeconds (packetInterGenerationTimeMs / numDevicesPerUE[i])));
  		  Address hostAddress = internetIpIfaces.GetAddress(1);
  		  // The client, who is transmitting, is installed in the UE, with destination address set as the remote host address
  		  dlClientApp.SetAttribute ("RemoteAddress", AddressValue (hostAddress));
  		  // The filter for the i-th TP's data flows
  		  Ptr<EpcTft> dlTft = Create<EpcTft> ();
  		  EpcTft::PacketFilter dlPf;
  		  dlPf.localPortStart = dlPort;
  		  dlPf.localPortEnd = dlPort;
  		  dlTft->Add (dlPf);
  		  // Install the Client application on the UEs
  		  for (uint32_t j = 0; j < ueContainers[i].GetN (); j++)
  		    {
  			  Ptr<NetDevice> ueDevice = ueNetDevContainers[i].Get(j);
  			  // Activate a dedicated bearer for the i-th TP
  			  EpsBearer dlBearer (bearer);
  			  nrHelper->ActivateDedicatedEpsBearer (ueDevice, dlBearer, dlTft);
  		    }
  		  clientApps.Add (dlClientApp.Install (ueContainers[i]));
  	    }
  }
  else
    {
	  cerr << "Traffic flow parameter file: Wrong file format!\n";
	  logFile << "Traffic flow parameter file: Wrong file format!\n";
      exit(1);
    }
  trafficFlowFile.close();
   epcHelper->AddX2Interface (gNbContainer.Get(0), gNbContainer.Get(1));	// to enable in case of inter-satellite links

  // start server and client apps
  serverApps.Start(MilliSeconds(200.0));
  clientApps.Start(MilliSeconds(500.0));
  serverApps.Stop(MilliSeconds(simRoundDurationMs - 500));
  clientApps.Stop(MilliSeconds(simRoundDurationMs - 500));

  cout << "The network has been created and configured!\n";
  logFile << "The network has been created and configured!\n";

  // enable the traces provided by the NR module
  //nrHelper->EnableTraces();
  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (host);
  endpointNodes.Add (ueGlobalContainer);

  Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));
   

  // Set up UDP trace connections before starting the simulation
  NS_LOG_INFO("Setting up UDP trace connections");
  for (uint32_t i = 0; i < clientApps.GetN(); i++) {
      Ptr<Application> app = clientApps.Get(i);
      Ptr<UdpClient> client = DynamicCast<UdpClient>(app);
      if (client) {
          NS_LOG_INFO("Connecting trace for UDP client " << i);
      }
  }

  for (uint32_t i = 0; i < serverApps.GetN(); i++) {
      Ptr<Application> app = serverApps.Get(i);
      Ptr<UdpServer> server = DynamicCast<UdpServer>(app);
      if (server) {
          NS_LOG_INFO("Connecting trace for UDP server " << i);
      }
  }

  // execute simulation
  Simulator::Stop (MilliSeconds (simRoundDurationMs));
  cout << "End Simulation preparation\n";
  cout << "\n";
  cout << "Start Simulation\n";
  logFile << "End Simulation preparation\n";
  logFile << "\n";
  logFile << "Start Simulation\n";
  logFile.close();
  PrintSimulationStatus(simRoundDurationS);

  // Initialize logging files
  positionLogPath = outputPath + "node_positions.csv";
  positionLogFile.open(positionLogPath);
  positionLogFile << "Time,NodeId,X,Y,Z\n";

  // Schedule periodic position logging for all nodes
  auto logPositions = [&gNbContainer, &terrestrialGnbContainer, &ueGlobalContainer]() {
    // Log satellite gNB positions
    for (uint32_t i = 0; i < gNbContainer.GetN(); i++) {
      Ptr<MobilityModel> mobility = gNbContainer.Get(i)->GetObject<MobilityModel>();
      if (mobility) {
        Vector pos = mobility->GetPosition();
        std::stringstream data;
        data << gNbContainer.Get(i)->GetId() << "," << pos.x << "," << pos.y << "," << pos.z;
        LogNodePosition(gNbContainer.Get(i)->GetId(), pos);
      }
    }
    
    // Log terrestrial gNB positions
    for (uint32_t i = 0; i < terrestrialGnbContainer.GetN(); i++) {
      Ptr<MobilityModel> mobility = terrestrialGnbContainer.Get(i)->GetObject<MobilityModel>();
      if (mobility) {
        Vector pos = mobility->GetPosition();
        std::stringstream data;
        data << terrestrialGnbContainer.Get(i)->GetId() << "," << pos.x << "," << pos.y << "," << pos.z;
        LogNodePosition(terrestrialGnbContainer.Get(i)->GetId(), pos);
      }
    }
    
    // Log UE positions
    for (uint32_t i = 0; i < ueGlobalContainer.GetN(); i++) {
      Ptr<MobilityModel> mobility = ueGlobalContainer.Get(i)->GetObject<MobilityModel>();
      if (mobility) {
        // Use a unique ID for UEs that doesn't overlap with gNBs
        Vector pos = mobility->GetPosition();
        std::stringstream data;
        data << i + 1000 << "," << pos.x << "," << pos.y << "," << pos.z;
        LogNodePosition(i + 1000, pos);  // Start UE IDs from 1000
      }
    }
  };

  // Schedule initial logging
  Simulator::Schedule(Seconds(0.0), logPositions);

  // Schedule periodic logging every 100ms
  for (double t = 0.1; t < simDurationS; t += 0.1) {
    Simulator::Schedule(Seconds(t), logPositions);
  }

  // Close log file at the end
  Simulator::Schedule(Seconds(simDurationS), []() {
    if (positionLogFile.is_open()) {
      positionLogFile.close();
    }
  });

  // Log initial node positions
  for (uint32_t i = 0; i < gNbContainer.GetN(); i++) {
      Vector pos = gNbContainer.Get(i)->GetObject<MobilityModel>()->GetPosition();
      LogNodePosition(i, pos);
  }

  for (uint32_t i = 0; i < terrestrialGnbContainer.GetN(); i++) {
      Vector pos = terrestrialGnbContainer.Get(i)->GetObject<MobilityModel>()->GetPosition();
      LogNodePosition(i + gNbContainer.GetN(), pos);
  }

  for (uint32_t tpIndex = 0; tpIndex < numTrafficProfile; tpIndex++) {
      for (uint32_t i = 0; i < ueNetDevContainers[tpIndex].GetN(); i++) {
          Ptr<NetDevice> netDevice = ueNetDevContainers[tpIndex].Get(i);
          Ptr<Node> node = netDevice->GetNode();
          Vector pos = node->GetObject<MobilityModel>()->GetPosition();
          LogNodePosition(i + 1000, pos);  // Start UE IDs from 1000
      }
  }

  // Schedule periodic position updates
  auto updatePositions = [&]() {
      for (uint32_t i = 0; i < gNbContainer.GetN(); i++) {
          Vector pos = gNbContainer.Get(i)->GetObject<MobilityModel>()->GetPosition();
          LogNodePosition(i, pos);
      }

      for (uint32_t i = 0; i < terrestrialGnbContainer.GetN(); i++) {
          Vector pos = terrestrialGnbContainer.Get(i)->GetObject<MobilityModel>()->GetPosition();
          LogNodePosition(i + gNbContainer.GetN(), pos);
      }

      for (uint32_t tpIndex = 0; tpIndex < numTrafficProfile; tpIndex++) {
          for (uint32_t i = 0; i < ueNetDevContainers[tpIndex].GetN(); i++) {
              Ptr<NetDevice> netDevice = ueNetDevContainers[tpIndex].Get(i);
              Ptr<Node> node = netDevice->GetNode();
              Vector pos = node->GetObject<MobilityModel>()->GetPosition();
              LogNodePosition(i + 1000, pos);  // Start UE IDs from 1000
          }
      }
  };

  // Schedule position updates every 100ms
  for (double t = 0.1; t < simRoundDurationMs/1000.0; t += 0.1) {
      Simulator::Schedule(Seconds(t), updatePositions);
  }

  // Log connections
  for (uint32_t i = 0; i < ueNetDevGlobalContainer.GetN(); i++) {
      Ptr<NetDevice> ueDevice = ueNetDevGlobalContainer.Get(i);
      Ptr<NrUeNetDevice> nrUeDevice = DynamicCast<NrUeNetDevice>(ueDevice);
      if (nrUeDevice) {
          uint64_t imsi = nrUeDevice->GetImsi();
          uint16_t cellId = nrUeDevice->GetCellId();
          // Connection logging is handled by the visualization system
      }
  }

  // Set up NetAnim
AnimationInterface anim("simulation-output.xml"); // Output file for NetAnim

// Assign node descriptions and colors
for (uint32_t i = 0; i < gNbContainer.GetN(); i++) {
    anim.UpdateNodeDescription(gNbContainer.Get(i), "Satellite gNB " + std::to_string(i));
    anim.UpdateNodeColor(gNbContainer.Get(i), 255, 0, 0); // Red for satellite gNBs
}

for (uint32_t i = 0; i < terrestrialGnbContainer.GetN(); i++) {
    anim.UpdateNodeDescription(terrestrialGnbContainer.Get(i), "Terrestrial gNB " + std::to_string(i));
    anim.UpdateNodeColor(terrestrialGnbContainer.Get(i), 0, 255, 0); // Green for terrestrial gNBs
}
/** 
for (uint32_t i = 0; i < ueGlobalContainer.GetN(); i++) {
    anim.UpdateNodeDescription(ueGlobalContainer.Get(i), "UE " + std::to_string(i));
    anim.UpdateNodeColor(ueGlobalContainer.Get(i), 0, 0, 255); // Blue for UEs
    
}
*/    

for (uint32_t tpIndex = 0; tpIndex < numTrafficProfile; tpIndex++) {
  for (uint32_t i = 0; i < ueNetDevContainers[tpIndex].GetN(); i++) {
      Ptr<NetDevice> netDevice = ueNetDevContainers[tpIndex].Get(i);
      Ptr<Node> node = netDevice->GetNode(); // Retrieve the associated Node
      anim.UpdateNodeDescription(node, "UE " + std::to_string(i));
      anim.UpdateNodeColor(node, 0, 0, 255); // Blue for UEs
  }
}


// Label the PGW node
anim.UpdateNodeDescription(pgw, "PGW");
anim.UpdateNodeColor(pgw, 255, 255, 0); // Yellow for the PGW



Packet::EnablePrinting();
PacketMetadata::Enable();
anim.EnablePacketMetadata(true);


  Simulator::Run ();

  logFile.open(logFileName.str().c_str(), ofstream::app);
  logFile << "Simulation progress: " << simRoundDurationS << " 100%\n";
  logFile << "End Simulation\n";
  logFile << "It took: " << (time(NULL) - startTime) / 60 << " min and " << (time(NULL) - startTime) % 60 << " sec\n";
  logFile << "\n";
  logFile.close();
  cout << "Simulation progress: " << simRoundDurationS << " 100%\n";
  cout << "End Simulation\n";
  cout << "It took: " << (time(NULL) - startTime) / 60 << "min and " << (time(NULL) - startTime) % 60 << " sec\n";
  cout << "\n";

  // Creation of the network status output text file
  uint32_t currentTime = (simRoundIndex + 1) * simRoundDurationS;
  stringstream networkStatusFileName;
  networkStatusFileName << outputPath << "networkStatus.txt";
  ofstream networkStatusFile(networkStatusFileName.str().c_str(), ofstream::out);
  if (!networkStatusFile)
	{
	  cerr << "Network current status file: Unable to create file!";
	  exit(1);
	}
  else
	{
//	  networkStatusFile << "IndexUE IndexgNB Distance(km) ElevationAngle(deg) RemainingContactTime(sec)\n";
	  for (uint32_t i = 0; i < ueGlobalContainer.GetN(); i++)
		{
		  uint32_t minDistance = 100000;
		  Vector UEPosition = ueGlobalContainer.Get(i)->GetObject<MobilityModel> ()->GetPosition ();
		  for (uint32_t j = 0; j < gNbContainer.GetN(); j++)
			{
			  Vector gNBPosition = gNbContainer.Get(j)->GetObject<MobilityModel> ()->GetPosition ();
			  double distance = CalculateDistance(gNBPosition, UEPosition);
			  if (distance < minDistance)
				  minDistance = distance;
			  uint32_t remainingContactTime = 0;
			  double elevationAngle = -M_PI / 2;
			  double elevationAngleArg = (pow(satelliteAltitudes[j],2) + (2 * EARTH_RADIUS * satelliteAltitudes[j]) - pow(distance,2)) / (2 * EARTH_RADIUS * distance);
			  if (elevationAngleArg > -1)
				  elevationAngle = asin((pow(satelliteAltitudes[j],2) + (2 * EARTH_RADIUS * satelliteAltitudes[j]) - pow(distance,2)) / (2 * EARTH_RADIUS * distance));	// Eq. 50, "Earth-referenced aircraft navigation and surveillance analysis
			  elevationAngle = elevationAngle * 180 / M_PI;
			  if (distance < maximumContactDistances[j])
				{
				  bool done = false;
				  int t2 = 1;
				  do
					{
					  // we proceed computing the remaining contact time only if we are not already at (or until we reach) the simulation end
					  if ((currentTime + t2) < simDurationS)
						{
						  double dist_temp = sqrt(pow(uePositions[i].coordinates[currentTime+t2][0] - gNBPositions[j].coordinates[currentTime+t2][0],2) + pow(uePositions[i].coordinates[currentTime+t2][1] - gNBPositions[j].coordinates[currentTime+t2][1],2) + pow(uePositions[i].coordinates[currentTime+t2][2] - gNBPositions[j].coordinates[currentTime+t2][2],2));
						  // we stop as soon as the computed distance becomes higher than the maximum distance threshold
						  if (dist_temp > maximumContactDistances[j])
							{
							  remainingContactTime = t2;
							  done = true;
							}
						  t2++;
						}
					  else
						{
						  remainingContactTime = t2;
						  done = true;
						}
					}
				  while(!done);
				}
			  networkStatusFile << i << " " << j << " " << distance << " " << elevationAngle << " " << remainingContactTime << "\n";
			}
		}
	}
  networkStatusFile.close();
  ifstream src(networkStatusFileName.str().c_str(), ios::binary);
  string networkStatusFileNamePerRound = outputPath + "networkStatus_" + to_string(simRoundIndex) + ".txt";
  ofstream dest(networkStatusFileNamePerRound.c_str(), ios::binary);
  dest << src.rdbuf();







//Evaluating the network conditions to determine the best gNB to attach to for handover if required.
// Open the previous resource allocation file for reading
//std::ifstream prevResourceFile("resourceAllocationPrevious.txt");
// Rename current resource allocation file in previous resource allocation file (needed to count the number of handovers between consecutive rounds)

stringstream prevResAllFileName;
prevResAllFileName << inputPath << "resourceAllocationPrevious.txt";
rename(resAllFileName.str().c_str(), prevResAllFileName.str().c_str());
ifstream prevResAllFile(prevResAllFileName.str().c_str());
if (!prevResAllFile.is_open())
{
    std::cerr << "Error: Unable to open previous resource allocation file for reading!\n";
    return 1;
}

// Open the new resource allocation file for writing
stringstream ResAllFileName;
ResAllFileName << inputPath << "resourceAllocation.txt";
ofstream resourceFile(ResAllFileName.str().c_str(), ofstream::out | ofstream::trunc);
if (!resourceFile.is_open())
{
    std::cerr << "Error: Unable to open resource allocation file for writing!\n";
    return 1;
}

std::string line;
bool isSatelliteGnbBlock = false;
bool isTerrestrialGnbBlock = false;

// Copy the satellite and terrestrial gNB blocks
while (std::getline(prevResAllFile, line))
{
    if (line == "satellite")
    {
        isSatelliteGnbBlock = true;
        isTerrestrialGnbBlock = false;
        resourceFile << line << "\n"; // Write the "satellite" header
        continue;
    }
    else if (line == "terrestrial")
    {
        isSatelliteGnbBlock = false;
        isTerrestrialGnbBlock = true;
        resourceFile << line << "\n"; // Write the "terrestrial" header
        continue;
    }

    if (isSatelliteGnbBlock || isTerrestrialGnbBlock)
    {
        resourceFile << line << "\n"; // Write the gNB block lines
    }

    // Stop copying when reaching the UE block
    if (line == "UE UEIndex UETrafficProfile gNBIndexToAttach")
    {
        break;
    }
}

// Write the header for the satellite UE block
//resourceFile << "satellite\n";
//resourceFile << "UE UEIndex UETrafficProfile gNBIndexToAttach\n";

// Iterate over all UEs in the network to update the satellite UE block
for (uint32_t i = 0; i < numTotalUE; i++) {
    double maxRSSI = -std::numeric_limits<double>::max();
    uint32_t bestSatGNBIndex = -1;
    uint32_t bestTerGNBIndex = -1;
    bool useSatellite = false;
    
    // Get the position of the current UE
    Vector uePosition = ueGlobalContainer.Get(i)->GetObject<MobilityModel>()->GetPosition();
    
    // Check satellite gNBs
    for (uint32_t j = 0; j < numgNB; j++) {
        if (activegNB[j]) {
            Vector gnbPosition = gNbContainer.Get(j)->GetObject<MobilityModel>()->GetPosition();
            double distance = CalculateDistance(uePosition, gnbPosition);
            
            // Calculate FSPL
            double frequency = centralFrequenciesBands[0];
            double fspl = 20 * log10(distance/1000) + 20 * log10(frequency) + 20 * log10(4 * M_PI / LIGHT_SPEED);
            
            // Calculate RSSI
            double Gt = 40; // Satellite gNB gain
            double Gr = 0;  // UE gain
            double rssi = txPower - fspl + Gt + Gr;
            
            if (rssi > maxRSSI) {
                maxRSSI = rssi;
                bestSatGNBIndex = j;
                useSatellite = true;
            }
        }
    }
    
    // Check terrestrial gNBs
    for (uint32_t j = 0; j < numTerrestrialGNBs; j++) {
        if (activeTerrestrialGNB[j]) {
            Vector gnbPosition = terrestrialGnbContainer.Get(j)->GetObject<MobilityModel>()->GetPosition();
            double distance = CalculateDistance(uePosition, gnbPosition);
            
            // Calculate FSPL
            double frequency = centralFrequenciesBands[0];
            double fspl = 20 * log10(distance/1000) + 20 * log10(frequency) + 20 * log10(4 * M_PI / LIGHT_SPEED);
            
            // Calculate RSSI
            double Gt = 18; // Terrestrial gNB gain
            double Gr = 0;  // UE gain
            double rssi = terrestrialTxPower - fspl + Gt + Gr;
            
            if (rssi > maxRSSI) {
                maxRSSI = rssi;
                bestTerGNBIndex = j;
                useSatellite = false;
            }
        }
    }
    
    // Write the result to the resource file
    if (useSatellite) {
        resourceFile << i << " " << UETrafficProfileIndex[i] << " " << bestSatGNBIndex << "\n";
        //std::cout << ">>>UE " << i << " will attach to Satellite gNB " << bestSatGNBIndex 
                  //<< " (RSSI: " << maxRSSI << " dBm)\n";
    } else {
        resourceFile << i << " " << UETrafficProfileIndex[i] << " -1\n"; // -1 for satellite attachment
        //std::cout << ">>>UE " << i << " will attach to Terrestrial gNB " << bestTerGNBIndex 
                 // << " (RSSI: " << maxRSSI << " dBm)\n";
    }
}

resourceFile << "\n";
// Write the header for the terrestrial UE block
resourceFile << "terrestrial\n";
resourceFile << "UE UEIndex UETrafficProfile gNBIndexToAttach\n";

// Write terrestrial UE attachments
for (uint32_t i = 0; i < numTotalUE; i++) {
    double maxRSSI = -std::numeric_limits<double>::max();
    uint32_t bestSatGNBIndex = -1;
    uint32_t bestTerGNBIndex = -1;
    bool useSatellite = false;
    
    // Get the position of the current UE
    Vector uePosition = ueGlobalContainer.Get(i)->GetObject<MobilityModel>()->GetPosition();
    
    // Check satellite gNBs
    for (uint32_t j = 0; j < numgNB; j++) {
        if (activegNB[j]) {
            Vector gnbPosition = gNbContainer.Get(j)->GetObject<MobilityModel>()->GetPosition();
            double distance = CalculateDistance(uePosition, gnbPosition);
            
            // Calculate FSPL
            double frequency = centralFrequenciesBands[0];
            double fspl = 20 * log10(distance/1000) + 20 * log10(frequency) + 20 * log10(4 * M_PI / LIGHT_SPEED);
            
            // Calculate RSSI
            double Gt = 40; // Satellite gNB gain
            double Gr = 0;  // UE gain
            double rssi = txPower - fspl + Gt + Gr;
            
            if (rssi > maxRSSI) {
                maxRSSI = rssi;
                bestSatGNBIndex = j;
                useSatellite = true;
            }
        }
    }
    
    // Check terrestrial gNBs
    for (uint32_t j = 0; j < numTerrestrialGNBs; j++) {
        if (activeTerrestrialGNB[j]) {
            Vector gnbPosition = terrestrialGnbContainer.Get(j)->GetObject<MobilityModel>()->GetPosition();
            double distance = CalculateDistance(uePosition, gnbPosition);
            
            // Calculate FSPL
            double frequency = centralFrequenciesBands[0];
            double fspl = 20 * log10(distance/1000) + 20 * log10(frequency) + 20 * log10(4 * M_PI / LIGHT_SPEED);
            
            // Calculate RSSI
            double Gt = 18; // Terrestrial gNB gain
            double Gr = 0;  // UE gain
            double rssi = terrestrialTxPower - fspl + Gt + Gr;
            
            if (rssi > maxRSSI) {
                maxRSSI = rssi;
                bestTerGNBIndex = j;
                useSatellite = false;
            }
        }
    }
    
    // Write the result to the resource file
    if (!useSatellite) {
        resourceFile << i << " " << UETrafficProfileIndex[i] << " " << bestTerGNBIndex << "\n";
        std::cout << ">>>UE " << i << " will attach to Terrestrial gNB " << bestTerGNBIndex 
                  << " (RSSI: " << maxRSSI << " dBm)\n";
    } else {
        resourceFile << i << " " << UETrafficProfileIndex[i] << " -1\n"; // -1 for terrestrial attachment
        std::cout << ">>>UE " << i << " will attach to Satellite gNB " << bestSatGNBIndex 
                  << " (RSSI: " << maxRSSI << " dBm)\n";
    }
}

std::cout << ">>>Resource allocation file updated successfully.\n";





  // Count the number of satellite handovers
  int prevgNBToAttachWithIndex;
  int prevTergNBToAttachWithIndex;
  int totalHandovers = 0;
  uint32_t numberOfHandovers = 0;
  uint32_t numberOfTerHandovers = 0;
  //stringstream prevResAllFileName;
  //prevResAllFileName << inputPath << "resourceAllocationPrevious.txt";
  //ifstream prevResAllFile(prevResAllFileName.str().c_str());
  if (prevResAllFile)	// the current simulation round is not the first one
    {
    prevResAllFile >> keyword;
	  prevResAllFile >> keyword;
	  prevResAllFile >> keyword;
	  prevResAllFile >> keyword;
	  for (uint32_t i = 0; i < numgNB; i++)
	    {
		  for (uint32_t j = 0; j <= numTrafficProfile; j++)
		    {
			  prevResAllFile >> keyword;
		    }
	    }
    prevResAllFile >> keyword;
	  prevResAllFile >> keyword;
	  prevResAllFile >> keyword;
	  prevResAllFile >> keyword;
	  prevResAllFile >> keyword;
     for (uint32_t a = 0; a < numTerrestrialGNBs; a++)
    {
        for (uint32_t b = 0; b <= numTrafficProfile; b++)
        {
            prevResAllFile >> keyword;
        }
    }
     // Process satellite UE attachments
    prevResAllFile >> keyword;
    prevResAllFile >> keyword;  // "satellite"
    prevResAllFile >> keyword;  // "UE"
    prevResAllFile >> keyword;  // "UEIndex"
    prevResAllFile >> keyword;  // "UETrafficProfile"
    prevResAllFile >> keyword;  // "gN
	  for (uint32_t k = 0; k < numTotalUE; k++)
	    {
		  prevResAllFile >> keyword;
		  prevResAllFile >> keyword;
		  prevResAllFile >> prevgNBToAttachWithIndex;
		  if (prevgNBToAttachWithIndex != gNBToAttachWithIndex[k])
			  numberOfHandovers++;
	    }
     // Process terrestrial UE attachments
    prevResAllFile >> keyword;
    prevResAllFile >> keyword;  // "terrestrial"
    prevResAllFile >> keyword;  // "UE"
    prevResAllFile >> keyword;  // "UEIndex"
    prevResAllFile >> keyword;  // "UETrafficProfile"
    prevResAllFile >> keyword;  // "gNBIndexToAttach"

    for (uint32_t y = 0; y < numTotalUE; y++)
    {
        prevResAllFile >> keyword;  // UE index
        prevResAllFile >> keyword;  // Traffic profile
        prevResAllFile >> prevTergNBToAttachWithIndex;
        if (prevTergNBToAttachWithIndex != terrestrialgNBToAttachWithIndex[y])
            numberOfTerHandovers++;
    }
    totalHandovers = numberOfHandovers + numberOfTerHandovers;
    }

  // Creation of the performance results output text file
  monitor->CheckForLostPackets ();
  ofstream performanceResultsFile;
  string performanceResultsFileName = outputPath + "/performanceResults.txt";
  performanceResultsFile.open (performanceResultsFileName.c_str (), ofstream::out | ofstream::trunc);
  if (!performanceResultsFile.is_open ())
    {
      cerr << "Can't open file " << performanceResultsFileName << "\n";
      return 1;
    }
  performanceResultsFile.setf (ios_base::fixed);
  
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  double averageFlowDeliveryTime = 0, averageFlowJitter = 0, averageSatelliteLinkThroughput = 0, averageTerrestrialLinkThroughput = 0;
  performanceResultsFile << "Average delivery time per traffic flow [ms]\n";

  for (map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      if (i->second.rxPackets > 0)
        {
          // Measure the packets' delivery time [ms]
          double flowDeliveryTime = 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets;
          performanceResultsFile << flowDeliveryTime << " ";
          averageFlowDeliveryTime += flowDeliveryTime;
        }
      else
    	  performanceResultsFile << "0 ";
    }
  performanceResultsFile << "\n";
  performanceResultsFile << "Average delivery time [ms]\n";
  performanceResultsFile << (averageFlowDeliveryTime / stats.size()) << "\n";
  performanceResultsFile << "Jitter per traffic flow [ms]\n";
  for (map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      if (i->second.rxPackets > 0)
        {
          // Measure the packets' jitter [ms]
          double flowJitter = 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets;
          performanceResultsFile << flowJitter << " ";
          averageFlowJitter += flowJitter;
        }
      else
    	  performanceResultsFile << "0 ";
    }
  performanceResultsFile << "\n";
  performanceResultsFile << "Average jitter [ms]\n";
  performanceResultsFile << (averageFlowJitter / stats.size()) << "\n";
  /** 
  performanceResultsFile << "Aggregated throughput per satellite channel [Mbps]\n";
  double satelliteLinkThroughput[numgNB];
  for (uint32_t l = 0; l < numgNB; l++)
	  satelliteLinkThroughput[l] = 0;
  uint32_t k = 0;
  for (map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      if (i->second.rxPackets > 0)
        {
          // Measure the satellite link throughput [Mbps]
    	  double rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();
    	  double flowThoughput = i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000;
    	  satelliteLinkThroughput[gNBToAttachWithIndex[k]] += flowThoughput;
        }
      k++;
    }
  for (uint32_t l = 0; l < numgNB; l++)
    {
	  performanceResultsFile << satelliteLinkThroughput[l] << " ";
	  averageSatelliteLinkThroughput += satelliteLinkThroughput[l];
    }
  performanceResultsFile << "\n";
  performanceResultsFile << "Average satellite aggregated throughput [Mbps]\n";
  performanceResultsFile << averageSatelliteLinkThroughput / numgNB << "\n";
  */




  performanceResultsFile << "Cell occupancy per satellite per carrier\n";
  uint32_t cellOccupancyMatrix[numTrafficProfile][numgNB], cellOccupancyPerSat[numgNB];
  for (uint32_t i = 0; i < numgNB; i++)
    {
	  for (uint32_t j = 0; j < numTrafficProfile; j++)
		  cellOccupancyMatrix[j][i] = 0;
	  cellOccupancyPerSat[i] = 0;
    }
  for (uint32_t k = 0; k < numTotalUE; k++)
	  cellOccupancyMatrix[UETrafficProfileIndex[k]][gNBToAttachWithIndex[k]] += numDevicesPerUE[k];

  for (uint32_t j = 0; j < numTrafficProfile; j++)
    {
	  for (uint32_t i = 0; i < numgNB; i++)
		{
		  performanceResultsFile << cellOccupancyMatrix[j][i] << " ";
		  cellOccupancyPerSat[i] += cellOccupancyMatrix[j][i];
	    }
	  performanceResultsFile << "\n";
    }
  performanceResultsFile << "Cell occupancy per satellite\n";
  for (uint32_t i = 0; i < numgNB; i++)
	  performanceResultsFile << cellOccupancyPerSat[i] << " ";
  performanceResultsFile << "\n";
  //performanceResultsFile << "Number of handovers\n";
  //performanceResultsFile << numberOfHandovers;
  // Update the performance results file to include both types of handovers
  
    /** 
  performanceResultsFile << "Aggregated throughput per terrestrial channel [Mbps]\n";
  double terrestrialLinkThroughput[numTerrestrialGNBs];
  for (uint32_t l = 0; l < numTerrestrialGNBs; l++)
	  terrestrialLinkThroughput[l] = 0;
  uint32_t q = 0;
  for (map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      if (i->second.rxPackets > 0)
        {
          // Measure the satellite link throughput [Mbps]
    	  double rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();
    	  double flowThoughput = i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000;
    	  terrestrialLinkThroughput[terrestrialgNBToAttachWithIndex[q]] += flowThoughput;
        }
      q++;
    }
  for (uint32_t l = 0; l < numTerrestrialGNBs; l++)
    {
	  performanceResultsFile << terrestrialLinkThroughput[l] << " ";
	  averageTerrestrialLinkThroughput += terrestrialLinkThroughput[l];
    }
  performanceResultsFile << "\n";
  performanceResultsFile << "Average terrestrial aggregated throughput [Mbps]\n";
  performanceResultsFile << averageTerrestrialLinkThroughput / numTerrestrialGNBs << "\n";
*/

double satelliteLinkThroughput[numgNB] = {0};
double terrestrialLinkThroughput[numTerrestrialGNBs] = {0};

uint32_t k = 0, q = 0; // Separate indices for satellite and terrestrial flows
for (map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i) {
    if (i->second.rxPackets > 0) {
        double rxDuration = i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();
        double flowThroughput = i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000; // Mbps

        // Check if the flow is associated with a satellite or terrestrial gNB
        if (gNBToAttachWithIndex[k] >= 0) { // Satellite gNB
            satelliteLinkThroughput[gNBToAttachWithIndex[k]] += flowThroughput;
        } else if (terrestrialgNBToAttachWithIndex[q] >= 0) { // Terrestrial gNB
            terrestrialLinkThroughput[terrestrialgNBToAttachWithIndex[q]] += flowThroughput;
        }
    }
    k++;
    q++;
}

// Calculate average throughput
//double averageSatelliteLinkThroughput = 0;
for (uint32_t l = 0; l < numgNB; l++) {
    averageSatelliteLinkThroughput += satelliteLinkThroughput[l];
}
averageSatelliteLinkThroughput /= numgNB;

//double averageTerrestrialLinkThroughput = 0;
for (uint32_t l = 0; l < numTerrestrialGNBs; l++) {
    averageTerrestrialLinkThroughput += terrestrialLinkThroughput[l];
}
averageTerrestrialLinkThroughput /= numTerrestrialGNBs;

// Write results to the performance results file
performanceResultsFile << "Aggregated throughput per satellite channel [Mbps]\n";
for (uint32_t l = 0; l < numgNB; l++) {
    performanceResultsFile << satelliteLinkThroughput[l] << " ";
}
performanceResultsFile << "\n";
performanceResultsFile << "Average satellite aggregated throughput [Mbps]\n";
performanceResultsFile << averageSatelliteLinkThroughput << "\n";

performanceResultsFile << "Aggregated throughput per terrestrial channel [Mbps]\n";
for (uint32_t l = 0; l < numTerrestrialGNBs; l++) {
    performanceResultsFile << terrestrialLinkThroughput[l] << " ";
}
performanceResultsFile << "\n";
performanceResultsFile << "Average terrestrial aggregated throughput [Mbps]\n";
performanceResultsFile << averageTerrestrialLinkThroughput << "\n";








  performanceResultsFile << "Number of satellite handovers\n";
  performanceResultsFile << numberOfHandovers << "\n";
  performanceResultsFile << "Number of terrestrial handovers\n";
  performanceResultsFile << numberOfTerHandovers << "\n";
  performanceResultsFile << "Total number of handovers\n";
  performanceResultsFile << totalHandovers << "\n";

  performanceResultsFile.close ();
  ifstream perResIn(performanceResultsFileName.c_str(), ios::binary);
  string performanceResultsFileNamePerRound = outputPath + "performanceResults_" + to_string(simRoundIndex) + ".txt";
  ofstream perResOut(performanceResultsFileNamePerRound.c_str(), ios::binary);
  perResOut << perResIn.rdbuf();

  // Creation of the satellite (gNB) position output text file
  stringstream gNBPositionFileName;
  gNBPositionFileName << outputPath << "satellitePositions.txt";
  ofstream gNBPositionFile(gNBPositionFileName.str().c_str(), ofstream::out);
//  gNBPositionFile << "IndexgNB X Y Z (ECI coordinates)\n";
  for (uint32_t i = 0; i < gNbContainer.GetN(); i++)
	{
	  Vector gNBPosition = gNbContainer.Get(i)->GetObject<MobilityModel> ()->GetPosition ();
	  gNBPositionFile << i << " " << gNBPosition.x << " " << gNBPosition.y << " " << gNBPosition.z << "\n";
	}
  gNBPositionFile.close();

  // Creation of the UE position output text file
  stringstream uePositionFileName;
  uePositionFileName << outputPath << "uePositions.txt";
  ofstream uePositionFile(uePositionFileName.str().c_str(), ofstream::out);
//  uePositionFile << "IndexUE X Y Z (ECI coordinates)\n";
  for (uint32_t j = 0; j < ueGlobalContainer.GetN(); j++)
	{
	  Vector UEPosition = ueGlobalContainer.Get(j)->GetObject<MobilityModel> ()->GetPosition ();
	  uePositionFile << j << " " << UEPosition.x << " " << UEPosition.y << " " << UEPosition.z << "\n";
	}
  uePositionFile.close();

  // Rename current resource allocation file in previous resource allocation file (needed to count the number of handovers between consecutive rounds)
  //rename(resAllFileName.str().c_str(), prevResAllFileName.str().c_str());

  Simulator::Destroy ();

  return 0;
}

/*
 * Periodically print the simulation status
 */
void PrintSimulationStatus(uint32_t simulationDuration) {
	stringstream logFileName;
	logFileName << outputPath << "logFile.txt";
	ofstream logFile(logFileName.str().c_str(), ofstream::app);
	cout << "Simulation progress: " << Simulator::Now().GetSeconds() << " s - " << ((double)Simulator::Now().GetSeconds() / simulationDuration * 100) << "%\n";
	logFile << "Simulation progress: " << Simulator::Now().GetSeconds() << " s - " << ((double)Simulator::Now().GetSeconds() / simulationDuration * 100) << "%\n";
	logFile.close();
	Simulator::Schedule(MilliSeconds(100.0), &PrintSimulationStatus, simulationDuration);
}





