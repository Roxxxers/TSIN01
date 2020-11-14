#ifndef ALOHASIM_H
#define ALOHASIM_H

#include <vector>
#include <random>
#include <string>

using namespace std;

// ---------------------------- Nodes ---------------------------- //
class Node
{
public:
  Node(double qa);

  int SuccSend(int DepSlot);

  bool HasPacket();
  bool NewArrival();
  bool NodeBacklog();
  bool backlogged;

  bool NormalTransmition(double ProbOfResend, int Slot);
  bool BayesianTransmition(double ProbOfResend, int Slot);
  static void RandomINIT();


private:

protected:

  static uniform_real_distribution<double> _RandomArrival;
  static default_random_engine _engine;
  double _qa;
  bool _HasPacket;
  int _PacketSlotArrival;
  bool _NewArrival;

};

class NormalNode : public Node
{
public:
  NormalNode(double qa) : Node{qa} {}
  virtual bool NormalTransmition(double qr, int slotNumber);
  virtual bool BayesianTransmition(double ProbOfResend, int Slot);

private:
};

// --------------------------------------------------------------- //

// -------------------------- Simulator -------------------------- //
struct SimData
{
  int outcome;
  int backlog;
  int arrival;
  int departure;
  double AttemptRate;
  double Ps;
  double AveragePS;
  double ProbOfResend;
  double estimatedBacklog;
  double DelayAverage;
  double _w;
  double ArrivalRate;
};

class AlohaSim
{

public:
  AlohaSim(int Nodes, double ArrivalRate, double qr);
  ~AlohaSim();
  double BayesianUpdate(int outcome);
  SimData Data();

  void CalculateNormalMode(int backlog, double& AttemptRate, double& Ps);
  void CalculateBayesianMode(int backlog, double& AttemptRate, double& Ps);

private:
  int _BackloggSize;
  int _Data;
  int _Delay;
  int _Departure;


  double _ArrivalRate;
  double _ProbOfArrival;
  double _ProbOfResend;
  double _Ar;



  //pseudo-Bayesian implementation
  double Bayesian_BL_Est;
  double DelayAverage;
  double _w;

  vector<Node *> _Nodes;
  vector<Node *> _sendingNodes;
  vector<double> _SuccessPR;
};

// --------------------------------------------------------------- //

#endif
