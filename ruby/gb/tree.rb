#A Tree is a collection of files defined by a specific anchorPoint in a file system:
# * Step from anchorPoint towards "/" until "root.tree" is found
# * All files in all subdirectories from "root.tree" are part of the collection
# * "root.tree: can include other files collections or exclude other files

require("root")
require("gubg/utils")

class Tree
    @@rootFileName = "root.tree"

    attr(:root, true)
    def initialize(anchorPoint)
        @anchorPoint = ::File.expand_path(anchorPoint)
        loadRoot_
        createLocalFileCollection_
        loadSubtrees_
    end
    def Tree.rootFileName
        @@rootFileName
    end

    def each(&block)
        @filesPerExtension.each do |extension, files|
            files.each{|file|block.call(file)}
        end
        @subtrees.each do |subtree|
            puts("\nSubtree")
            subtree.each(block)
        end
    end

    private
    def loadRoot_
        #Find @rootFilePath: step down from @anchorPoint towards "/", until "root.tree" is found
        @rootFilePath = nil
        @rootDirectory = nil
        directory = ::File.directory?(@anchorPoint) ? @anchorPoint : ::File.dirname(@anchorPoint)
        loop do
            tmpRootFile = ::File.expand_path(@@rootFileName, directory)
            if ::File.exist?(tmpRootFile)
                @rootFilePath = tmpRootFile
                @rootDirectory = directory
                break
            end
            nextDirectory = ::File.dirname
            raise("No #{@@rootFile} found") if nextDirectory == directory
        end
        #Load "root.tree"
        puts("rootFilePath = #{@rootFilePath}")
        @root = Root.createFromFile(@rootFilePath)
    end
    class File
        attr(:directory, true)
        attr(:filename, true)
        def initialize(directory, filename)
            @directory, @filename = directory, filename
            @extension = ::File.extname(@filename)
        end
        def extension
            @extension
        end
        def filepath
            ::File.expand_path(@filename, @directory)
        end
        alias to_s filepath
    end
    def createLocalFileCollection_
        @filesPerExtension = Hash.new{|h, k|h[k] = []}
        Dir.each(@rootDirectory) do |dir, fn|
            File.new(dir, fn).tap{|file|@filesPerExtension[file.extension] << file}
            nil
        end
    end
    def loadSubtrees_
        @subtrees = @root.subtrees.collect do |subtree|
            Root.new(subtree)
        end
    end
end

if __FILE__ == $0
    tree = Tree.new("/home/gfannes/gubg/d/gubg/Sequence.d")
    tree.each do |file|
        puts("#{file}")
    end
end
