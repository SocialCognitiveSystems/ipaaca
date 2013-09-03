/*
 * This file is part of IPAACA, the
 *  "Incremental Processing Architecture
 *   for Artificial Conversational Agents".  
 *
 * Copyright (c) 2009-2013 Sociable Agents Group
 *                         CITEC, Bielefeld University   
 *
 * http://opensource.cit-ec.de/projects/ipaaca/
 * http://purl.org/net/ipaaca
 *
 * This file may be licensed under the terms of of the
 * GNU Lesser General Public License Version 3 (the ``LGPL''),
 * or (at your option) any later version.
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the LGPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the LGPL along with this
 * program. If not, go to http://www.gnu.org/licenses/lgpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  
 *
 * The development of this software was supported by the
 * Excellence Cluster EXC 277 Cognitive Interaction Technology.
 * The Excellence Cluster EXC 277 is a grant of the Deutsche
 * Forschungsgemeinschaft (DFG) in the context of the German
 * Excellence Initiative.
 */

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


