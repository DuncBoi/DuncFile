#pragma once

#include <ostream>
#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <utility>

namespace mymake {

/*
 * DO NOT REMOVE any existing data members of Rule or Maker.
 */

struct Rule {
	bool phony = false, implicit_dep = false;
	std::deque<std::string> commands;

	const std::deque<std::string>& deps() const { return deps_dq; }

	// This interface guarantees that dependencies are
	// ordered and unique.
	void push_dep(std::string dep)
	{
		auto ins = deps_set.insert(std::move(dep));
		if (!ins.second)
			return;
		deps_dq.push_back(*ins.first);
	}
	// Add implicit dependency to the front,
	// sets implicit_dep if successful.
	void push_impl_dep(std::string dep)
	{
		auto ins = deps_set.insert(std::move(dep));
		if (!ins.second)
			return;
		implicit_dep = true;
		deps_dq.push_front(*ins.first);
	}

private:
	std::deque<std::string> deps_dq;
	std::unordered_set<std::string> deps_set;
};

class Maker {
public:
	Maker();

	void make() const
	{
		make(default_rule->first);
	}
	void make(const std::string& target) const
	{
		make(target, {});
	}

	friend std::ostream& operator<<(std::ostream& output, const Maker& maker);

private:
	// Internal recursive overload of `make()`;
	// tracks `seen` targets to catch circular dependencies,
	// returns bool to indicate whether circular dependency was found.
	bool make(const std::string& target, std::unordered_set<std::string> seen) const;

	// `rules` maps targets to rules.
	// `default_rule` points to the key-value pair in `rules` for the default target.
	std::unordered_map<std::string, Rule> rules;
	std::pair<const std::string, Rule> *default_rule = nullptr;
};

inline std::ostream& operator<<(std::ostream& output, const Maker& maker)
{
	auto put_tar_rule = [&output](const std::pair<const std::string, Rule>& tar_rule)
	{
		const Rule& rule = tar_rule.second;
		if (rule.phony)
			output << "(phony)" << '\n';
		output << tar_rule.first << ": ";
		for (const std::string& dep : rule.deps())
			output << dep << " ";
		output << '\n';
		for (const std::string& comm : rule.commands)
			output << '\t' << comm << '\n';
		output << '\n';
	};

	put_tar_rule(*maker.default_rule);
	for (const auto& tar_rule : maker.rules)
		if (&tar_rule != maker.default_rule)
			put_tar_rule(tar_rule);

	return output;	
}

inline std::ofstream& operator<<(std::ofstream& os, const std::pair<std::string,Rule>& pr){
	os << pr.first << "\n";
	os << (pr.second.phony       ? 1 : 0)
       << " " 
       << (pr.second.implicit_dep ? 1 : 0)
       << "\n";
	
	auto const &deps = pr.second.deps();
    os << deps.size() << "\n";
    for (auto &d : deps) os << d << "\n";
    //write commands
    os << pr.second.commands.size() << "\n";
    for (auto &c : pr.second.commands) {
		os << c << "\n";
	}
    return os;
}

inline std::ifstream& operator>>(
    std::ifstream& is,
    std::pair<std::string,Rule>& pr
) {
    std::string key;
    if (!std::getline(is, key)) return is; 

    Rule r;
    int ph, im;
    is >> ph >> im; 
    r.phony       = (ph != 0);
    r.implicit_dep = (im != 0);

    size_t n;
    is >> n;
    is.ignore();  
    for (size_t i = 0; i < n; ++i) {
      std::string d;
      std::getline(is, d);
      r.push_dep(d);
    }

    is >> n;
    is.ignore();
    for (size_t i = 0; i < n; ++i) {
      std::string cmd;
      std::getline(is, cmd);
      r.commands.push_back(cmd);
    }

    pr = { key, r };
    return is;
}

} // namespace mymake
