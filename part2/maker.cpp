#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include "exception.h"
#include "helper.h"
#include "maker.h"
#include "xdb.h"

using namespace mymake;

Maker::Maker()
{
	const std::string makefile = "MyMakefile";
	const std::string cache = ".mymake.cache";

	//check cache
	if (std::filesystem::exists(cache) && std::filesystem::exists(makefile) && std::filesystem::last_write_time(cache) >= std::filesystem::last_write_time(makefile)){
		try {
			//read every cache entry into rules map
            xdb::XdbReader<std::pair<std::string, Rule>> rdr(cache.c_str());
            for (size_t i = 0; i < rdr.size(); ++i) {
                auto pr = rdr[i];
                rules[pr.first] = pr.second;
                if (!default_rule)
                    default_rule = &*rules.find(pr.first);
            }
			//if at least one rule successfully loaded
            if (default_rule) {
				bool cache_is_fresh = true;

				//for every rule with implicit dep, make sure source file still exists
				for (auto & entry: rules) {
					const Rule &r = entry.second;
                    if (!r.phony && r.implicit_dep) {
                        const std::string &src = r.deps().front();
                        if (!std::filesystem::exists(src)) {
                            cache_is_fresh = false;
                            break;
                        }
                    }
                }

				if (cache_is_fresh) {
					//check whether new source file has appeared for any non-phony rule with no commands and no implicit dependencies
                    for (auto & entry: rules) {
						const std::string &t = entry.first;
                        const Rule &r = entry.second;
                        if (!r.phony && r.commands.empty() && !r.implicit_dep) {
                            auto stem = std::filesystem::path(t).stem().string();
                            if (   std::filesystem::exists(stem + ".cpp")
                                || std::filesystem::exists(stem + ".c"))
                            {
                                cache_is_fresh = false;
                                break;
                            }
                        }
                    }
                }

				if (cache_is_fresh){
					return;
				}
            }
        } catch (...){
			//catch any errors with cache and just parse
		}

		//invalidate a half-loaded cache and reparse
		rules.clear();
    	default_rule = nullptr;
	}

	static const std::regex comm_ln("^\\t(.*)$");
	static const std::regex assign_ln("^(.*?)=(.*)$");
	static const std::regex rule_ln("^(.*?):(.*)$");
	static const std::regex var("\\$\\((.*?)\\)");

	std::unordered_map<std::string, std::string> vars = {
		{ "CC", "cc" },
		{ "CXX", "c++" },
	};

	if (!std::filesystem::exists(makefile)) {
		throw exception::NoMyMakefile();
	}

	std::ifstream in(makefile);

	Rule* last_rule = nullptr;
	default_rule = nullptr;

	std::string line;
	int i = 0;
	while (std::getline(in, line)){
		i++;

		//remove comments
		auto commpos = line.find('#');
		if (commpos != std::string::npos){
			line.erase(commpos);
		}

		//scan and replace all variables in vars map with their value
		std::string expanded = line;
		std::smatch vm;
		while (std::regex_search(expanded, vm, var)) {
  			auto nm  = vm[1].str();
			//unassigned variables = empty string
  			auto val = vars.count(nm) ? vars[nm] : "";
  			expanded = vm.prefix().str() + val + vm.suffix().str();
		}

		//remove whitespace and empty lines
		std::string trimLine = helper::trim(expanded);
		if(trimLine.empty()){
			continue;
		}

		//check if line is a command line by looking for tab
		std::smatch m;
    	if (std::regex_match(expanded, m, comm_ln)) {
			//throw parse error if ther is no preceding rule
        	if (!last_rule){
				throw exception::ParseError(i, "commands before first target");
			}
			last_rule -> commands.push_back(m[1].str());
    	}

		//detects variable assignment
    	else if (std::regex_match(trimLine, m, assign_ln)) {
        	auto name = helper::trim(m[1].str());
			if (name.empty()){
				throw exception::ParseError(i, "empty variable name");
			}
			vars[name] = m[2].str();
    	}	

		//handles rule-definition
    	else if (std::regex_match(trimLine, m, rule_ln)) {
			//list of targets
        	auto tgts = helper::split(helper::trim(m[1].str()));
			//list of dependencies
			auto deps = helper::split(helper::trim(m[2].str()));
			if (tgts.empty()){
				throw exception::ParseError(i, "empty target list");
			}

			//loops through the targets & pushes them into dependency list, marks Phony as phony 
			for (auto &t : tgts) {
				Rule &r = rules[t];
				if (t == ".PHONY") {
					r.phony = true;
				}
				for (auto &d : deps) {
					r.push_dep(d);
					if (t == ".PHONY") {
						rules[d].phony = true;
					}
				}
			}
			if (!default_rule) {
				for (auto &t : tgts) {
					if (t != ".PHONY") {
						auto it = rules.find(t);
            			default_rule = &*it;    
						break;
					}
				}
			}
			last_rule = &rules[tgts[0]];
    	}	
    	else {
			throw exception::ParseError(i, "unrecognized syntax");
    	}
	}

	//part 2c
	for (auto &entry : rules) {
		const std::string &target = entry.first;
		Rule &R = entry.second;

		//only generate implicit for non-phony rules with no explicit commands
		if (R.phony || !R.commands.empty()) {
			continue;
		}

		std::filesystem::path p(target);
		//get stem and extension from target
		auto stem = p.stem().string();
    	auto ext  = p.extension().string();

		if (ext == ".o") {
			//check for .cpp first, then .c
			std::filesystem::path cpp = stem + ".cpp";
			std::filesystem::path c   = stem + ".c";
			if (std::filesystem::exists(cpp)) {
				R.push_impl_dep(cpp.string());
				std::ostringstream cmd;
				cmd << vars["CXX"] << " " << vars["CXXFLAGS"]
					<< " -c -o " << target << " " << cpp.string();
				R.commands.push_back(cmd.str());
			}
			else if (std::filesystem::exists(c)) {
				R.push_impl_dep(c.string());
				std::ostringstream cmd;
				cmd << vars["CC"] << " " << vars["CFLAGS"]
					<< " -c -o " << target << " " << c.string();
				R.commands.push_back(cmd.str());
			}
			//no source found, skip implicit rule
			continue;
		}

		//if there is already a .o rule, link that with dependencies
		std::string ofile = target + ".o";
    	if (rules.find(ofile) != rules.end()) {
        	// insert .o at front
        	R.push_impl_dep(ofile);
        	std::ostringstream cmd;
        	cmd << vars["CC"] << " " << vars["LDFLAGS"] << " "
            	<< ofile;
        	for (auto &d : R.deps()) {
				if (d != ofile) {
					cmd << " " << d;
				}
			}
        	cmd << " " << vars["LDLIBS"]
            	<< " -o " << target;
        	R.commands.push_back(cmd.str());
        	continue;
    	}

		std::filesystem::path cpp = stem + ".cpp";
    	std::filesystem::path c   = stem + ".c";
    	if (std::filesystem::exists(cpp)) {
        	R.push_impl_dep(cpp.string());
     		std::ostringstream cmd;
        	cmd << vars["CXX"] << " " << vars["CXXFLAGS"] << " "
            	<< vars["LDFLAGS"] << " "
            	<< cpp.string();
        	for (auto &d : R.deps()){
				if (d != ofile) {
            		cmd << " " << d;
				}
			}
        	cmd << " " << vars["LDLIBS"]
            	<< " -o " << target;
        	R.commands.push_back(cmd.str());
    	}
    	else if (std::filesystem::exists(c)) {
        	R.push_impl_dep(c.string());
        	std::ostringstream cmd;
        	cmd << vars["CC"] << " " << vars["CFLAGS"] << " "
            	<< vars["LDFLAGS"] << " "
            	<< c.string();
        	for (auto &d : R.deps())
            	cmd << " " << d;
        	cmd << " " << vars["LDLIBS"]
            	<< " -o " << target;
        	R.commands.push_back(cmd.str());
    	}
	}

	if (!default_rule) {
		throw exception::NoTargets();
	}

	try {
        xdb::XdbWriter<std::pair<std::string,Rule>> wr(cache.c_str(), false);
		wr << std::make_pair(default_rule->first, rules.at(default_rule->first));
        for (auto &ent : rules) {
			if (ent.first == default_rule->first) continue;
            wr << std::make_pair(ent.first, ent.second);
        }
    } catch (...) { 
		//ignore cache faile
	}
	return;
}

