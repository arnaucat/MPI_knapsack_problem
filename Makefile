SERIAL_TARGET_BB=knapsackDYN_serial
MPI_TARGET_DYN=knapsackDYN_mpi
RESULTS=results.csv
RESULTS_SPEEDUPS=results_speedups.csv
RESULTS_TIMES=results_times_per_problem.csv
OUTPUT_FOLER=output
ERROR_FOLDER=errors

all : serial omp

bb : $(SERIAL_TARGET_DYN) $(MPI_TARGET_DYN)

serial : $(SERIAL_TARGET_DYN)

$(SERIAL_TARGET_DYN) : knapsackDYN_serial.c
	gcc knapsackDYN_serial.c -o $(SERIAL_TARGET_DYN)

omp : $(MPI_TARGET_DYN)

$(MPI_TARGET_SYN) : knapsackSYN_mpi.c
	mpicc knapsackDYN_mpi.c -o $(MPI_TARGET_DYN) $(MPI_TARGET_DYN)

run-moore : dyn
	./run_all.sh

results : $(RESULTS) $(RESULTS_TIMES) $(RESULTS_SPEEDUPS)

$(RESULTS) : $(OUTPUT_FOLER)/*
	./get_results.sh > $(RESULTS)

$(RESULTS_TIMES) : $(RESULTS)
	./get_times_per_problem.py $(RESULTS) > $(RESULTS_TIMES)

$(RESULTS_SPEEDUPS) : $(RESULTS)
	./get_speedups.py $(RESULTS) > $(RESULTS_SPEEDUPS)

plot : $(RESULTS_TIMES)
	./plot_generator.py $(RESULTS_TIMES)

clean :
	rm -rf $(SERIAL_TARGET_DYN) $(MPI_TARGET_DYN) $(RESULTS) $(RESULTS_SPEEDUPS) $(RESULTS_TIMES)

clean-outputs :
	rm -rf $(OUTPUT_FOLER)

clean-errors :
	rm -rf $(ERROR_FOLDER)