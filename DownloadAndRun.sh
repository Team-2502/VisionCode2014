export LD_LIBRARY_PATH=/usr/local/lib/
export IP_ADDRESS=$1
echo '----- Download ----- '$IP_ADDRESS
scp -pr josh@$IP_ADDRESS:'Robotics/RaspberryPi/RPi2014/*' ./ > /dev/null
echo '----- Compile ------'
date1=$(date +"%s")
if make raspberry_pi_vision
then
	date2=$(date +"%s")
	diff=$(($date2-$date1))
	echo "$(($diff / 60)) minutes and $(($diff % 60)) seconds elapsed."
	echo "----- Running -----"
	sudo LD_PRELOAD=/usr/lib/uv4l/uv4lext/armv6l/libuv4lext.so ./raspberry_pi_vision
else
	date2=$(date +"%s")
	diff=$(($date2-$date1))
	echo "$(($diff / 60)) minutes and $(($diff % 60)) seconds elapsed."
fi

