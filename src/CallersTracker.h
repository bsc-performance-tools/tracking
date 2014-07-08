#ifndef __CALLERS_TRACKER_H__
#define __CALLERS_TRACKER_H__

#include "CallersHistogram.h"
#include "Tracker.h"

class CallersTracker : public Tracker
{
  public:
    CallersTracker(CallersHistogram *histogram1, CallersHistogram *histogram2);
    ~CallersTracker();

    void RunTracker();

  private:
    CallersHistogram *H1, *H2;
};

#endif /* __CALLERS_TRACKER_H__ */
