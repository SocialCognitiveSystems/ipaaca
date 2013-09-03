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

#ifndef __IPAACA_UTIL_NOTIFIER_H__
#define __IPAACA_UTIL_NOTIFIER_H__

#include <ipaaca/ipaaca.h>

#define _IPAACA_COMP_NOTIF_CATEGORY "componentNotify"

#define _IPAACA_COMP_NOTIF_NAME      "name"
#define _IPAACA_COMP_NOTIF_FUNCTION  "function"
#define _IPAACA_COMP_NOTIF_STATE     "state"
#define _IPAACA_COMP_NOTIF_SEND_CATS "send_categories"
#define _IPAACA_COMP_NOTIF_RECV_CATS "recv_categories"

#define _IPAACA_COMP_NOTIF_STATE_NEW  "new"
#define _IPAACA_COMP_NOTIF_STATE_OLD  "old"
#define _IPAACA_COMP_NOTIF_STATE_DOWN "down"

namespace ipaaca {
namespace util {

class ComponentNotifier {
	protected:
		ComponentNotifier(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& receiveCategories);
		ComponentNotifier(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& receiveCategories, ipaaca::OutputBuffer::ptr out_buf, ipaaca::InputBuffer::ptr in_buf);
	public:
		static boost::shared_ptr<ComponentNotifier> create(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& receiveCategories);
		static boost::shared_ptr<ComponentNotifier> create(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& receiveCategories, ipaaca::OutputBuffer::ptr out_buf, ipaaca::InputBuffer::ptr in_buf);
	public:
		~ComponentNotifier();
	protected:
		void submit_notify(const std::string& current_state);
		void handle_iu_event(ipaaca::IUInterface::ptr iu, ipaaca::IUEventType event_type, bool local);
	public:
		void add_notification_handler(ipaaca::IUEventHandlerFunction function);
		void initialize();
		void go_down();
	protected:
		ipaaca::OutputBuffer::ptr out_buf;
		ipaaca::InputBuffer::ptr in_buf;
		ipaaca::Lock lock;
		bool initialized, gone_down;
		std::vector<ipaaca::IUEventHandlerFunction> _handlers;
	protected:
		std::string name;
		std::string function;
		std::string state;
		std::string send_categories;
		std::string recv_categories;
	public:
	typedef boost::shared_ptr<ComponentNotifier> ptr;
};


}} // of namespace ipaaca::util

#endif

