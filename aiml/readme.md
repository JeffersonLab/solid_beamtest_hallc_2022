AIML code (copy and organize from /work/halla/solid/dwupton/Beamtest)

dir "Script_sim" to covert sim root file to sim root reduce file

dir "Pencil_Beam" to analyze sim output of pencil beam (shooting particle direction into the detector)
* Filter_Pencil_Beam.ipynb  read sim root reduce file and save to pkl file at /work/halla/solid/dwupton/Beamtest/Pencil_Beam/SavedSim_Pencil
* Sim_Pencil_View.ipynb and TID1_Pencil.ipynb read pkl file and mix them to save to csv file at /work/halla/solid/dwupton/Beamtest/Pencil_Beam/Sim_CSV

* Pencil_Cher_ML.ipynb    main code to read csv file and do PID

* others ... 