bool Maker::make(const std::string& target, std::unordered_set<std::string> seen) const
{

	auto it = rules.find(target);
	if (it == rules.end()){
		if (!std::filesystem::exists(target)) {
			throw exception::TargetNoRule(target);
		} else {
			throw exception::TargetNothingToDo(target);
		}
	}

	const Rule &rule = it->second;

	//circular dep check
	if (seen.count(target)) {
		std::cerr << "mymake: Circular " << target << " dependency dropped.\n";	
		return true;
	}
	seen.insert(target);

	//make dependencies first
	bool anyBuilt = false;
	for (auto &dep : rule.deps()){
		try {
			bool circ = make(dep, seen);
			if (!circ) {
				anyBuilt = true;
			}
		}
		catch (const exception::NonFatal&){
			//do nothing for non-fatal exceptions
		}
	}

	//up to date check for non-phony targets
	if (!rule.phony && std::filesystem::exists(target)) {
		auto targTime = std::filesystem::last_write_time(target);
		bool allOlder = true;
		for (auto &dep : rule.deps()){
			if (!std::filesystem::exists(dep) || std::filesystem::last_write_time(dep) > targTime) {
				allOlder = false;
				break;
			}
		}
		if (allOlder) {
			throw exception::TargetUpToDate(target);
		}
	}

	if (rule.commands.empty() && !rule.implicit_dep && !anyBuilt) {
		throw exception::TargetNothingToDo(target);
	}

	//executre the commands
	for (auto &cmd : rule.commands) {
		std::cout << cmd << "\n";
		int status = std::system(cmd.c_str());
		int exitCode = WEXITSTATUS(status);
		if (exitCode != 0) {
			throw exception::TargetCommandFailed(target, exitCode);
		}
	}

	return false;
}
