require("tools/allTools.rb")

system("ce")

tabReplacement = ' '*8

Dir.each('./') do |dir,fn|
  case fn
  when /\.o$/
  when /^a\.out$/
  when /^gmon\.out$/
  when /^execTest/
  when /((\.cpp)|(\.hpp))$/
    fileName = File.expand_path(fn,dir)
    str=String.load(fileName)
    str.gsub("\t",tabReplacement).export(fileName)
    fn = nil
  else
    fn=nil
  end
  if fn
    fn=File.expand_path(fn,dir)
    puts("Removing #{fn}")
    `rm #{fn}`
  end
  nil
end
