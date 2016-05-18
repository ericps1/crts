#ifndef _SC_TEMPLATE_
#define _SC_TEMPLATE_

#include "SC.hpp"

class SC_Template : public Scenario_Controller {

private:
  unsigned int debugLevel;
public:
  SC_Template(int argc, char **argv);
  ~SC_Template();
  virtual void execute();
  virtual void initialize_node_fb();
};

#endif
