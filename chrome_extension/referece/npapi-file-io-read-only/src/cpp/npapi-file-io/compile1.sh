#32 bits
  echo step 1
  g++ npapi-file-io.cpp file-io.cpp -c -fPIC -Wall -DOS_LINUX=1 -I"third_party" -I"third_party/npapi" -m32 -O3 
  echo step 2
  g++ -shared -W1,-soname,npapi-file-io-32.so -o npapi-file-io-32.so npapi-file-io.o file-io.o -m32 -O3
  echo step 3
# cp npapi-file-io-32.so ../../../test/extension/
#64 bits	
#  echo step 1
#  g++ npapi-file-io.cpp file-io.cpp -c -fPIC -Wall -DOS_LINUX=1 -I"third_party" -I"third_party/npapi" -m64 -O3 
#  echo step 2
#  g++ -shared -W1,-soname,npapi-file-io-64.so -o npapi-file-io-64.so npapi-file-io.o file-io.o -m64 -O3
#  echo step 3
#  cp npapi-file-io-64.so ../../../test/extension/

	
