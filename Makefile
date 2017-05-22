SERIAL_TARGET_BB=knapsackDYN_serial
MPI_TARGET_DYN=knapsackDYN_mpi
RESULTS=results.csv
RESULTS_SPEEDUPS=results_speedups.csv
RESULTS_TIMES=results_times_per_problem.csv
OUTPUT_FOLER=output
ERROR_FOLDER=errors

all : serial omp

dyn : $(SERIAL_TARGET_DYN) $(MPI_TARGET_DYN)

serial : $(SERIAL_TARGET_DYN)

$(SERIAL_TARGET_DYN) : knapsackDYN_serial.c
	gcc knapsackDYN_serial.c -o $(SERIAL_TARGET_DYN)

mpi : $(MPI_TARGET_DYN)

$(MPI_TARGET_SYN) : knapsackSYN_mpi.c
	mpicc knapsackDYN_mpi.c -o $(MPI_TARGET_DYN) $(MPI_TARGET_DYN)

run-moore : dyn
	./run_all.sh

