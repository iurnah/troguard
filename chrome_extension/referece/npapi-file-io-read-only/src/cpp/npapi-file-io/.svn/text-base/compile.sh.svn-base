for BITS in 32 64
do
  g++ npapi-file-io.cpp file-io.cpp -c -fPIC -Wall -DOS_LINUX=1 -I"third_party" -I"third_party/npapi" -m$BITS -O3 && g++ -shared -W1,-soname,npapi-file-io-$BITS.so -o npapi-file-io-$BITS.so npapi-file-io.o file-io.o -m$BITS -O3 && cp npapi-file-io-$BITS.so ../../../test/extension/
done
