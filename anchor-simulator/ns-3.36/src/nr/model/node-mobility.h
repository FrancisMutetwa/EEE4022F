#ifndef NODEMOBILITY_H
#define NODEMOBILITY_H

#include <math.h>
#include <string>
#include "ns3/netanim-module.h"

using namespace std;
namespace ns3 {

	// Define variable related to satellite orbits
	#define EARTH_RADIUS 6371																// Earth radius [km] (km instead of m to avoid integer overflow)
	#define MINIMUM_ELEVATION_ANGLE 0.35													// minimum elevation angle for contact [radians] (10°=0.175, 20°=0.35)
	#define LIGHT_SPEED 300																	// light speed [km/ms]
	const double MASS_EARTH = 5.9736e24;			                                        // mass of the Earth [kg]
	const double GRAVITATIONAL_CONST = 6.673e-11;			                                // gravity constant [N*m^2/kg^2]
	// paths where to find the input files and where to store the created output files
	const string inputPath = "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/Input/trial1/";	
	const string outputPath = "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/Output/trial1Output/";

	struct NodePositions
	{
		int nodeIndex;
		int* time;
		double** coordinates;
	};

	void CalculategNBPosition(uint32_t numgNB, uint32_t simulationDurationS, NodePositions gNBPositions[], double satelliteAltitudes[]);
	void CalculateUEPosition(uint32_t ueTotalNum, uint32_t simulationDurationS, NodePositions uePositions[]);
	void SetgNBPosition(NodeContainer gNbContainer, uint32_t currentTime, NodePositions gNBPositions[]);
	void SetUEPosition(NodeContainer ueGlobalContainer, uint32_t currentTime, NodePositions uePositions[]);

}
#endif /* NODEMOBILITY_H */
