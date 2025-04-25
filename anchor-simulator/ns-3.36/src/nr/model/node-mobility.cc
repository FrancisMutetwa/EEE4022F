#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "node-mobility.h"
#include "sgp4ext.h"
#include "sgp4unit.h"
#include "sgp4io.h"
#include "ns3/netanim-module.h"

using namespace std;
namespace ns3 {


/*
 * Calculate gNB (satellite) positions
 * The TLE of each satellite, written in a .txt file, is passed as input and used to calculate its ECI coordinates using the SGP4 orbit model one time per second for all simulation duration
 * ECI coordinates are written in .txt files
 */

void CalculategNBPosition(uint32_t numgNB, uint32_t simulationDurationS, NodePositions gNBPositions[], double satelliteAltitudes[]) {
	// open the .txt file containing the satellite TLE data
	stringstream tleFile;
	tleFile << inputPath << "satelliteTLEs.txt";
	string tle = tleFile.str();
	const char* tleFileName = tle.c_str();
	// Write possible unexpected events in the log file
	stringstream logFileName;
	logFileName << outputPath << "logFile.txt";
	ofstream logFile(logFileName.str().c_str(), ofstream::out);
	FILE* infile = fopen(tleFileName, "r");
	if (!infile)
	  {
		cerr << "gNB coordinate file: Unable to open file!";
		logFile << "gNB coordinate file: Unable to open file!";
	    exit(1);   // call system to stop
	  }
	for (uint32_t i = 0; i < numgNB; i++)
	  {
		gNBPositions[i].nodeIndex = i;
		// define needed variables
		char longstr0[130], longstr1[130], longstr2[130];		// first line is the satellite name, then the two lines of the TLE data
		char typerun = 'v';						// v allows to calculate the position in a certain interval time contained in the TLE
		char typeinput;							// specify the input type of the time if it is manually inserted (not useful in our case)
		char opsmode = 'a';						// mode of operation afspc or improved 'a', 'i'
		gravconsttype  whichconst = wgs84;		// which set of constants to use depending on the used ellipsoidal model
		double startmfe = 0, stopmfe = simulationDurationS, delta = 1;		// start, stop, and delta of the interval time to calculate the satellite position (in seconds)
		double tsince = 0;						// current computation time
		double vo[3];							// velocity vector of the satellite
		elsetrec satrec;						// structure containing all the sgp4 satellite information
		// convert the char string to sgp4 elements
		// includes initialization of sgp4
		fgets(longstr0,130,infile);				// Satellite name
		fgets(longstr1,130,infile);				// TLE first line
		fgets(longstr2,130,infile);				// TLE second line
		twoline2rv (longstr1, longstr2, typerun, typeinput, opsmode, whichconst, startmfe, stopmfe, delta, satrec);
		char revolutionsPerDay[16];
		for (uint32_t p = 0; p < 16; p++)
			revolutionsPerDay[p] = longstr2[p+52];
		double revolutionsPerDayNumber = atof(revolutionsPerDay);
		satelliteAltitudes[i] = cbrt(1.86624 * GRAVITATIONAL_CONST * MASS_EARTH / pow(M_PI * revolutionsPerDayNumber,2)) - EARTH_RADIUS;
		tsince = startmfe;
		gNBPositions[i].time = new int[simulationDurationS];
		gNBPositions[i].coordinates = new double*[simulationDurationS];
		// ----------------- loop to perform the propagation ----------------
		for (uint32_t j = 0; j <= simulationDurationS; j++)
		  {
			gNBPositions[i].coordinates[j] = new double[3];
			sgp4(whichconst,satrec,tsince,gNBPositions[i].coordinates[j],vo);
			tsince += delta;
		  }
	  }
	logFile.close();
}

/*
 * Calculate UE position
 * The Latitude, Longitude, Altitude (LLA) coordinates of each ground terminal, wrote in a .txt file, is passed as input
 * LLA coordinates are converted in ECEF coordinates, then in ECI coordinates, and finally written in .txt files
 */
void CalculateUEPosition(uint32_t ueTotalNum, uint32_t simulationDurationS, NodePositions uePositions[]) {
	// open the .txt file containing the UE coordinates data
	stringstream coordinateFileName;
	coordinateFileName << inputPath << "UECoordinates.txt";
	// Write possible unexpected events in the log file
	stringstream logFileName;
	logFileName << outputPath << "logFile.txt";
	ofstream logFile(logFileName.str().c_str(), ofstream::out);
	ifstream coordinateFile(coordinateFileName.str().c_str());
	  if (!coordinateFile)
	    {
		  cerr << "UE coordinate file: Unable to open file!";
		  logFile << "UE coordinate file: Unable to open file!";
	      exit(1);   // call system to stop
	    }
	for (uint32_t i = 0; i < ueTotalNum; i++)
	  {
		uePositions[i].nodeIndex = i;
		double latitude, longitude, altitude;
		// using WGS84 orbit model, the equatorial radius and the polar radius are 6378.137 km and 6356.755 km respectively
		double equatorialRadius = 6378.137, polarRadius = 6356.755;
		coordinateFile >> latitude >> longitude >> altitude;		// latitude and longitude in degrees, altitude in m
		// convert latitude and longitude in radians (ATTENTION! Depending on the input coordinate format)
		latitude = latitude * M_PI / 180;
		longitude = longitude * M_PI / 180;
		altitude = altitude / 1000;		// convert altitude in km
		// calculate Earth radius at the ground station position and geocentric latitude
		double geocentricLatitude = atan((polarRadius / equatorialRadius) * tan(latitude));
		double earthRadius = altitude + ((equatorialRadius * polarRadius) / sqrt(pow(equatorialRadius * sin(geocentricLatitude),2) + pow(polarRadius * cos(geocentricLatitude),2)));
		// calculate ECI (Greenwich) ground station coordinates
		uePositions[i].time = new int[simulationDurationS];
		uePositions[i].coordinates = new double*[simulationDurationS];
		for (uint32_t j = 0; j <= simulationDurationS; j++)
		  {
			uePositions[i].coordinates[j] = new double[3];
			uePositions[i].coordinates[j][0] = earthRadius * cos(longitude) * cos(geocentricLatitude);
			uePositions[i].coordinates[j][1] = earthRadius * sin(longitude) * cos(geocentricLatitude);
			uePositions[i].coordinates[j][2] = earthRadius * sin(geocentricLatitude);
			longitude += (double)(2 * M_PI / 86400);
		  }
	  }
	coordinateFile.close();
	logFile.close();
}

/*
 * Set the position of all gNBs at the time currentTime
 */

void SetgNBPosition(NodeContainer gNbContainer, uint32_t currentTime, NodePositions gNBPositions[]) {
  for (uint32_t i = 0; i < gNbContainer.GetN(); i++)
	{
	  Ptr<MobilityModel> gNbMobility = gNbContainer.Get(i)->GetObject<MobilityModel> ();
	  gNbMobility->SetPosition(Vector(gNBPositions[i].coordinates[currentTime][0], gNBPositions[i].coordinates[currentTime][1], gNBPositions[i].coordinates[currentTime][2]));
	}
  Simulator::Schedule(Seconds(1.0), SetgNBPosition, gNbContainer, ++currentTime, gNBPositions);
}


/** 
void SetgNBPosition(NodeContainer gNbContainer, uint32_t currentTime, NodePositions* gNBPositions) {
    for (uint32_t i = 0; i < gNbContainer.GetN(); i++) {
        Ptr<MobilityModel> gNbMobility = gNbContainer.Get(i)->GetObject<MobilityModel>();
        gNbMobility->SetPosition(Vector(gNBPositions[i].coordinates[currentTime][0],
                                        gNBPositions[i].coordinates[currentTime][1],
                                        gNBPositions[i].coordinates[currentTime][2]));
    }
    Simulator::Schedule(Seconds(1.0), &SetgNBPosition, gNbContainer, ++currentTime, gNBPositions);
}

*/



/*
 * Set the position of all UEs at the time currentTime
 */


