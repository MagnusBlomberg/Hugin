# Script to start logging data on platform HUGIN
rm main.mat& # remove previous logfile
../usr/hugin/hugin& �# Start hugin program that samples sensors
./main.elf& # start simulink RT program.
