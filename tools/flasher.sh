if [ -f "flashed.log" ];
then 
 echo 'sin novedades...'
else
 echo 'grabando...'
  ./lm4flash_64b -E  ../../gcc/out.bin 
  touch "flashed.log"
fi
#./openocd.sh
