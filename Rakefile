require 'rake/clean'
require 'rbconfig'

CLOBBER.include('mdr')
CLOBBER.include('mdr.exe')
CLOBBER.include('release/mdr')
CLEAN.include('*.o')
CLEAN.include('*.css.h')

@dev_exe = 'mdr'
@dev_flags = %w{-Wall -g}
@release_flags = %w{-02}
@exe_flags = []

if Config::CONFIG['target_os'] == 'mingw32'
  @dev_exe = 'mdr.exe'
end

if Config::CONFIG['host_vendor'] == 'apple'
  @exe_flags = [
    '-framework Cocoa',
    '-framework WebKit'
  ]
  mac_flags = [
    '-arch i386', # 32bit Intel
    '-arch ppc', # 32bit PPC
    '-isysroot /Developer/SDKs/MacOSX10.5.sdk', # Base SDK
    '-mmacosx-version-min=10.4', # Mac OS X Deployment Target
  ];
  @dev_flags.push mac_flags
  @release_flags.push mac_flags
end

@release_exe = "release/#{@dev_exe}"

task :default => 'build'
task :build => @dev_exe
task :release => @release_exe

file @dev_exe => ['mac/MakeDiffReadable.m', 'Reader.o', 'bstrlib.o'] do
  sh "gcc #{@dev_flags.join ' '} #{@exe_flags.join ' '} bstrlib.o Reader.o mac/MakeDiffReadable.m -o #{@dev_exe}"
end

file @release_exe => ['mac/MakeDiffReadable.m', 'Reader.o', 'bstrlib.o'] do
  sh "gcc #{@release_flags.join ' '} #{@exe_flags.join ' '} bstrlib.o Reader.o mac/MakeDiffReadable.m -o #{@release_exe}"
end

file 'Reader.o' => ['Reader.c', 'style.css.h', 'Reader.h'] do
  sh "gcc #{@dev_flags.join ' '} -c Reader.c"
end

file 'bstrlib.o' => 'bstrlib.c' do
  # define BSTRLIB_NOVSNP macro because system doesn't support vsnprintf
  sh "gcc #{@dev_flags.join ' '} -c bstrlib.c -DBSTRLIB_NOVSNP"
end

file 'test' => ['Reader.o', 'bstrlib.o', 'test.c'] do
  sh "gcc #{@dev_flags.join ' '} bstrlib.o Reader.o test.c -o test"
end

file 'style.css.h' => 'css/style.css' do
  sh 'cd css; xxd -i style.css > ../style.css.h'
end
