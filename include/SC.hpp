#ifndef _SC_HPP_
#define _SC_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "CRTS.hpp"

class Scenario_Controller {
public:
  Scenario_Controller();
  ~Scenario_Controller();
  virtual void execute(int node, char fb_type, void *_arg);
  virtual void initialize_node_fb();
  void set_node_parameters(int node, char cont_type, void* _arg);
  int * TCP_nodes;
  int num_nodes;
};

#endif
