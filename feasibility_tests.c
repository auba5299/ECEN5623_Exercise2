// Sam Siewert, August 2020
//
// This example code provides feasibiltiy decision tests for single core fixed priority rate monontic systems only (not dyanmic priority such as deadline driven
// EDF and LLF).  These are standard algorithms which either estimate feasibility (as the RM LUB does) or automate exact analysis (scheduling point, completion test) for
// a set services sharing one CPU core.  This can be emulated on Linux SMP multi-core systemes by use of POSIX thread affinity, to "pin" a thread to a specific core.
//
// Coded based upon standard definition of:
//
// 1) RM LUB based upon model by Liu and Layland
// 2) Scheduling Point - an exact feasibility algorithm based upon Lehoczky, Sha, and Ding exact analysis
// 3) Completion Test - an exact feasibility algorithm
//
// All 3 are also covered in RTECS with Linux and RTOS p. 84 to p. 89
//
// Original references for single core AMP systems:
//
// 1) RM LUB - Liu, Chung Laung, and James W. Layland. "Scheduling algorithms for multiprogramming in a hard-real-time environment." Journal of the ACM (JACM) 20.1 (1973): 46-61.
// 2) Scheduling Point - Lehoczky, John, Lui Sha, and Yuqin Ding. "The rate monotonic scheduling algorithm: Exact characterization and average case behavior." RTSS. Vol. 89. 1989.
// 3) Completion Test - Joseph, Mathai, and Paritosh Pandya. "Finding response times in a real-time system." The Computer Journal 29.5 (1986): 390-395.
//
// References for mulit-core systems:
//
// 1) Bertossi, Alan A., Luigi V. Mancini, and Federico Rossini. "Fault-tolerant rate-monotonic first-fit scheduling in hard-real-time systems."
//    IEEE Transactions on Parallel and Distributed Systems 10.9 (1999): 934-945.
// 2) Burchard, Almut, et al. "New strategies for assigning real-time tasks to multiprocessor systems." IEEE transactions on computers 44.12 (1995): 1429-1442.
// 3) Dhall, Sudarshan K., and Chung Laung Liu. "On a real-time scheduling problem." Operations research 26.1 (1978): 127-140.
//
//
// Deadline Montonic (not implemented in this example, but covered in class and notes):
//
// 1) Audsley, Neil C., et al. "Hard real-time scheduling: The deadline-monotonic approach." IFAC Proceedings Volumes 24.2 (1991): 127-132.
//
// Note that Deadline Monotoic simply uses the deadine interval, D(i) to assign priority, rather than the period interval, T(i) and relaxes T=D constraint.  Anlaysis can
// be done as it is done for RM, but with evaluation of feasbility based upon modified D(i) and with modified fixed priorities.  This is covered by manual analysis examples.
//
// For a more interactive tool, students can use Cheddar:
//
// http://beru.univ-brest.fr/~singhoff/cheddar/
//
// This open source tool handles single and multi-core and allows for modeling of the platform hardware, RTOS/OS, and scheduler with a particular fixed priority or dynamic
// priority policy.
//
// This code is provided primarily so students can learn the methods of worst case analysis and compare exact and estimated feasibility decision testing.
//

//LIBRARIES - so far nothing related to threads is included
#include <math.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define U32_T unsigned int

//EXAMPLE SERVICES
// EX0: U=0.7333
U32_T ex0_period[] = {2, 10, 15};
U32_T ex0_wcet[] = {1, 1, 2};

// EX1: U=0.9857
U32_T ex1_period[] = {2, 5, 7};
U32_T ex1_wcet[] = {1, 1, 2};

// EX2: U=0.9967
U32_T ex2_period[] = {2, 5, 7, 13};
U32_T ex2_wcet[] = {1, 1, 1, 2};

// EX3: U=0.93
U32_T ex3_period[] = {3, 5, 15};
U32_T ex3_wcet[] = {1, 2, 3};

// EX4: U=1.0
U32_T ex4_period[] = {2, 4, 16};
U32_T ex4_wcet[] = {1, 1, 4};


//////BEGIN PART 2 WHERE MUST TEST RM, EDF, AND LLF//////
// EX5: U=1.0
U32_T ex5_period[] = {2, 5, 10};
U32_T ex5_wcet[] = {1, 2, 1};

// EX6: DEADLINE MONOTONIC EXAMPLE

// EX7: U=1.0
U32_T ex7_period[] = {3, 5, 15};
U32_T ex7_wcet[] = {1, 2, 4};

// EX8: U=0.9967
U32_T ex8_period[] = {2, 5, 7, 13};
U32_T ex8_wcet[] = {1, 1, 1, 2};

// EX9: U=1.0
U32_T ex9_period[] = {6, 8, 12, 24};
U32_T ex9_wcet[] = {1, 2, 4, 6};

//FUNCTION PROTOTYPES
int completion_time_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);
int scheduling_point_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);
int rate_monotonic_least_upper_bound(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);
int utilization_100_test(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);