 void SetUEPosition(NodeContainer ueGlobalContainer, uint32_t currentTime, NodePositions uePositions[]) {
	for (uint32_t i = 0; i < ueGlobalContainer.GetN(); i++)
	  {
		Ptr<MobilityModel> ueMobility = ueGlobalContainer.Get(i)->GetObject<MobilityModel> ();
		ueMobility->SetPosition(Vector(uePositions[i].coordinates[currentTime][0], uePositions[i].coordinates[currentTime][1], uePositions[i].coordinates[currentTime][2]));
	  }
	Simulator::Schedule(Seconds(1.0), SetUEPosition, ueGlobalContainer, ++currentTime, uePositions);
}


/** 

void SetUEPosition(NodeContainer ueGlobalContainer, uint32_t currentTime, NodePositions* uePositions) {
	for (uint32_t i = 0; i < ueGlobalContainer.GetN(); i++)
	  {
		Ptr<MobilityModel> ueMobility = ueGlobalContainer.Get(i)->GetObject<MobilityModel> ();
		ueMobility->SetPosition(Vector(uePositions[i].coordinates[currentTime][0], uePositions[i].coordinates[currentTime][1], uePositions[i].coordinates[currentTime][2]));
	  }
	Simulator::Schedule(Seconds(1.0), SetUEPosition, ueGlobalContainer, ++currentTime, uePositions);
}

*/



/** 

void SetUEPosition(NodeContainer ueGlobalContainer, uint32_t currentTime, NodePositions uePositions[], AnimationInterface* anim) {
    for (uint32_t i = 0; i < ueGlobalContainer.GetN(); i++) {
        Ptr<MobilityModel> ueMobility = ueGlobalContainer.Get(i)->GetObject<MobilityModel>();
        Vector newPosition(uePositions[i].coordinates[currentTime][0], 
                           uePositions[i].coordinates[currentTime][1], 
                           uePositions[i].coordinates[currentTime][2]);
        ueMobility->SetPosition(newPosition);

        // Notify NetAnim of the new position
        anim->SetConstantPosition(ueGlobalContainer.Get(i), newPosition.x, newPosition.y);
    }
    Simulator::Schedule(Seconds(1.0), &SetUEPosition, ueGlobalContainer, ++currentTime, uePositions, anim);
}
*/
}
