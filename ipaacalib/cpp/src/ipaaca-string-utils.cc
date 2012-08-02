#include <ipaaca/ipaaca.h>

namespace ipaaca {

std::string str_join(const std::set<std::string>& set,const std::string& sep)
{
	if(set.size()==0)
		return "";
	std::string::size_type size=sep.length()*set.size();
	for(std::set<std::string>::const_iterator it = set.begin(); it !=set.end(); it++)
	{
		size+=(*it).length();
	}
	std::string tmp;
	tmp.reserve(size);
	tmp=*(set.begin());
	std::set<std::string>::const_iterator it = set.begin();
	it++;
	for(; it !=set.end(); it++)
	{
		tmp=tmp+sep+(*it);
	}
	return tmp;
}

std::string str_join(const std::vector<std::string>& vec,const std::string& sep)
{
	if(vec.size()==0)
		return "";
	std::string::size_type size=sep.length()*vec.size();
	for(unsigned int i=0;i<vec.size();i++)
	{
		size+=vec[i].length();
	}
	std::string tmp;
	tmp.reserve(size);
	tmp=vec[0];
	for(unsigned int i=1;i<vec.size();i++)
	{
		tmp=tmp+sep+vec[i];
	}
	return tmp;
}

void str_split_wipe(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters )
{
	tokens.clear();
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		lastPos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, lastPos);
	}
}
void str_split_append(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters )
{
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		lastPos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, lastPos);
	}
}

} // namespace ipaaca


