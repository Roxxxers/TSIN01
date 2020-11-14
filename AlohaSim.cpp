#include "AlohaSim.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>


// ---------------------------- Random ---------------------------- //

/*
This randomizer is ued for the transmission and retransmission delay for the
packets
*/
uniform_real_distribution<double> Node::_RandomArrival{0, 1};
default_random_engine Node::_engine{};

Node::Node(double qa) : _qa{qa}, _HasPacket{false}, _PacketSlotArrival{0}, _NewArrival{false}
{
}

void Node::RandomINIT()
{
  random_device rnd;
  _engine.seed(rnd());
}

// ---------------------------------------------------------------- //



// -------------------------- Simulator -------------------------- //

AlohaSim::AlohaSim(int Nodes, double ArrivalRate, double qr)
    : _ArrivalRate{ArrivalRate}, _ProbOfResend{qr}, _Data{0}, _Departure{0}, _Delay{0}
{
  Node::RandomINIT();
  _ProbOfArrival = (1 - exp(-_ArrivalRate / Nodes));

 //Fills upp the _Nodes vector
  for (int i = 0; i < Nodes; ++i)
  {
    _Nodes.push_back(new Node(_ProbOfArrival));
  }
}

//Destructor
AlohaSim::~AlohaSim()
{
  while (_Nodes.size() > 0)
  {
    delete _Nodes.back();
    _Nodes.pop_back();
  }
}

void AlohaSim::CalculateNormalMode(int backlog, double& AttemptRate, double& Ps)
{
  //G(n) = (m−n)qa+nqr  Attempt rate
  AttemptRate = (_Nodes.size()-backlog)*_ProbOfArrival + (backlog*_ProbOfResend);

  // Ps≈G(n)e^−G(n) probability of seccessful transmission
  Ps = AttemptRate*exp(-AttemptRate);
}

SimData AlohaSim::Data()
{

  int outcome = 0;
  int backlog = 0;
  int Arrivals = 0;

  double AttemptRate = 0;
  double Ps = 0;
  double AveragePS = 0;

  _sendingNodes.clear();
  _ProbOfResend = BayesianUpdate(outcome);

  //scout << _ProbOfResend << endl;


  for (auto node : _Nodes)
  {
    if (node->HasPacket())
    {
      node->backlogged = true;
      ++backlog;
    }

    if (node->BayesianTransmition(_ProbOfResend, _Data))
    {
      _sendingNodes.push_back(node);
    }

    if (node->NewArrival())
      ++Arrivals;
  }


  if (_sendingNodes.size() == 1)
  {
    outcome = 1;
    int value = _sendingNodes.front()->SuccSend(_Data);
    _Delay += value;
    ++_Departure;
    if (_sendingNodes.front()->backlogged)
    {
      _sendingNodes.front()->backlogged = false;
      --backlog;
    }

  }
  else if (_sendingNodes.size() > 1)
  {
    outcome = -1;
  }
  double averageDelay = -1;
  if (_Departure != 0)
    averageDelay = _Delay / static_cast<double>(_Departure);

  ++_Data;




// IF bayesian implementation is NOT used, use this code, else comment out
// CalculateNormalMode(backlog, AttemptRate, Ps)


//IF bayesian implementation is used, use this code, else comment out
  _w = (exp(1)-(1.0/2.0))/(1-_ArrivalRate*exp(1)) - ((exp(1)-1)*(exp(_ArrivalRate)-1))/_ArrivalRate*(1-(exp(1)-1)*(exp(_ArrivalRate)-1));
  _ProbOfResend = BayesianUpdate(outcome);
  CalculateBayesianMode(backlog, AttemptRate, Ps);



  _SuccessPR.push_back(Ps);
  cout << averageDelay << endl;

//Calculates the average delay
  for_each(_SuccessPR.begin(), _SuccessPR.end(), [&] (double n)
  {
    AveragePS += n;
  });
  AveragePS /= _SuccessPR.size();
  //cout << AveragePS << endl;


  //Normal mode return
  //return SimData{outcome, backlog, 0, Arrivals, (outcome == 1 ? 1 : 0), averageDelay, AttemptRate, Ps, AveragePS};

  //Bayesian implementation returns
  return SimData{outcome, backlog, Arrivals, (outcome == 1 ? 1 : 0), AttemptRate, Ps, AveragePS, _ProbOfResend, Bayesian_BL_Est, averageDelay, _w, _ArrivalRate};

}
// ---------------------------------------------------------------- //


// -----------------Pseudo-Bayesian implementation----------------- //

//
double AlohaSim::BayesianUpdate(int outcome){
  if(outcome == -1)
    Bayesian_BL_Est = Bayesian_BL_Est + _ArrivalRate + 1/(exp(1)-2);
  else
    Bayesian_BL_Est = max(_ArrivalRate, Bayesian_BL_Est + _ArrivalRate - 1);


    //cout << Bayesian_BL_Est << endl;
  return min(1.0, 1/ Bayesian_BL_Est);
}

void AlohaSim::CalculateBayesianMode(int backlog, double& AttemptRate, double& Ps)
{
  if(backlog >= 1)
     Ps = backlog*_ProbOfResend*pow(1-_ProbOfResend, backlog-1);
     //Ps = n*qr*(1 - qr)^(n-1)
    //cout << backlog << "      " << _ProbOfResend << endl;

     AttemptRate = backlog*_ProbOfArrival; //G(n) = n*qr
     _Ar = (_Nodes.size()-backlog)*_ProbOfArrival; //(m-n)*qa
}

bool Node::BayesianTransmition(double ProbOfResend, int Slot)
{
  double randValue = _RandomArrival(_engine);
	_NewArrival = false;

	/*If a random value is less than _qa, and the node has no packet, node will have a new arrival*/
	if (randValue < _qa && !_HasPacket)
		_NewArrival = true;

	/*If node has a new arrival, it will be backlogged according to the Pseudo-Bayesian algorithm*/
	if (_NewArrival)
		{
			_PacketSlotArrival = Slot;
			_HasPacket = true;
			return false;
		}

	if(_HasPacket)
		return randValue < ProbOfResend;
	return false;


}

// ---------------------------------------------------------------- //


// ---------------------------- Nodes ---------------------------- //

//--------Getters-------//

bool Node::HasPacket()
{
  return _HasPacket;
}

bool Node::NewArrival()
{
  return _NewArrival;
}

//----------------------//

int Node::SuccSend(int DepSlot)
{
  int delay = DepSlot - _PacketSlotArrival;
  _HasPacket = false;
  _PacketSlotArrival = 0;

  return delay;
}

bool Node::NormalTransmition(double ProbOfResend, int Slot)
{
  double randValue = _RandomArrival(_engine);
  _NewArrival = false;

  if (_HasPacket)
    return randValue < ProbOfResend;
  else
  {
    if (randValue < _qa)
    {
      _PacketSlotArrival = Slot;
      _NewArrival = true;
      _HasPacket = true;
      return true;
    }
    return false;
  }
}
