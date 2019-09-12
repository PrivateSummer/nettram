#nettram mean 'net tram', just a name, called nt for short

#build libevent

tar -zxvf libevent-1.4.13-stable.tar.gz
cd libevent-1.4.13-stable
./configure
make
make install


#build nt

mkdir build
cd build
cmake ..
make

#if your gcc compiler with a version lower than 4.3: cmake .. -DWITH_BOOST=ON -DBOOST_INCLUDE_DIR:PATH=/xx/boost_1_55_0 -DBOOST_LIB_DIR:PATH=/xx/boost_1_55_0/stage/lib 


#run example

cd httpbroke
./httpbroke

