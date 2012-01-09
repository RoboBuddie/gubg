require("targets.rb")
require("gubg/target")
require("gubg/options")

options = parseOptions(name: "Generic build utility", author: "Geert Fannes", version: "0.1") do |parser, options|
    parser.on("-e", "--executable SOURCE", "Produce executable from SOURCE"){|filename|options[:executable] = filename}
end
$verbose = options[:verbose]

location = Location.new
configs = Configs.new
trees = Trees.new
cppFiles = CppFiles.new
hppFiles = HppFiles.new
compileSettings = CompileSettings.new
linkSettings = LinkSettings.new
objectFiles = ObjectFiles.new
executables = Executables.new
targets = [location, configs, trees, cppFiles, hppFiles, compileSettings, linkSettings, objectFiles, executables]
targetGraph = TargetGraph.new(targets)

if options[:executable]
    filename = File.expand_path(options[:executable])
    location.set(File.dirname(filename))
    cppFiles.starter = filename
end

cycleCount = -1
loop do
    cycleCount += 1
    puts("\nRunning cycle #{cycleCount}")
    res = targetGraph.process
    puts("\tCycle #{cycleCount} resulted in \"#{res}\"")
    case res
    when :direct
    when :implicit
    else
        break
    end
end