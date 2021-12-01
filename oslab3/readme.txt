./mmu –f<num_frames> -a<algo> [-o<options>] inputfile randomfile 
(arguments can be in any order)
e.g. ./mmu -f4 -ac –oOPFS infile rfile

- MAKE A DIRECTORY FIRST: 
mkdir objects
cd scripts
./runit.sh ../inputs/ ../myout/ ../mmu
./gradeit.sh ../refout ../myout