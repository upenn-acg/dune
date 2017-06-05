source $stdenv/setup

# Call top level directory
cd $src
sudo make
mv kern/dune.ko $out
