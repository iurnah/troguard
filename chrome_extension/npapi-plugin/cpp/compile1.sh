  g++ drive-by-download-plugin.cpp drive-by-download.cpp -c -fPIC -Wall -DOS_LINUX=1 -I"third_party" -I"third_party/npapi" -m64 -O3 
  g++ -shared -W1,-soname,drive-by-download-plugin-64.so -o drive-by-download-plugin-64.so drive-by-download-plugin.o drive-by-download.o -m64 -O3
  cp drive-by-download-plugin-64.so ../test/extension/

	
