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

#include <../include/ipaaca/ipaaca.h>
#include <typeinfo>
#include <iostream>

using namespace ipaaca;

void my_first_iu_handler(IUInterface::ptr iu, IUEventType type, bool local)
{
	std::cout<<"Update!"<<std::endl;	
}

int main() {
	try{
		OutputBuffer::ptr output_buffer = OutputBuffer::create("Tester");
		IU::ptr iu = IU::create("cat1");
		output_buffer->add(iu);
		
		InputBuffer::ptr input_buffer = InputBuffer::create("InputTester","cat1");
		input_buffer->set_resend(true);
		input_buffer->register_handler(my_first_iu_handler);
		iu->payload()["key"]="Dummy";
		sleep(1);
		std::cout<<"Input buffer #of ius: "<<input_buffer->get_ius().size()<<std::endl;
		
	} catch (ipaaca::Exception& e) {
		std::cout << "== IPAACA EXCEPTION == " << e.what() << std::endl;
	}
}



