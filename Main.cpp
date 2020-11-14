#include "AlohaSim.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <iomanip>

using namespace std;

int main()
{
  //----Given variables----//
  const int Nodes = 100;
  const int Slots = 1000;
  double ArrivalRate = (0.20);
  double qr = 0.1;


  //-----Simulator varibales-----//

  int TotalAmountOfArrivals = 0;
  int TotalAmountOfDepartures = 0;

  AlohaSim Sim{Nodes, ArrivalRate, qr};
  SimData SimData;
  ofstream Output;
  string outcomeString;

//  G(n) = (m−n)qa+nqr  Attempt rate
//  Ps≈G(n)e^−G(n)


  //-------Simulator-------/

  Output.open("output.txt", ios::trunc);


  /*
  Output << " Slot "
             << " Backlog "
      //       << " Arrivals "
      //       << " Departures "
      //       << " AttemptRate "
      //       << " ProbSucc "
             << " ProbOfResend "
             << " estimatedBacklog "
             << " DelayAverage "
             << " AproxDelay "
             << " ArrivalRate "
             << " Outcome"
             << endl;
  */

  for (int i = 0; i < Slots; ++i)
  {
    //This gets the information from the current slot
    SimData = Sim.Data();
    TotalAmountOfArrivals += SimData.arrival;
    TotalAmountOfDepartures += SimData.departure;

    Output.setf(ios::showpoint);
    Output << setw(5) << i << " ";
    Output << setw(8) << setprecision(4) << SimData.backlog << " ";
    Output << setw(8) << setprecision(4) << TotalAmountOfArrivals << " ";
    Output << setw(8) << setprecision(4) << TotalAmountOfDepartures << " ";
    //Output << setw(12) << setprecision(4) << SimData.AttemptRate << " ";
    //Output << setw(12) << setprecision(4) << SimData.AveragePS << " ";
    //Output << setw(12) << setprecision(4) << SimData.ProbOfResend << " ";
    //Output << setw(12) << setprecision(4) << SimData.estimatedBacklog << endl;
    Output << setw(12) << setprecision(4) << SimData.DelayAverage << " ";
    Output << setw(12) << setprecision(4) << SimData._w << endl;
    //Output << setw(12) << setprecision(4) << SimData.ArrivalRate << " ";
    //Output << setw(12) << SimData.outcome << endl;


  }
}
