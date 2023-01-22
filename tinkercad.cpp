struct Info
{
  int id;
  int route;
  float passTime;
};

class Vehicle
{
  public:
    int id;
    float speed;
    float length;
    int route;
    Vehicle(int id, float speed, float length);
  	float sendRequest();
    void setRoute(int route); 
};

class IntersectionManager{
  public:
  	IntersectionManager();
    String acceptVehicle(Vehicle &vehicle);
    void removePassedVehicles(int vehiclesCount);
};

float calcSpeed(float distance, float time);
float calcTime(float distance, float speed);
float calcIntersectionInnerDistance(int route);
bool checkConflict(int targetRoute);
bool checkVehicleHasConflict(int targetRoute, int prevVehicleRoute, float prevVehicleRoutePassTime);

const int cols = 12, rows = 12;
const float distanceFromIntersection = 300;
const float intersectionRadius = 30;
const int vehicleCount = 20;
Info infoList[vehicleCount];

IntersectionManager::IntersectionManager(){}

String IntersectionManager::acceptVehicle(Vehicle &vehicle)
    {
  
      bool conflict = false;
      // check conflict matrix
      conflict = checkConflict(vehicle.route);

      float passTime;
      float arriveTime = 0;
      float speed = vehicle.speed;

      if (conflict)
      {
        // get pass time and arrive time of latest vehicle in conflicted route
        for (int j = 0; j < vehicleCount; j++)
        {
          if (checkVehicleHasConflict(vehicle.route, infoList[j].route, infoList[j].passTime))
          {
            arriveTime = infoList[j].passTime; // pass time of previous vehicle
          }
        }
        // calculate arrive time and speed
        if (arriveTime > 0.1)
        {
          speed = calcSpeed(distanceFromIntersection, arriveTime); // regulate speed according to calculated arrive time
        }
        else
        {
          arriveTime = calcTime(distanceFromIntersection - vehicle.length, speed);
          // speed can remain same
        }
      }else{
      	arriveTime = calcTime(distanceFromIntersection - vehicle.length, speed);
      }
  		
      passTime = arriveTime + calcTime(calcIntersectionInnerDistance(vehicle.route) + vehicle.length, speed);
      return String(passTime) + " " + String(speed);
    }

void IntersectionManager::removePassedVehicles(int vehiclesCount){
      unsigned long secondsAtStart = millis() / 1000;
      int sizeOfInfoList = vehiclesCount;
      unsigned long fetchStartTime;
      unsigned long fetchEndTime;

      while (sizeOfInfoList)
      {
        int vehicleToDeleteIndex = -1;
        unsigned long deletionSeconds;
        for (int i = 0; i < sizeOfInfoList; i++)
        {
          unsigned long currentSeconds = millis() / 1000;
          deletionSeconds = currentSeconds - secondsAtStart;
          if (infoList[i].passTime <= deletionSeconds)
          {
            vehicleToDeleteIndex = i;
          }
        }

        if (vehicleToDeleteIndex >= 0)
        {
          int deletedVehicleId = infoList[vehicleToDeleteIndex].id;
          for (int j = vehicleToDeleteIndex; j < sizeOfInfoList; j++)
          {
            infoList[j] = infoList[j + 1];
          }
          Serial.print("Vehicle: ");
          Serial.print(deletedVehicleId);
          Serial.print(" removed from vehicle list after ");
          Serial.print(deletionSeconds);
          Serial.println(" seconds");
          // reset last element and decrease size of array
          infoList[sizeOfInfoList - 1].id = 0;
          infoList[sizeOfInfoList - 1].route = 0;
          infoList[sizeOfInfoList - 1].passTime = 0;
          sizeOfInfoList--;
        }
      }
    }

// Intersection Manager's IDLE State 
IntersectionManager intersectionManager;


Vehicle::Vehicle(int id, float speed, float length)
{
  this->id = id;
  this->speed = speed;
  this->length = length;
}

void Vehicle::setRoute(int route){
  this->route = route;
} 

float Vehicle::sendRequest(){

  String response = intersectionManager.acceptVehicle(*this);
	int index = response.indexOf(' ');
  	float passTime = response.substring(0, index).toFloat();
    // Vehicle's Acceleration State set speed 
     this->speed = response.substring(index+1).toFloat();

  return passTime;
}


Vehicle *vehicles[vehicleCount];

int conflictMatrix[rows][cols] = {
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1},
    {0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1},
    {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1},
    {0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0},
    {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0},
    {0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0},
    {0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1}};

float calcSpeed(float distance, float time)
{
  return ceil(distance / time);
}

float calcTime(float distance, float speed)
{
  return ceil(distance / speed);
}

float calcIntersectionInnerDistance(int route)
{
  if (route == 8 || route == 2 || route == 11 || route == 5)
  {
    return intersectionRadius * 3.14 / 2;
  }
  else if (route == 1 || route == 7 || route == 4 || route == 10)
  {
    return intersectionRadius * 2;
  }
  return 0;
}

bool checkConflict(int targetRoute)
{
  for (int j = 0; j < cols; j++)
  {
    if (conflictMatrix[targetRoute][j] == 1 && targetRoute != j)
    {
      return true;
    }
  }
  return false;
}

