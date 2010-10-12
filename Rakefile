require 'rake/clean'

CLOBBER.include('mdr')
CLEAN.include('Reader.o')

task :default => 'mdr'

file 'mdr' => ['MakeDiffReadable.m', 'Reader.o', 'Reader.h'] do
  sh 'gcc Reader.o MakeDiffReadable.m -o mdr -framework Cocoa -framework WebKit'
end

file 'Reader.o' => 'Reader.c' do
  sh 'gcc -c Reader.c -o Reader.o'
end
