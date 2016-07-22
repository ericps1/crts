#ifndef _SC_PERFORMANCE_SWEEP_UTILITY_
#define _SC_PERFORMANCE_SWEEP_UTILITY_

#include "scenario_controller.hpp"

#define DEFAULT_DWELL_TIME_S 60
#define DEFAULT_SETTLE_TIME_S 2

// Two sweeping modes are defined. Nested sweeps will iterate
// through all permutations of the parameter values provided,
// linear sweeps will iterate through a list of specific parameter
// values
enum SweepModes {
  SWEEP_MODE_NESTED = 0,
  SWEEP_MODE_STEP
};

class SC_Performance_Sweep_Utility : public ScenarioController {

private:

  int debug_level;
  FILE *log;
  float dwell_time_s;
  float settle_time_s;
  bool node_1_feedback_received;
  bool node_2_feedback_received;
  char sweep_cfg_file[1024];
  char log_file_name[1024];

  int sweep_mode;
  int num_sweep_params;
  int num_data_points;
  int data_ind;
  std::vector<int> params;
  std::vector<int> num_vals;
  std::vector<void*> vals;
  std::vector<int> indices;
  std::vector<struct ExtensibleCognitiveRadio::rx_statistics> rx_stats;

  void print_sweep_point_summary();
  void update_sweep_params();
  void set_params(int param_ind);
  void read_sweep_cfg();
  void alloc_vals(int i);
public:
  SC_Performance_Sweep_Utility(int argc, char **argv);
  ~SC_Performance_Sweep_Utility();
  virtual void execute();
  virtual void initialize_node_fb();
};

#endif
