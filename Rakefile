require 'rake/clean'
require 'rbconfig'

CLOBBER.include('mdr')
CLOBBER.include('mdr.exe')
CLOBBER.include('release/mdr')
CLOBBER.include('release/mdr.exe')
CLOBBER.include('release/mdr.zip')
CLEAN.include('*.o')
CLEAN.include('*.css.h')
CLEAN.include('*.png.h')

@dev_exe = 'mdr'
@dev_flags = %w{-Wall -g}
@release_flags = []
@exe_flags = []

# Files that are dependencies and are fed to compiler
@extra_objects = []

# Files that are dependencies but must only be build and do not have to be fed
# to compiler explicitly.
@extra_deps = []

@system = case RbConfig::CONFIG['host_os']
  when /mswin|msys|mingw|cygwin|bccwin|wince|emc/
    :windows
  when /darwin|mac os/
    :macosx
  when /linux/
    :linux
  when /solaris|bsd/
    :unix
  else
    raise "Unknown OS: #{host_os.inspect}"
  end

if @system == :windows
  @main_file = 'win/MakeDiffReadable.c'
  @dev_exe = 'mdr.exe'
  @extra_objects.push 'Resources.o'
end

if @system == :macosx
  @main_file = 'mac/MakeDiffReadable.m'
  @exe_flags = [
    '-framework Cocoa',
    '-framework WebKit'
  ]
  mac_flags = [
    '-arch i386', # 32bit Intel
    #'-arch ppc', # 32bit PPC

    # Base SDK, run `xcodebuild -sdk -version` to see possible options
    '-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk',

    # Mac OS X Deployment Target
    '-mmacosx-version-min=10.8',
  ];
  @dev_flags.push mac_flags
  @release_flags.push mac_flags
  @release_flags.push '-03'
  @extra_deps.push 'appIcon.png.h'
end

if @system == :linux
  @main_file = 'linux/MakeDiffReadable.c'
  linux_flags = [
    `pkg-config --libs --cflags webkit2gtk-3.0`.chomp
  ]

  @dev_flags.push linux_flags
  @release_flags.push linux_flags
  @release_flags.push '-O3'
end

@release_exe = "release/#{@dev_exe}"

task :default => 'build'
task :build => @dev_exe
task :release => @release_exe
task :package => 'release/mdr.zip'

file @dev_exe => [@main_file, 'Reader.o', 'bstrlib.o'] + @extra_objects + @extra_deps do
  sh "gcc bstrlib.o Reader.o #{@main_file} #{@dev_flags.join ' '} #{@exe_flags.join ' '} #{@extra_objects.join ' '} -o #{@dev_exe}"
end

file @release_exe => [@main_file, 'Reader.o', 'bstrlib.o'] + @extra_objects do
  sh "gcc #{@release_flags.join ' '} #{@exe_flags.join ' '} #{@extra_objects.join ' '} bstrlib.o Reader.o #{@main_file} -o #{@release_exe}"
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
  Dir.chdir 'css' do
    sh 'xxd -i style.css > ../style.css.h'
  end
end

file 'appIcon.png.h' => 'img/appIcon.png' do
  Dir.chdir 'img' do
    sh 'xxd -i appIcon.png > ../appIcon.png.h'
  end
end

file 'Resources.o' => 'win/Resources.rc' do
  sh 'windres win/Resources.rc Resources.o'
end

file 'release/mdr.zip' => @release_exe do
  Dir.chdir 'release' do
    sh 'cp ../README.md .' # Copy README into release dir so it will be in the zip.
    sh 'zip mdr.zip *'
    sh 'rm README.md' # But don't leave it there.
  end
end

task :install => @release_exe do
  sh "cp #{@release_exe} ~/bin/mdr"
end

task :uninstall do
  sh 'rm ~/bin/mdr'
end
