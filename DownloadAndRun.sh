export LD_LIBRARY_PATH=/usr/local/lib/
export IP_ADDRESS=$1
echo '----- Download ----- '$IP_ADDRESS
scp -r --preserve josh@$IP_ADDRESS:'Robotics/RaspberryPi/RPi2014/*' ./ > /dev/null
echo '----- Compile ------'
if make raspberry_pi_vision
then
	echo "----- Running -----"
	LD_PRELOAD=/usr/lib/uv4l/uv4lext/armv6l/libuv4lext.so ./raspberry_pi_vision
fi

