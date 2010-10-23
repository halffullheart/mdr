require 'rake/clean'
require 'rbconfig'

CLOBBER.include('mdr')
CLOBBER.include('mdr.exe')
CLOBBER.include('release/mdr')
CLEAN.include('*.o')
CLEAN.include('*.css.h')

task :default => 'build'
task :build => 'mdr'
task :release => 'release/mdr'

if Config::CONFIG['host_vendor'] == 'apple'

  file 'mdr' => ['mac/MakeDiffReadable.m', 'Reader.o', 'bstrlib.o'] do
    sh 'gcc -Wall -g bstrlib.o Reader.o mac/MakeDiffReadable.m -o mdr -framework Cocoa -framework WebKit'
  end

  file 'release/mdr' => ['mac/MakeDiffReadable.m', 'Reader.o', 'bstrlib.o'] do
    sh 'gcc -Wall -02 bstrlib.o Reader.o mac/MakeDiffReadable.m -o release/mdr -framework Cocoa -framework WebKit'
  end

end

if Config::CONFIG['target_os'] == 'mingw32'

  task :mdr => 'mdr.exe'
  task 'release/mdr' => 'release/mdr.exe'

  file 'mdr.exe' => ['win/MakeDiffReadable.c', 'Reader.o', 'bstrlib.o'] do
    sh 'gcc -Wall -g bstrlib.o Reader.o win/MakeDiffReadable.c -o mdr'
  end

  file 'release/mdr.exe' => ['win/MakeDiffReadable.c', 'Reader.o', 'bstrlib.o'] do
    sh 'gcc -Wall -02 bstrlib.o Reader.o win/MakeDiffReadable.c -o release/mdr'
  end

end

file 'Reader.o' => ['Reader.c', 'style.css.h', 'Reader.h'] do
  sh 'gcc -Wall -g -c Reader.c'
end

file 'bstrlib.o' => 'bstrlib.c' do
  # define BSTRLIB_NOVSNP macro because system doesn't support vsnprintf
  sh 'gcc -Wall -g -c bstrlib.c -DBSTRLIB_NOVSNP'
end

file 'test' => ['Reader.o', 'bstrlib.o', 'test.c'] do
  sh 'gcc -Wall -g bstrlib.o Reader.o test.c -o test'
end

file 'style.css.h' => 'css/style.css' do
  sh 'cd css; xxd -i style.css > ../style.css.h'
end
