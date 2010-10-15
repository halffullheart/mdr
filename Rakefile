require 'rake/clean'
require 'rbconfig'

CLOBBER.include('mdr')
CLEAN.include('*.o')

task :default => 'mdr'

if Config::CONFIG['target_os'] == 'mac' # TODO: Verrify 'mac' is correct
    file 'mdr' => ['mac/MakeDiffReadable.m', 'Reader.o', 'bstrlib.o'] do
      sh 'gcc -Wall -g bstrlib.o Reader.o mac/MakeDiffReadable.m -o mdr -framework Cocoa -framework WebKit'
    end
end

if Config::CONFIG['target_os'] == 'mingw32'
    file 'mdr' => ['win/MakeDiffReadable.c', 'Reader.o', 'bstrlib.o'] do
      sh 'gcc -Wall -g bstrlib.o Reader.o win/MakeDiffReadable.c -o mdr'
    end
end

file 'Reader.o' => ['Reader.c'] do
  sh 'gcc -Wall -g -c Reader.c'
end

file 'bstrlib.o' => 'bstrlib.c' do
  # define BSTRLIB_NOVSNP macro because system doesn't support vsnprintf
  sh 'gcc -Wall -g -c bstrlib.c -DBSTRLIB_NOVSNP'
end
