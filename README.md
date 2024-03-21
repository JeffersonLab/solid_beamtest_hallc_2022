code for SoLID beamtest in hallc from 2022 to 2023

dir decoder is for code to decode coda output into root file 
starting from a copy of cmmit92e1840 of https://github.com/xbai0624/beamtest_hallc_decoder, which in turn is a fork from https://github.com/JixieZhang/beamtest_hallc_decoder
it's done by the following command
git clone --mirror <original-repository-url>
cd <cloned-repository-name>
git remote set-url --push origin <new-repository-url>
git push --mirror

dir analysis is for code to analyze decoder output root file
