0.	to build:           
						./build_all.sh
	permissions:
						chmod +x ./build_all.sh

	to build manually each of the projects main directory:
	Algorithm A, Algorithm B, Simulator:
						cd ./AlgoA; mkdir build; cd ./build; cmake ..; make; cd ../../;
						cd ./AlgoB; mkdir build; cd ./build; cmake ..; make; cd ../../;
						cd ./Simulator; 			mkdir build; cd ./build; cmake ..; make; cd ../../;
	
1.	to execute:  		
						./<path to Simulator>/build/myrobot
	
	- execute program from ANY directory.
 	- the output files will be stored in the CWD.
						
	possible flags:
						-house_path: The directory that contains the .house files that represents
						houses need to be cleaned. If not used, then the main search for .house files
						in current directory. 
						default = CWD

						-algo_path: The directory that contains the .so file that are the shared
						libraries of the algorithms. If not used, then the main search for .so files in
						current directory. 
						default = CWD

						-num_thread: The number of threads that the main can use at most. If not
						used. 
						default = 10
						
						-summary_only: Output files won't be created, only the csv file.
						default = false

						-log: In order to create a log file for each algorithm-house pair.
						default = false

2.	simulation:
						python3 ./<path to Simulation>/Simulation.py ./<houseName>.house ./<outputName>.txt
   			