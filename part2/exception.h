#ifndef MYMAKE_EXCEPTION_H
#define MYMAKE_EXCEPTION_H

#include <exception>
#include <string>

namespace mymake{
namespace exception {

class MyMakeException : public std::exception {
	protected:
		std::string msg;
	public:
		MyMakeException(std::string msg) : msg(msg){}

		const char* what() const noexcept override{
			return msg.c_str();
		}
};

class NonFatal : public MyMakeException {
	public:
		using MyMakeException::MyMakeException;
};

class Fatal : public MyMakeException {
	public:
		using MyMakeException::MyMakeException;
};

class ParseError : public Fatal {
	public:
		ParseError(int lineno, const std::string& reason) : Fatal(
				"Parse error on line " + 
				std::to_string(lineno) +
				": " +
				reason
			){}
};

class NoMyMakefile : public Fatal {
	public:
		NoMyMakefile() : Fatal("No MyMakefile found") {}
};

class NoTargets : public Fatal {
	public:
    		NoTargets() : Fatal("No targets specified in MyMakefile") {}
};

class TargetNoRule : public Fatal {
	public:
       		TargetNoRule(const std::string& target) : Fatal(
				"No rule to make target '" + 
				target + 
				"'") {}
};

class TargetCommandFailed : public Fatal {
	public:
		TargetCommandFailed(const std::string& target, int exit_status) : Fatal(
				"Command for target '" +
				target +
				"' failed with exit status " +
				std::to_string(exit_status)), exit_stat(exit_status) {}

		int exit_status() const noexcept { return exit_stat; }
	private:
		int exit_stat;
};

class TargetNothingToDo : public NonFatal {
public:
    	TargetNothingToDo(const std::string& target) : NonFatal(
			"Nothing to be done for target '" + 
			target + 
			"'") {}
};

class TargetUpToDate : public NonFatal {
public:
    	TargetUpToDate(const std::string& target) : NonFatal(
			"Target '" + 
			target + 
		       "' is up to date") {}
};

}}
#endif
