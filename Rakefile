require 'rake/clean'

CLOBBER.include('mdr')
CLEAN.include('*.o')

task :default => 'mdr'

file 'mdr' => ['MakeDiffReadable.m', 'Reader.o', 'bstrlib.o'] do
  sh 'gcc -Wall bstrlib.o Reader.o MakeDiffReadable.m -o mdr -framework Cocoa -framework WebKit'
end

file 'Reader.o' => ['Reader.c'] do
  sh 'gcc -Wall -c Reader.c'
end

file 'bstrlib.o' => 'bstrlib.c' do
  # define BSTRLIB_NOVSNP macro because system doesn't support vsnprintf
  sh 'gcc -Wall -c bstrlib.c -DBSTRLIB_NOVSNP'
end
