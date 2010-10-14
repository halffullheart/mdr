require 'rake/clean'

CLOBBER.include('mdr')
CLEAN.include('*.o')

file 'mac' => ['mac/MakeDiffReadable.m', 'Reader.o', 'bstrlib.o'] do
  sh 'gcc -Wall -g bstrlib.o Reader.o mac/MakeDiffReadable.m -o build/mdr -framework Cocoa -framework WebKit'
end

file 'Reader.o' => ['Reader.c'] do
  sh 'gcc -Wall -g -c Reader.c'
end

file 'bstrlib.o' => 'bstrlib.c' do
  # define BSTRLIB_NOVSNP macro because system doesn't support vsnprintf
  sh 'gcc -Wall -g -c bstrlib.c -DBSTRLIB_NOVSNP'
end
