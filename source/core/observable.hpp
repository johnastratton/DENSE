#ifndef CORE_OBSERVABLE_HPP_INCLUDED
#define CORE_OBSERVABLE_HPP_INCLUDED

#include <vector>

#include "context.hpp"


class Observer;

/**
Superclass for Simulation
*/
class Observable {

  public:

    Observable() : t(0.0), abort_signaled(false) {}

    void addObserver(Observer *o) { observerList.push_back(o); }

    // Called by Observer in update
    void abort() { abort_signaled = true; }

    virtual void run() = 0;

    //"abort_signaled" condition checked
    void notify(ContextBase& start);

    void finalize();

  protected:

    std::vector<Observer*> observerList;

    double t;

    bool abort_signaled;

};

/**
Superclass for CSV Writer and Analysis
*/
class Observer {

  public:

    Observer(Observable * oAble, int mn, int mx, RATETYPE startT, RATETYPE endT);

    virtual ~Observer() = default;

    int getMin() const;

    bool isInTimeBounds(double t) const {
        return t >= start_time && t < end_time;
    };

    virtual void finalize() = 0;

    virtual void update(ContextBase& start) = 0;

  protected:

    Observable * subject;

    RATETYPE start_time, end_time;

    int min, max;

};

#endif
