require 'rake/clean'
require 'rake/testtask'
require 'rbconfig'
require 'fileutils'

# Unity stuff
UNITY_ROOT = File.expand_path('vendor/unity') + '/'
require UNITY_ROOT + 'rakefile_helper'
include RakefileHelpers
DEFAULT_CONFIG_FILE = 'gcc_32.yml'
configure_toolchain(DEFAULT_CONFIG_FILE)

# Some quick definitions:
#   System = mac, linux, windows, etc.
#   Target = dev, test, release, etc.

SYSTEM = case RbConfig::CONFIG['host_os']
  when /mswin|msys|mingw|cygwin|bccwin|wince|emc/
    :windows
  when /darwin|mac os/
    :mac
  when /linux/
    :linux
  when /solaris|bsd/
    :unix
  else
    raise "Unknown OS"
  end

TARGETS     = [:dev, :release]
SRC_PATH    = 'src'
BUILD_PATH  = 'build'
BIN_NAME    = 'mdr'
TMP         = 'tmp'
BIN_EXT     = SYSTEM == :windows ? '.exe' : ''
BIN         = BIN_NAME + BIN_EXT
BIN_PATHS   = TARGETS.map { |t| { t => File.join(BUILD_PATH, t.to_s, BIN) } }.reduce(&:merge)
TMP_PATHS   = TARGETS.map { |t| { t => File.join(BUILD_PATH, t.to_s, TMP) } }.reduce(&:merge)
CFLAGS      = TARGETS.map { |t| { t => ["-I#{ TMP_PATHS[t] }"] } }.reduce(&:merge)
LFLAGS      = TARGETS.map { |t| { t => ["-I#{ TMP_PATHS[t] }"] } }.reduce(&:merge)

CFLAGS[:all] = %w( -I. )
LFLAGS[:all] = %w( -I. )

def src(*f)
  File.join(SRC_PATH, *f)
end

def tmp(target, *f)
  File.join(TMP_PATHS[target], *f)
end

def cflags(target)
  (CFLAGS[:all] + CFLAGS[target]).join(' ')
end

def lflags(target)
  (LFLAGS[:all] + LFLAGS[target]).join(' ')
end

CFLAGS[:dev] += %w( -Wall -g )

CLOBBER.include(BIN_PATHS.values)
CLEAN.include(TMP_PATHS.values.map { |p| File.join(p, '**') })

# Files that are dependencies and are fed to compiler
extra_objects = []

# Files that are dependencies that must be built but do not have to be fed to
# compiler explicitly when making the final binary.
extra_headers = ['style.css.h']

main_file = File.join(SRC_PATH, SYSTEM.to_s, 'main.c')

if SYSTEM == :windows
  extra_objects << 'Resources.o'
end

if SYSTEM == :mac
  main_file = File.join(SRC_PATH, 'mac', 'main.m')
  LFLAGS[:all] += [
    '-framework Cocoa',
    '-framework WebKit',

    #'-arch i386', # 32bit Intel
    #'-arch ppc', # 32bit PPC

    # Base SDK, run `xcodebuild -sdk -version` to see possible options
    '-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk',

    # Mac OS X Deployment Target
    '-mmacosx-version-min=10.8',
  ]
  CFLAGS[:release] += ['-03']
  extra_headers   += ['appIcon.png.h']
end

if SYSTEM == :linux
  LFLAGS[:all] += [
    `pkg-config --libs --cflags webkitgtk-3.0`.chomp
  ]
  extra_headers   += ['appIcon.png.h']
end

task :default     => :build
task :build       => BIN_PATHS[:dev]
task :release     => BIN_PATHS[:release]

task :test do
  run_tests get_unit_test_files
end

TARGETS.each do |target|
  directory(tmp(target))

  linked = ['mdr.o', 'bstrlib.o']
  deps = (linked + extra_objects + extra_headers).map { |f| tmp(target, f) }
  inputs = (linked + extra_objects).map { |f| tmp(target, f) }
  file BIN_PATHS[target] => deps do
    sh "gcc #{ main_file } #{ inputs.join(' ') } #{ lflags(target) } -o #{ BIN_PATHS[target] }"
  end

  file tmp(target, 'mdr.o') => [src('mdr.c'), src('mdr.h'), src('version.h'), tmp(target, 'style.css.h')] do
    sh "gcc #{ cflags(target) } -c #{ src('mdr.c') } -o #{ tmp(target, 'mdr.o') }"
  end

  file tmp(target, 'bstrlib.o') => [src('bstrlib.c'), src('bstrlib.h'), tmp(target)] do
    # define BSTRLIB_NOVSNP macro because system may not support vsnprintf, and we don’t need it.
    sh "gcc #{ cflags(target) } -c #{ src('bstrlib.c') } -o #{ tmp(target, 'bstrlib.o') } -DBSTRLIB_NOVSNP"
  end

  file tmp(target, 'appIcon.png.h') => [src('appIcon.png'), tmp(target)] do
    sh "xxd -i #{ src('appIcon.png') } | sed 's/src_//g' > #{ tmp(target, 'appIcon.png.h') }"
  end

  file tmp(target, 'style.css.h') => [src('style.css'), tmp(target)] do
    sh "xxd -i #{ src('style.css') } | sed 's/src_//g' > #{ tmp(target, 'style.css.h') }"
  end

  file tmp(target, 'Resources.o') => [src('windows', 'Resources.rc'), src('version.h'), tmp(target)] do
    sh "windres #{ src('windows', 'Resources.rc') } #{ tmp(target, 'Resources.o') }"
  end

end
