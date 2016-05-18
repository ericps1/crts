#ifndef _SC_CONTROL_AND_FEEDBACK_TEST_
#define _SC_CONTROL_AND_FEEDBACK_TEST_

#include "SC.hpp"

class SC_Control_and_Feedback_Test : public Scenario_Controller {

private:
  // internal members used by this CE

public:
  SC_Control_and_Feedback_Test(int argc, char **argv);
  ~SC_Control_and_Feedback_Test();
  virtual void execute(int node, char fb_type, void *_arg);
  virtual void initialize_node_fb();
};

#endif
