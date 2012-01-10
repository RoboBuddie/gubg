require("gubg/utils")
require("gubg/target")
require("tree.rb")
require("set")

class Targett
    attr_reader(:state, :msg)
    def setState(state, msg)
        @state, @msg = state, msg
        puts("state: #{@state}::#{@msg}")
        @state
    end
    def state?(wantedState)
        @state == wantedState
    end
    def progressible?
        false
    end
end

class Location < Targett
    attr_reader(:location)
    include Target
    def initialize
        defineDependencies()
    end
    def set(loc)
        @location = loc
    end
    def generate_
        if @location.nil?
            setState(:error, "You have to set the location")
        else
            setState(:generated, "Location is #{@location}")
        end
    end
end
class Configs < Targett
    attr_reader(:compiler, :linker, :roots)
    include Target
    def initialize
        defineDependencies(location: Location)
        @compiler = "g++ -std=c++0x"
        @linker = "g++ -std=c++0x"
    end
    def generate_
        location = getTarget(:location)
        #This is still a short cut
        gubgRoot = File.expand_path("cpp/libs/gubg", ENV["GUBG"])
        @roots = [location.location.dup, gubgRoot]
        str = @roots.join("\n")
        setState(:generated, "The config contains the following roots (#{@roots.length}):\n#{str}")
    end
end
class Trees < Targett
    attr_reader(:trees)
    include Target
    def initialize
        defineDependencies(configs: Configs)
    end
    def generate_
        configs = getTarget(:configs)
        reWantedFiles = /\.[ch]pp$/
            @trees = configs.roots.map{|root|Tree.new(root, reWantedFiles)}
        setState(:generated, "I can use #{@trees.map{|t|t.files}.flatten.length} files in #{@trees.length} trees")
    end
end
class CppFiles < Targett
    attr_reader(:files)
    attr_accessor(:starter)
    include Target
    def initialize
        defineDependencies(hppFiles: HppFiles, trees: Trees, configs: Configs)
        @files = []
        @sourceInfoPerHeader = {}
    end
    def progressible?
        getTarget(:trees).state?(:generated)
    end
    def generate_
        hppFiles, trees, configs = getTargets(:hppFiles, :trees, :configs)
        case generationState
        when :firstTime
            #We search for the specified file in the trees
            trees.trees.each do |tree|
                if file = tree.find(@starter, :exact)
                    @files << file
                    break
                end
            end
            return setState(:error, "The starter #{@starter} could not be found") if @files.empty?
            setState(:generating, "I will start with #{@files.length} files for compilation")
        when :generating
            if hppFiles.state?(:halted)
                setState(:halted, "The headers are not making any progress anymore, I will halt too")
            else
                #Check for new includes
                uncheckedIncludes = hppFiles.files.reject{|f|@sourceInfoPerHeader.has_key?(f)}
                if uncheckedIncludes.empty?
                    return setState(:halted, "No more headers to check")
                end
                uncheckedIncludes.each do |hpp|
                    #We try to find a matching source file for this header
                    cppName = hpp.name.gsub(/\.hp?p?$/, ".cpp")
                    cpp = trees.trees.map{|tree|tree.find(cppName, :exact)}.flatten.compact.first
                    @files << cpp if cpp and !@files.include?(cpp)
                    @sourceInfoPerHeader[hpp] = {file: cpp}
                end
                setState(:generating, "I have currently selected #{@files.length} files for compilation")
            end
        when :halted
            if [:halted, :generated].include?(hppFiles.state)
                str = @files.join("\n")
                setState(:generated, "I will compile the following files (#{@files.length}):\n#{str}")
            else
                setState(:generating, "The headers are still making progress, I will wait some more")
            end
        end
    end
