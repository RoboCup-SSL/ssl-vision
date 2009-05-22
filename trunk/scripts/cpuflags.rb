#!/usr/bin/ruby
# Copyright 2005-2006 James Bruce.
# LICENSE: Distributed under the GNU General Public Licence, version
# 2, or (at your option) any later version.

def exists(filename)
  Dir[filename] == [filename]
end

# get the flags for a particular processor model
def cpuflags(model)
  case model
    when /Athlon.*64/        then "-march=athlon64 -mfpmath=sse"
    when /Athlon.*XP/        then "-march=athlon-xp"
    when /Athlon/            then "-march=athlon"

    when /Intel.*Core.*i7/   then "-march=core2 -msse4.1 -msse4.2 -mfpmath=sse"
    when /Intel.*Core.*2\s/  then "-march=core2 -mfpmath=sse"
    when /Intel.*Core/       then "-march=pentium-m -mfpmath=sse"
    when /Intel.* T[0-9]*00/ then "-march=pentium-m -mfpmath=sse"
    when /Pentium/           then "-march=pentium"

    when /VIA Nehemiah/ then "-march=pentium"
    else ""
  end
end

# read proc file and extract the processor name
proc_cpu_file = "/proc/cpuinfo"
model = ""
if(exists(proc_cpu_file))
  File.open(proc_cpu_file,"r") {|file|
    file.each_line {|line|
      case line
        when /model name.*: *(.*)/ then model = $1
      end
    }
  }
end

# look up the flags for this model and output it
print cpuflags(model),"\n"

exit 0
