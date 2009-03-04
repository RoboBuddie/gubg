require("digest/md5")

# Represents a file with extra information that can be added
class FileInfo
  def initialize(name, info = {})
    @ext = File.extname(name)
    @basename = File.basename(name, @ext)
    @info = info
  end
  def name
    @basename + "." + Digest::MD5.hexdigest(canonicalize) + @ext
  end
  def []=(key, v)
    case v
    when String
    when Array
      v.each do |el|
        raise "Non-String element found in Array." if !(String === el)
      end
    else
      raise "I can only deal with Strings and Arrays of Strings"
    end
    raise "Key \"#{key}\" already exists." if @info.has_key?(key)
    @info[key] = v
    v
  end
  def [](key)
    @info[key]
  end

  def canonicalize
    res = ""
    @info.keys.sort.each do |k|
      v = @info[k]
      case v
      when String
        res += "#{k}: #{v}\n"
      when Array
        v.each { |el| raise "Non-String element found in Array." if !(String === el)}
        res += "#{k}: [#{v.join(', ')}]"
      else
        raise "I can only deal with Strings or Arrays of Strings"
      end
    end
    res
  end
end

class FileStore
  def initialize(base = "./")
    @base = base
    FileUtils.mkdir_p(@base) if !File.exist?(@base)
  end
  def create(fi)
    raise "No block given to create the file." if !block_given?
    fileName = name(fi)
    yield(fileName) if !File.exist?(fileName)
    raise "Could not find file \"#{fileName}\"" if !File.exist?(fileName)
    true
  end
  def name(fi)
    File.expand_path(fi.name, @base)
  end
end

if __FILE__ == $0
  fi = FileInfo.new("test.o")
  fi["date"] = "asdfsadf"
  puts fi.name

  fs = FileStore.new
  puts fs.name(fi)
  fs.create(fi) do |fn|
    puts("Creating file \"#{fn}\"")
    File.open(fn, "w"){}
  end
  fs.create(fi) do |fn|
    raise("Creating file \"#{fn}\" for the second time")
    File.open(fn, "w"){}
  end
end