end
class HppFiles < Targett
    attr_reader(:files)
    include Target
    def initialize
        defineDependencies(cppFiles: CppFiles, trees: Trees)
        @files = []
        #Per source file: {ref: "header.h", file: file} (:file is present if found in a tree)
        @includeInfoPerSource = Hash.new{|h, k|h[k] = []}
    end
    def progressible?
        getTarget(:trees).state?(:generated)
    end
    def includePaths
        paths = Set.new
        @includeInfoPerSource.values.each do |ary|
            ary.each do |info|
                if info.has_key?(:file)
                    path = info[:file].name.dup
                    path[info[:ref]] = ""
                    paths << path
                end
            end
        end
        paths.to_a
    end
    def generate_
        cppFiles, trees = getTargets(:cppFiles, :trees)
        if cppFiles.files.empty?
            return setState(:generated, "No cpp files present") if generationState?(:halted)
            return setState(:halted, "No cpp files present (yet)")
        else
            #Search for sources we did not check for headers yet
            uncheckedSources = cppFiles.files.reject{|f|@includeInfoPerSource.has_key?(f)}
            puts("There are #{uncheckedSources.length} unchecked sources")
            if uncheckedSources.empty?
                if generationState?(:halted)
                    str = ""
                    @includeInfoPerSource.each do |s, ary|
                        str += "#{s.name} includes " + ary.map{|info|info[:ref]}.join(", ")
                    end
                    return setState(:generated, "No more cpp files to check:\n#{str}")
                end
                return setState(:halted, "No more sources to check")
            end
            uncheckedSources.each do |cpp|
                refs = extractIncludeRefs_(cpp.name)
                refs.each do |ref|
                    puts("Searching for a match for include ref #{ref}")
                    matches = trees.trees.map{|tree|tree.find(ref, :approx)}.flatten
                    puts("I found #{matches.length} matches: #{matches.map{|m|m.name}}")
                    if matches.empty?
                        @includeInfoPerSource[cpp] << {ref: ref}
                    else
                        bestMatch = matches.sort do |x, y|
                            stringEquality_(cpp.name, x) <=> stringEquality_(cpp.name, y)
                        end.last
                        puts("Best match: #{bestMatch}")
                        @files << bestMatch if !@files.include?(bestMatch)
                        @includeInfoPerSource[cpp] << {ref: ref, file: bestMatch}
                    end
                end
            end
            setState(:generating, "Still checking")
        end
    end
    private
    def HppFiles.stringEquality_(x, y)
        minLength = [x, y].min
        eq = -1
        loop do
            eq += 1
            break if eq >= minLength
            break if x[eq] != y[eq]
        end
        eq
    end
    @@reInclude = /^\#include\s+["<](.+)[">]\s*/
        def extractIncludeRefs_(cpp)
            String.loadLines(cpp).map{|l|l[@@reInclude, 1]}.compact.uniq
        end
end
class CompileSettings < Targett
    attr_reader(:includePaths)
    include Target
    def initialize
        defineDependencies(configs: Configs, trees: Trees, hppFiles: HppFiles, cppFiles: CppFiles)
    end
    def generate_
        hppFiles = getTargets(:hppFiles)
        @includePaths = hppFiles.includePaths
        setState(:generated, "I will use the following include paths: #{@includePaths.join('|')}")
    end
end
class LinkSettings < Targett
    attr_reader(:libraryPaths, :libraries)
    include Target
    def initialize
        defineDependencies(configs: Configs)
    end
    def generate_
        @libraryPaths = []
        @libraries = []
        setState(:generated, "I will link against the following libraries: #{@libraries.join('|')}")
    end
end
class ObjectFiles < Targett
    attr_reader(:objects)
    include Target
    def initialize
        defineDependencies(compile: CompileSettings, cppFiles: CppFiles)
        @objects = []
    end
    def generate_
        cppFiles, compile = getTargets(:cppFiles, :compile)
        includePaths = compile.includePaths.map{|ip|"-I#{ip}"}.join(" ")
        cppFiles.files.each do |cppFile|
            objectFile = cppFile.name.gsub(/\.cpp$/, ".o")
            command = "g++ -std=c++0x -c #{cppFile} -o #{objectFile} #{includePaths}"
            puts("Compile command: #{command}")
            @objects << objectFile if system(command)
        end
        setState(:generated, "I linked #{@objects.length} objects")
    end
end
class Executables < Targett
    attr_reader(:executable)
    include Target
    def initialize
        defineDependencies(link: LinkSettings, objects: ObjectFiles)
        @executable = ""
    end
    def generate_
        objects = getTargets(:objects)
        command = "g++ -o exe " + objects.objects.join(" ")
        system(command)
        setState(:generated, "I compiled #{@executable}")
    end
end
