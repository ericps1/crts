#ifndef _SC_CONTROL_AND_FEEDBACK_TEST_
#define _SC_CONTROL_AND_FEEDBACK_TEST_

#include "scenario_controller.hpp"

class SC_Control_and_Feedback_Test : public ScenarioController {

private:
  // internal members used by this CE

public:
  SC_Control_and_Feedback_Test(int argc, char **argv);
  ~SC_Control_and_Feedback_Test();
  virtual void execute();
  virtual void initialize_node_fb();
};

#endif
