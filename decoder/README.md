# beamtest_hallc_decoder

This project is the decoder to the raw data of solid ecal beamtest in Hall C.

It includes 2 parts: fadc and ssp(gem).

This code is mainly inherited from C. Peng's fadc decoder and X. Bai's GEM decoder.

It requires cmake version > 3.0 and compile with gcc 8 and up and using c++ std 17.

In the farm nodes, not many version of root software is compiled with c++ std 17. 
We provide a version here: /u/home/pcrad/apps/root-6.22.02.

To compile this code, get into this dir, then do the next 2 lines:
source ./setup.csh
./install.csh