bool checkVehicleHasConflict(int targetRoute, int prevVehicleRoute, float prevVehicleRoutePassTime)
{
  return conflictMatrix[targetRoute][prevVehicleRoute] == 1 && prevVehicleRoutePassTime > 0; 
}


void printVehicles()
{
  for (int i = 0; i < vehicleCount; i++)
  {
    Serial.print("Vehicle: ");
    Serial.print(vehicles[i]->id);
    Serial.print(" Route: ");
    Serial.print(vehicles[i]->route);
    Serial.print(" Speed: ");
    Serial.print(vehicles[i]->speed);
    Serial.print(" Length: ");
    Serial.print(vehicles[i]->length);
    Serial.print("\n");
  }
  Serial.print("\n");
}

void printInfoList()
{
  for (int i = 0; i < vehicleCount; i++)
  {
    Serial.print("Vehicle: ");
    Serial.print(infoList[i].id);
    Serial.print(" Route: ");
    Serial.print(infoList[i].route);
    Serial.print(" Pass Time: ");
    Serial.print(infoList[i].passTime);
    Serial.print("\n");
  }
  Serial.print("\n");
}

bool testCheckConflict()
{
  int targetRoute1 = 1;
  int targetRoute2 = 0;
  return checkConflict(targetRoute1) == 1 && checkConflict(targetRoute2) == 0;
}

bool testCheckVehicleHasConflict()
{
  int targetRoute1 = 1; 
  int prevVehicleRoute1 = 2; 
  float prevVehicleRoutePassTime1 = 3; 
  int targetRoute2 = 1; 
  int prevVehicleRoute2 = 8; 
  float prevVehicleRoutePassTime2 = 3; 
  return checkVehicleHasConflict(targetRoute1, prevVehicleRoute1, prevVehicleRoutePassTime1) == false && checkVehicleHasConflict(targetRoute2, prevVehicleRoute2, prevVehicleRoutePassTime2) == true;
}

bool testCalcSpeed()
{
  float distance = 20; 
  float time = 5; 
  return calcSpeed(distance, time) == 4; 
}

bool testCalcTime()
{
  float distance = 20;
  float speed = 5;
  return calcTime(distance, speed) == 4;
}

bool testCalcIntersectionInnerDistance()
{
  int route1 = 0, route2 = 1, route3 = 2;
  float result1 = 0, result2 = 60.00, result3 = 47.10;
  return calcIntersectionInnerDistance(route1) == result1 && calcIntersectionInnerDistance(route2) == result2 && abs(calcIntersectionInnerDistance(route3) - result3) < 0.01;
}


bool testVehicleSendRequest()
{
  Vehicle vehicle = Vehicle(1, 60, 4);
  
  return vehicle.sendRequest() == float(vehicle.sendRequest());
}



void TEST(String title, auto func){
	Serial.println("-------\nRUN TEST");
  
    if(func()){
      Serial.print("PASSED TEST ");
    }else{
      Serial.print("FAILED TEST ");
    }
    Serial.println(title);
    return;	
}



void setup()
{
  Serial.begin(9600);
  
  TEST("Should check conflict matrix", testCheckConflict);
  TEST("Should calculate speed", testCalcSpeed);
  TEST("Should calculate time", testCalcTime);
  TEST("Should check vehicle has conflict", testCheckVehicleHasConflict);
  TEST("Should calculate intersection inner distance", testCalcIntersectionInnerDistance);
  TEST("Should return speed from vehicle's request to Intersection Manager", testVehicleSendRequest);
}

void loop()
{  
  for (int i = 0; i < vehicleCount; i++)
  {
    vehicles[i] = new Vehicle(i+1, 60, 3 + (i % 2));
  }
  vehicles[0]->setRoute(0);
  vehicles[1]->setRoute(1);
  vehicles[2]->setRoute(2);
  vehicles[3]->setRoute(3);
  vehicles[4]->setRoute(4);
  vehicles[5]->setRoute(5);
  vehicles[6]->setRoute(6);
  vehicles[7]->setRoute(7);
  vehicles[8]->setRoute(8);
  vehicles[9]->setRoute(9);
  vehicles[10]->setRoute(10);
  vehicles[11]->setRoute(11);
  vehicles[12]->setRoute(0);
  vehicles[13]->setRoute(1);
  vehicles[14]->setRoute(2);
  vehicles[15]->setRoute(3);
  vehicles[16]->setRoute(4);
  vehicles[17]->setRoute(5);
  vehicles[18]->setRoute(6);
  vehicles[19]->setRoute(7);
  
  Serial.print("\nVehicles' Driving State (Driving)\n");
  printVehicles();
  
  // Intersection Manager's Scheduling State 
  for (int i = 0; i < vehicleCount; i++)
  {
    float passTime = vehicles[i]->sendRequest();
    infoList[i].id = vehicles[i]->id;
    infoList[i].route = vehicles[i]->route;
    infoList[i].passTime = passTime;
  }

  // Vehicle's Approaching and Passing State
  Serial.print("\nVehicles' Approaching and Passing State\n");
  printInfoList();
  
  // Intersection Manager's Remove State
  Serial.print("Intersection Manager's Remove State\n");
  intersectionManager.removePassedVehicles(vehicleCount);
  

  delay(10000); // Wait for 1000 millisecond(s)
}