int main(void)
{ 
    int i;
	U32_T numServices;
    
    // COMPLETION TESTS
    printf("******** Completion Test Feasibility Example\n");
   
    // EXAMPLE 0
    printf("Ex-0 U=%4.2f%% (C1=1, C2=1, C3=2; T1=2, T2=10, T3=15; T=D): ",
		   ((1.0/2.0)*100.0 + (1.0/10.0)*100.0 + (2.0/15.0)*100.0));
	numServices = 3;
    if(completion_time_feasibility(numServices, ex0_period, ex0_wcet, ex0_period) == TRUE)
        printf("CT test FEASIBLE\n");
    else
        printf("CT test INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex0_period, ex0_wcet, ex0_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");

    // EXAMPLE 1
    printf("Ex-1 U=%4.2f%% (C1=1, C2=1, C3=2; T1=2, T2=5, T3=7; T=D): ", 
		   ((1.0/2.0)*100.0 + (1.0/5.0)*100.0 + (2.0/7.0)*100.0));
	numServices = 3;
    if(completion_time_feasibility(numServices, ex1_period, ex1_wcet, ex1_period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex1_period, ex1_wcet, ex1_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");

	// EXAMPLE 2
    printf("Ex-2 U=%4.2f%% (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): ",
		   ((1.0/2.0)*100.0 + (1.0/5.0)*100.0 + (1.0/7.0)*100.0 + (2.0/13.0)*100.0));
	numServices = 4;
    if(completion_time_feasibility(numServices, ex2_period, ex2_wcet, ex2_period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex2_period, ex2_wcet, ex2_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");

    // EXAMPLE 3
    printf("Ex-3 U=%4.2f%% (C1=1, C2=2, C3=3; T1=3, T2=5, T3=15; T=D): ",
		   ((1.0/3.0)*100.0 + (2.0/5.0)*100.0 + (3.0/15.0)*100.0));
	numServices = 3;
    if(completion_time_feasibility(numServices, ex3_period, ex3_wcet, ex3_period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex3_period, ex3_wcet, ex3_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");

	// EXAMPLE 4
    printf("Ex-4 U=%4.2f%% (C1=1, C2=1, C3=4; T1=2, T2=4, T3=16; T=D): ",
		   ((1.0/2.0)*100.0 + (1.0/4.0)*100.0 + (4.0/16.0)*100.0));
	numServices = 3;
    if(completion_time_feasibility(numServices, ex4_period, ex4_wcet, ex4_period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex4_period, ex4_wcet, ex4_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");


    // SCHEDULING POINT TESTS + RM LUB
    printf("\n\n");
    printf("******** Scheduling Point Feasibility Example\n");

    // EXAMPLE 0
    printf("Ex-0 U=%4.2f%% (C1=1, C2=1, C3=2; T1=2, T2=10, T3=15; T=D): ",
		   ((1.0/2.0)*100.0 + (1.0/10.0)*100.0 + (2.0/15.0)*100.0));
	numServices = 3;
    if(scheduling_point_feasibility(numServices, ex0_period, ex0_wcet, ex0_period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex0_period, ex0_wcet, ex0_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");

    // EXAMPLE 1
    printf("Ex-1 U=%4.2f%% (C1=1, C2=1, C3=2; T1=2, T2=5, T3=7; T=D): ", 
		   ((1.0/2.0)*100.0 + (1.0/5.0)*100.0 + (2.0/7.0)*100.0));
	numServices = 3;
    if(scheduling_point_feasibility(numServices, ex1_period, ex1_wcet, ex1_period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex1_period, ex1_wcet, ex1_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");

	// EXAMPLE 2
    printf("Ex-2 U=%4.2f%% (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): ",
		   ((1.0/2.0)*100.0 + (1.0/5.0)*100.0 + (1.0/7.0)*100.0 + (2.0/13.0)*100.0));
	numServices = 4;
    if(scheduling_point_feasibility(numServices, ex2_period, ex2_wcet, ex2_period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex2_period, ex2_wcet, ex2_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");

    // EXAMPLE 3
    printf("Ex-3 U=%4.2f%% (C1=1, C2=2, C3=3; T1=3, T2=5, T3=15; T=D): ",
		   ((1.0/3.0)*100.0 + (2.0/5.0)*100.0 + (3.0/15.0)*100.0));
	numServices = 3;
    if(scheduling_point_feasibility(numServices, ex3_period, ex3_wcet, ex3_period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex3_period, ex3_wcet, ex3_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");

	// EXAMPLE 4
    printf("Ex-4 U=%4.2f%% (C1=1, C2=1, C3=4; T1=2, T2=4, T3=16; T=D): ",
		   ((1.0/2.0)*100.0 + (1.0/4.0)*100.0 + (4.0/16.0)*100.0));
	numServices = 3;
    if(scheduling_point_feasibility(numServices, ex4_period, ex4_wcet, ex4_period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");


    if(rate_monotonic_least_upper_bound(numServices, ex4_period, ex4_wcet, ex4_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");

}


int rate_monotonic_least_upper_bound(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[])
{
  double utility_sum=0.0, lub=0.0;
  int idx;

  printf("for %d, utility_sum = %lf\n", numServices, utility_sum);

  // Sum the C(i) over the T(i)
  for(idx=0; idx < numServices; idx++)
  {
    utility_sum += ((double)wcet[idx] / (double)period[idx]);
    printf("for %d, wcet=%lf, period=%lf, utility_sum = %lf\n", idx, (double)wcet[idx], (double)period[idx], utility_sum);
  }
  printf("utility_sum = %lf\n", utility_sum);

  // Compute LUB for number of services
  lub = (double)numServices * (pow(2.0, (1.0/((double)numServices))) - 1.0);
  printf("LUB = %lf\n", lub);

  // Compare the utilty to the bound and return feasibility
  if(utility_sum <= lub)
	  return TRUE;
  else
	  return FALSE;
}

/* The completion‑time feasibility test computes the worst‑case response time
 * for each task by accounting for interference from all higher‑priority tasks.
 * We begin with an initial estimate equal to the sum of execution times for
 * the task and all higher‑priority tasks. We then iteratively add additional
 * interference based on how many times each higher‑priority task can release
 * within the current response‑time estimate. This process continues until the
 * value converges, giving the worst‑case completion time under maximum load.
 * If this final response time does not exceed the task’s deadline, the task
 * is schedulable under fixed‑priority (RM) analysis. NOTE: I had an AI revise this comment as 
 * I was getting a bit wordy but it matches my thoughts and is based on my description.
 */
int completion_time_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[])
{
  int i, j;
  U32_T an, anext;
  
  // assume feasible until we find otherwise
  int set_feasible=TRUE;
   
  //printf("numServices=%d\n", numServices);
 
  // For all services in the analysis - why the double loop?
  for (i=0; i < numServices; i++)
  {
       an=0; anext=0;
       
       for (j=0; j <= i; j++)
       {
           an+=wcet[j];
       }
       
	   //printf("i=%d, an=%d\n", i, an);

       while(1)
       {
             anext=wcet[i];
	     
             for (j=0; j < i; j++)
                 anext += ceil(((double)an)/((double)period[j]))*wcet[j];
		 
             if (anext == an)
                break;
             else
                an=anext;

			 //printf("an=%d, anext=%d\n", an, anext);
       }
       
	   //printf("an=%d, deadline[%d]=%d\n", an, i, deadline[i]);

       if (an > deadline[i])
       {
          set_feasible=FALSE;
       }
  }
  
  return set_feasible;
}

/* To the best of my knowledge... The scheduling‑point feasibility test examines all critical instants, which occur at multiples of higher‑priority task periods, up to the
 * period of the task being analyzed. At each such time t, the test checks whether the processor can supply enough CPU time to handle all jobs
 * released by tasks of equal or higher priority. The right side of the inequality is the available CPU time t, while the left side is the sum
 * of each higher‑priority task’s execution time multiplied by the number of times it can release within that window. If demand is less than or
 * equal to supply at any scheduling point, the task is feasible; if not, the task set is infeasible under fixed‑priority scheduling.
 */
int scheduling_point_feasibility(U32_T numServices, U32_T period[], 
				 U32_T wcet[], U32_T deadline[])
{
   int rc = TRUE, i, j, k, l, status, temp;

   // For all services in the analysis
   for (i=0; i < numServices; i++) // iterate from highest to lowest priority
   {
      status=0;

      // Look for all available CPU minus what has been used by higher priority services
      for (k=0; k<=i; k++) 
      {
	  // find available CPU windows and take them
          for (l=1; l <= (floor((double)period[i]/(double)period[k])); l++)
          {
               temp=0;

               for (j=0; j<=i; j++) temp += wcet[j] * ceil((double)l*(double)period[k]/(double)period[j]);

	       // Can we get the CPU we need or not?
               if (temp <= (l*period[k]))
			   {
				   // insufficient CPU during our period, therefore infeasible
				   status=1;
				   break;
			   }
           }
           if (status) break;
      }

      if (!status) rc=FALSE;
   }
   return rc;
}

/*This is a simple function to compare utilization against 100%. For dynamic priority algorithms such as EDF and LLF,
* this is a necessary and sufficient condition for feasibility. If under 100% utilization, the system is feasible using a
* EDF or LLF shceduler. If over 100% the schedule is infeasible.
*/
int utilization_100_test(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]){
    
    double utility_sum=0.0;
    int idx;

    // Sum the C(i) over the T(i) for all services
    for(idx=0; idx < numServices; idx++)
    {
        utility_sum += ((double)wcet[idx] / (double)period[idx]);
    }

    // Compare the utilty to 1.0 and return feasibility if under this value
    if(utility_sum <= 1.0)
        return TRUE;
    else
        return FALSE;
}
