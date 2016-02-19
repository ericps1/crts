#ifndef _SC_TEMPLATE_
#define _SC_TEMPLATE_

#include "SC.hpp"

class SC_Template : public Scenario_Controller {

private:
  // internal members used by this CE

public:
  SC_Template();
  ~SC_Template();
  virtual void execute(int node, char fb_type, void *_arg);
  virtual void initialize_node_fb();
};

#endif
