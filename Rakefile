require 'rake/clean'
require 'rbconfig'
require 'fileutils'

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
CFLAGS      = Hash.new([])
LFLAGS      = Hash.new([])

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

def header_case(filename)
  filename.gsub(/[\.\/-]/, '_').upcase
end

def mktmp(target)
  FileUtils.mkdir_p(tmp(target))
end


CFLAGS[:all] += %w( -I. )
CFLAGS[:dev] += %w( -Wall -g )

LFLAGS[:all] += %w( -I. )

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

task :default => :build
task :build   => BIN_PATHS[:dev]
task :release => BIN_PATHS[:release]
#task :package => 'release/mdr.zip'

TARGETS.each do |target|
  # Add flags defining all the extra header locations.
  define_header_flags = extra_headers.map { |h| %(-D#{ header_case(h) }=\\"#{ tmp(target, h) }\\") }
  CFLAGS[target] += define_header_flags
  LFLAGS[target] += define_header_flags

  linked = ['mdr.o', 'bstrlib.o']
  deps = (linked + extra_objects + extra_headers).map { |f| tmp(target, f) }
  inputs = (linked + extra_objects).map { |f| tmp(target, f) }
  file BIN_PATHS[target] => deps do
    sh "gcc #{ main_file } #{ inputs.join(' ') } #{ lflags(target) } -o #{ BIN_PATHS[target] }"
  end

  file tmp(target, 'mdr.o') => [src('mdr.c'), src('mdr.h'), src('version.h'), tmp(target, 'style.css.h')] do
    sh "gcc #{ cflags(target) } -c #{ src('mdr.c') } -o #{ tmp(target, 'mdr.o') }"
  end

  file tmp(target, 'bstrlib.o') => [src('bstrlib.c'), src('bstrlib.h')] do
    mktmp(target)
    # define BSTRLIB_NOVSNP macro because system may not support vsnprintf, and we don’t need it.
    sh "gcc #{ cflags(target) } -c #{ src('bstrlib.c') } -o #{ tmp(target, 'bstrlib.o') } -DBSTRLIB_NOVSNP"
  end

  file tmp(target, 'appIcon.png.h') => src('appIcon.png') do
    mktmp(target)
    sh "xxd -i #{ src('appIcon.png') } | sed 's/src_//g' > #{ tmp(target, 'appIcon.png.h') }"
  end

  file tmp(target, 'style.css.h') => src('style.css') do
    mktmp(target)
    sh "xxd -i #{ src('style.css') } | sed 's/src_//g' > #{ tmp(target, 'style.css.h') }"
  end

  file tmp(target, 'Resources.o') => [src('windows', 'Resources.rc'), src('version.h')] do
    mktmp(target)
    sh "windres #{ src('windows', 'Resources.rc') } #{ tmp(target, 'Resources.o') }"
  end

end
