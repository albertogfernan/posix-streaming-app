#include <stdio.h>
#include <iostream> // std::cout
#include <sstream>  // std::stringstream
#include <string>
#include "lp_lib.h"
#include "optimizer.h"

void pixel_to_qos(long pixel_diff, long thrs, int *QoS_int)
{
  if (pixel_diff >= thrs)
    *QoS_int = 1; // If greater or equal to threshold, then High QoS.
  else
    *QoS_int = 0; // Otherwise, Low QoS.

  return;
}

// REAL *optimizer(int argc, char *argv[])
REAL *optimizer(int QoS)
{
  # if defined ERROR
  #  undef ERROR
  # endif
  # define ERROR() { fprintf(stderr, "Error\n"); exit(1); }
    lprec *lp;
    int majorversion, minorversion, release, build;

  #if defined FORTIFY
    Fortify_EnterScope();
  #endif

  int num_variables = 3;
  // int number_of_CPUs, number_of_GPUs;
  int QoS_int; // 1: High QoS; 2: Low QoS
  int CPU_ST_performance;
  int CPU_MT_performance;
  int GPGPU_performance;
  int solver_ret;
  double TimeUpperBound;
  double PowerUpperBound;
  double row[1+num_variables];
  REAL *decision;
//int QoS_int; // 1: High QoS; 2: Low QoS

  // if (argc == 2)
  // {
    printf("%s\n", "Using performance values defined at design time\n");

    CPU_ST_performance = 95;
    CPU_MT_performance = 150;
    GPGPU_performance = 210;

    QoS_int = QoS;
    // std::stringstream str_QoS(argv[1]);
    // str_QoS >> QoS_int;
  // }
  // else if (argc == 5)
  // {
  //   printf("%s\n", "Using performance values returned by PAPI routine inside streaming-app\n");
  //
  //   std::stringstream str_QoS(argv[1]);
  //   str_QoS >> QoS_int;
  //   std::stringstream str_CPU_ST_performance(argv[2]);
  //   str_CPU_ST_performance >> CPU_ST_performance;
  //   std::stringstream str_CPU_MT_performance(argv[3]);
  //   str_CPU_MT_performance >> CPU_MT_performance;
  //   std::stringstream str_GPGPU_performance(argv[4]);
  //   str_GPGPU_performance >> GPGPU_performance;
  // }


// Formulating LP ==============================================================
  lp_solve_version(&majorversion, &minorversion, &release, &build);
  printf("Using lp_solve %d.%d.%d.%d \n\n", majorversion, minorversion, release, build);

  // std::stringstream str_QoS(argv[1]);
  // str_QoS >> QoS_int;

  // Print QoS
  if (QoS_int == 1){
    printf("QoS = %s\n", "High");
  }
  else if (QoS_int == 0){
    printf("QoS = %s\n", "Low");
  }

  printf("\nAdding %d variables\n", num_variables);
  lp = make_lp(0,num_variables);
  if (lp == NULL)
    ERROR();

  // Setting decision variables to be binary.
  set_binary(lp, 1, TRUE);
  set_binary(lp, 2, TRUE);
  set_binary(lp, 3, TRUE);

  // Setting cost function
  // In GFLOPS/W - Performance per Watt. Originally I used hypotetical values
  // from http://pcl.intel-research.net/publications/isca319-lee.pdf
  row[1] = CPU_ST_performance;
  row[2] = CPU_MT_performance;
  row[3] = GPGPU_performance;

  set_obj_fn(lp, row);
  set_maxim(lp);

  // Setting constraints. Each QoS has an specific set of constraints.
  // constraint 1: convexity --> Common to all QoS levels
  row[1] = 1; row[2] = 1; row[3] = 1;
  add_constraint(lp, row, EQ, 1);

  if (QoS_int == 1) { // if QoS == High
    // constraint 2: completion times
    row[1] = 1e-6; row[2] = 1.2e-7; row[3] = 1e-7; // In seconds - Completion in each core
    TimeUpperBound = 1e-6; //In seconds - Completion time bound.
    add_constraint(lp, row, LE, TimeUpperBound);
  }
  else if(QoS_int == 0) { // if QoS == Low
    // constraint 2: completion times
    row[1] = 200; row[2] = 300; row[3] = 340; // In watts - Power consumption in each core
    PowerUpperBound = 335; //In watts - power upper bound.
    add_constraint(lp, row, LE, PowerUpperBound);
  }

  //Show the current problem;
  print_lp(lp);

  // Solve LP
  printf("\nSolving the LP;\n");
  solver_ret = solve(lp);
  //TODO:  add switch case to print the final value of the problem: Optimal,
  // suboptimal etc.
  printf("%d", solver_ret);

// PRINTING OUTPUT +============================================================
  // Print objective
  print_objective(lp);
  // Print solutions, i.e. final variable values
  print_solution(lp, 1); // Uses 1 column to print the values

  //get_variables(lp, var);
  get_ptr_variables(lp, &decision);

  #if defined FORTIFY
    Fortify_LeaveScope();
  #endif

  return decision;

} // optimizer
