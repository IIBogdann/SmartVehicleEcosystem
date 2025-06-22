#ifndef TRAFFIC_SIGNS_H
#define TRAFFIC_SIGNS_H

#include "../display/DisplayManager.h"

class TrafficSigns {
  public:
    TrafficSigns();
    
    // Funcții pentru afișarea semnelor de trafic
    void showStop();
    void showYield();
    void showSpeedLimit(int limit);
    
    // Funcție de test generală
    void testSignDisplay();
    
  private:
    // Referință la managerul display-ului
    DisplayManager& display;
};

extern TrafficSigns trafficSigns;

#endif // TRAFFIC_SIGNS_H
