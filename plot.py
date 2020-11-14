import matplotlib.pyplot as plt
import numpy as np


def main():
    #Slot, backlog, arrivals, departures, feedback
    Arrival = [0.05, 0.10, 0.15, 0.20 , 0.25 , 0.30, 0.35]
    avgdelay = [1.451, 2.433, 3.620, 10.74, 9.828, 17.73, 50.49]
    _w = [0.9605, 1.566, 2.407, 3.682, 5.923,11.22, 45.07]


    plt.xlabel('Arrival Rate')
    plt.ylabel('True Delay (Blue) / Theoretical Delay(Orange)')


    plt.plot(Arrival, avgdelay, Arrival, _w)

    plt.show()


if __name__ == "__main__":
    main()
