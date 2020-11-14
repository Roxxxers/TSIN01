#include "aloha_simulator.h"
#include <math.h>
#include <unistd.h>

#include <iostream>

using namespace std;
AlohaSimulator::AlohaSimulator(int nodes, double arrivalRate, double qr)
: _arrivalRate{arrivalRate}, _qr{qr}, _currentSlot{0},
  _departures{0}
{

	_qa = 1 - exp(-_arrivalRate / nodes); //Probability of Arrival Qa.
	Node::initRandom();

  /*Fills private vector "_nodes" with Node objects according to defined
  probability of Arrival _qa*/
	for (int i = 0; i < nodes; ++i)
	{

		_nodes.push_back(new Node{_qa});
	}
}

//Destructor for Simulator class
AlohaSimulator::~AlohaSimulator()
{
	while(_nodes.size() > 0)
	{
		delete _nodes.back();
		_nodes.pop_back();
	}
}

/*Simply calculates an updated average Ps each slot.*/
double AlohaSimulator::calculateAverage(vector<double> v) {
  double sum = accumulate(v.begin(), v.end(), 0.0);
  return sum/v.size();
}

/*Makes various calculations according to assignment. Formula used are provided to the right*/
void AlohaSimulator::calculate(int backlogCount, bool bayesianMode){
  if(bayesianMode) {
    if (backlogCount >= 1)
      _successChance = backlogCount*_qr*pow(1-_qr, backlogCount-1); //Ps = n*qr*(1 - qr)^(n-1)
      _attemptRate = backlogCount*_qr; //G(n) = n*qr
      _ar = (_nodes.size() - backlogCount)*_qa; //(m-n)*qa
  } else {
      _attemptRate = (_nodes.size() - backlogCount)*_qa + (backlogCount*_qr); //G(n) = (m-n)qa + n
      _successChance = (_attemptRate*exp(-_attemptRate)); //Ps = G(n)e^(-G(n))
  }

      _ps.push_back(_successChance);
      _avgSuccessChance = calculateAverage(_ps);
      averageDelay();
}

void AlohaSimulator::averageDelay(){
  if(_totalDelay.size() > 0)
    _avgDelay = double(accumulate(_totalDelay.begin(), _totalDelay.end(), 0)) / _totalDelay.size();
}

/*Update qr based on estimated backlog accodring to pseudo-bayesian stabilization algorithm.*/
double AlohaSimulator::updateBayesian(int feedback){
  if (feedback == -1)
    _estBacklog = _estBacklog + _arrivalRate + 1/(exp(1)-2);
  else
    _estBacklog = max(_arrivalRate, _estBacklog + _arrivalRate - 1);

  _qr = min(1.0, 1 / _estBacklog);

  return _qr;
}

/*Calculates and returns the approxiamted delay W from the Pseudo-Bayesian approximate delay analysis*/
double AlohaSimulator::delayApproximation(){
  double e = exp(1);
  double l = _arrivalRate;
  return (e-(1.0/2.0))/(1-l*e) - ((e-1)*(exp(l)-1))/l*(1-(e-1)*(exp(l)-1));
}


SimulatorData AlohaSimulator::currentSlot()
{
	int feedback = 0; //IDLE
	int backlogCount = 0;
	int newArrival = 0;
	transmittingNodes.clear();
  bool nodeBacklogged;
  bool bayesianMode = true;
  int delay = 0;


  //Iterates through all nodes to simulate if any node/nodes is sending.
  if (bayesianMode)
    _qr = updateBayesian(feedback);


	for (auto node : _nodes)
	{
		if (node->packetExists()) { //If a node already has a packet, send node to backlog.
			++backlogCount;
      node->_backlogged = true;
    }
    //If node is backlogged, returns true if a random value is less than qr. If node is not backlogged returns true if a random value is less than qa. Otherwise node will be IDLE.
		if (node->transmitPacketBayesian(_qr, _currentSlot))
		{
			transmittingNodes.push_back(node);
		}

    /*Increment newArrival if node has a new arrival*/
		if (node->newArrival()) {
			++newArrival;
    }
	}

  /*Size of _transmittingNodes is the number of nodes transmitting from previous iteration loop for the current Slot. */
	if (transmittingNodes.size() == 1)
	{
		feedback = 1; //Success
    _totalDelay.push_back(transmittingNodes.front()->calculateDelay(_currentSlot));

		++_departures;
     if (transmittingNodes.front()->_backlogged) { //If true, backlogged node has retransmitted. Decrement backlog count.
        --backlogCount;
        transmittingNodes.front()->_backlogged = false;
      }

	}
  /*If more than 1 node wants to send in the same slot, a collision has occured. Next, these nodes will be backlogged when checking transmitting nodes in the next slot when (node->packetExists() == true).*/
	else if (transmittingNodes.size() > 1)
	{
		feedback = -1; //Collision
	}

  //Moves to next slot
  ++_currentSlot;

/*Different operations depending on if we are running Pseudo-bayesian mode or normal slotted aloha.*/
  if(bayesianMode) {
    _w = delayApproximation();
    _qr = updateBayesian(feedback);
    calculate(backlogCount, bayesianMode);
    return SimulatorData{feedback, backlogCount, newArrival, (feedback == 1 ? 1 : 0), _attemptRate, _successChance, _avgSuccessChance, _qr, _estBacklog, _avgDelay, _w, _arrivalRate};

  } else {
    calculate(backlogCount, bayesianMode);
  	return SimulatorData{feedback, backlogCount, newArrival, (feedback == 1 ? 1 : 0), _attemptRate, _successChance, _avgSuccessChance};
  }
}
