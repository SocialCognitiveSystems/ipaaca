/*
 * This file is part of IPAACA, the
 *  "Incremental Processing Architecture
 *   for Artificial Conversational Agents".
 *
 * Copyright (c) 2009-2015 Social Cognitive Systems Group
 *                         (formerly the Sociable Agents Group)
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

/**
 * \file   ipaaca-tester.cc
 *
 * \brief Multifunction tester component, C++ version
 *
 * This file is not used in the ipaaca library, but produces
 * a separate program, if enabled in CMakeLists.txt
 *
 * \author Ramin Yaghoubzadeh (ryaghoubzadeh@uni-bielefeld.de)
 * \date   January, 2017
 */

#include <ipaaca/ipaaca.h>

#include <cstdio>
#include <iomanip>

#ifdef WIN32
	double get_time_as_secs() { return 0.0; } // TODO implement time function for Windows when required
#else
	#include <sys/time.h>
	double get_time_as_secs() {
		struct timeval tv;
		if (gettimeofday(&tv, NULL)) return 0.0;
		return (0.001 * ((double)tv.tv_sec * 1000000.0 + tv.tv_usec));
	}
#endif

class TesterCpp {
	public:
	void handle_iu_inbuf(boost::shared_ptr<ipaaca::IUInterface> iu, ipaaca::IUEventType etype, bool local)
	{
		std::cout << std::fixed << std::setprecision(3) << get_time_as_secs() << " ";
		std::cout << ipaaca::iu_event_type_to_str(etype) << " category=" << iu->category() << " uid=" << iu->uid() << std::endl;
		//
		auto links = iu->get_all_links();
		if (links.size()>0) {
			std::cout << "links={" << std::endl;
			for (auto kv : links) {
				std::cout << "\t" << kv.first << ": [";
				bool first = true;
				for (const auto& lnk : kv.second) {
					if (first) { first=false; } else { std::cout << ", "; }
					std::cout << lnk;
				}
				std::cout << "]";
			}
			std::cout << "}" << std::endl;
		}
		//
		std::cout << "payload={" << std::endl;
		for (auto kv : iu->payload()) {
			std::cout << "\t'" << kv.first << "': " << ((std::string) kv.second) << ',' << std::endl;
		}
		std::cout << "}" << std::endl;
	}

	int run()
	{
		ipaaca::OutputBuffer::ptr ob = ipaaca::OutputBuffer::create("testerCpp");
		ipaaca::InputBuffer::ptr ib = ipaaca::InputBuffer::create("testerCpp", std::set<std::string>{""});
		ib->set_resend(true);
		ib->register_handler(boost::bind(&TesterCpp::handle_iu_inbuf, this, _1, _2, _3));
		std::cout << "Listening for all IU events ..." << std::endl;
		while(true) {
			sleep(5);
		}
		return 0;
	}

};

int main(int argc, char** argv)
{
	ipaaca::__ipaaca_static_option_log_level = IPAACA_LOG_LEVEL_DEBUG;
	TesterCpp tester;
	tester.run();
}

