{
    gSystem->SetBuildDir(".root_build_dir", true);
    //install dir
    char *HallCBeamtestDir = std::getenv("HallCBeamtestDir");
    // analyzer
    gInterpreter->AddIncludePath(Form("%s/include",HallCBeamtestDir));
    gSystem->Load(Form("%s/lib64/libfdec.so",HallCBeamtestDir));


    // online monitor
    /*
    gInterpreter->AddIncludePath("./online");
    gInterpreter->AddIncludePath("./third_party/et/include/coda/et");
    gSystem->Load("./third_party/et/lib64/libcoda_et.so");
    */
}
