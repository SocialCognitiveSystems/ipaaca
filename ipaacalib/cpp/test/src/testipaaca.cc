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
#include <typeinfo>

#define BOOST_TEST_MODULE TestIpaacaCpp
#include <boost/test/unit_test.hpp>

using namespace ipaaca;

class TestReceiver {
	public:
		InputBuffer::ptr  _ib;
		std::string received_info;
		TestReceiver();
		void inbuffer_handle_iu_event(IUInterface::ptr iu, IUEventType event_type, bool local);
};

TestReceiver::TestReceiver()
{
	_ib = ipaaca::InputBuffer::create("TestReceiver", "cppTestCategory");
	_ib->register_handler(boost::bind(&TestReceiver::inbuffer_handle_iu_event, this, _1, _2, _3));
	received_info = "NOTHING RECEIVED YET";
}

void TestReceiver::inbuffer_handle_iu_event(IUInterface::ptr iu, IUEventType event_type, bool local)
{
	if (event_type == IU_ADDED) {
		received_info = (std::string) iu->payload()["word"];
		{
			ipaaca::Locker locker(iu->payload());
			iu->payload()["replyVector"] = std::vector<double> { 1.0, 2.0, 3.0 };
			iu->payload()["replyComment"] = "OK";
		}
	}
}


class TestSender {
	public:
		OutputBuffer::ptr _ob;
		std::vector<double> double_vec;
		std::string comment;
		long num_replies;
		TestSender();
		void publish_one_message();
		void outbuffer_handle_iu_event(IUInterface::ptr iu, IUEventType event_type, bool local);
};

TestSender::TestSender() {
	_ob = OutputBuffer::create("TestSender");
	_ob->register_handler(boost::bind(&TestSender::outbuffer_handle_iu_event, this, _1, _2, _3));
	comment = "NO COMMENT YET";
	num_replies = 0;
}

void TestSender::outbuffer_handle_iu_event(IUInterface::ptr iu, IUEventType event_type, bool local)
{
	if (event_type == IU_UPDATED) {
		num_replies++;
		double_vec = iu->payload()["replyVector"];
		comment = (std::string) iu->payload()["replyComment"];
	}
}

void TestSender::publish_one_message()
{
	ipaaca::IU::ptr iu = ipaaca::IU::create("cppTestCategory");
	iu->payload()["word"] = "OK";
	_ob->add(iu);
}

BOOST_AUTO_TEST_SUITE (testIpaacaCpp)


BOOST_AUTO_TEST_CASE( testIpaacaCpp01 )
{
	TestSender sender;
	TestReceiver receiver;
	std::cout << "Publishing one message and waiting 1s for replies from other module." << std::endl;
	sender.publish_one_message();
	sleep(1);
	std::cout << "Checking for changes." << std::endl;
	BOOST_CHECK( receiver.received_info == "OK" );
	BOOST_CHECK( sender.num_replies == 1 );
	BOOST_CHECK( sender.comment == "OK" );
	BOOST_CHECK( sender.double_vec.size() == 3 );
	std::cout << "Complete." << std::endl;
}

BOOST_AUTO_TEST_SUITE_END( )

