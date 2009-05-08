#ifndef gubg_log_hpp
#define gubg_log_hpp

#include <string>
#include <vector>

namespace gubg
{
    class Log
    {
    public:
	class Output
	{
	public:
	    class Primitive
	    {
	    public:
		virtual void write(const std::string &str) = 0;
	    };

	    Output(Primitive *primitive);
	    virtual ~Output();

	    virtual void newLevel(const std::string &tag) = 0;
	    virtual void closeLevel(const std::string &tag) = 0;
	    virtual void newLine() = 0;
	    virtual void closeLine() = 0;
	    virtual Output &operator<<(const std::string &str) = 0;

	    Primitive *swapPrimitive(Primitive *primitive);

	protected:
	    void write(const std::string &str);

	private:
	    // Disallowed
	    Output();
	    Output(const Output &output){};
	    Output &operator=(const Output &output){};

	    Primitive *_primitive;
	};

	class Scope
	{
	public:
	    Scope(const std::string &msg);
	    ~Scope();
	    Scope &operator<<(const std::string &msg);
	    Scope &operator()(const std::string &msg);

	private:
	    std::string _msg;
	    bool _lineIsOpen;
	};

	virtual ~Log();

	static Log &instance();

	static void add(Output *output);

	void newLevel(const std::string &tag);
	void closeLevel(const std::string &tag);
	void newLine();
	void closeLine();
	Output &operator<<(const std::string &str);

    private:
	// Singleton: disallow constructor
	Log();
	// Disallow copying
	Log(const Log& log);
	Log &operator=(const Log& log);

	std::vector<Output*> _outputs;
    };
};

#endif
