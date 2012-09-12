#include <ipaaca/ipaaca.h>
#include <typeinfo>

using namespace ipaaca;

const char RECV_CATEGORY[] = "WORD";
const char SEND_CATEGORY[] = "TEXT";

class TextSender {
	protected:
		OutputBuffer::ptr _ob;
		InputBuffer::ptr  _ib;
	public:
		TextSender();
		void outbuffer_handle_iu_event(IUInterface::ptr iu, IUEventType event_type, bool local);
		void inbuffer_handle_iu_event(IUInterface::ptr iu, IUEventType event_type, bool local);
		IUInterface::ptr find_last_iu();
		void publish_text_to_print(const std::string& text, const std::string& parent_iu_uid="");
};

TextSender::TextSender() {
	_ob = OutputBuffer::create("TextSenderOut");
	_ob->register_handler(boost::bind(&TextSender::outbuffer_handle_iu_event, this, _1, _2, _3));
	_ib = InputBuffer::create("TextSenderIn", RECV_CATEGORY);
	_ib->register_handler(boost::bind(&TextSender::inbuffer_handle_iu_event, this, _1, _2, _3));
}

void TextSender::outbuffer_handle_iu_event(IUInterface::ptr iu, IUEventType event_type, bool local)
{
	std::cout << "(own IU event " << iu_event_type_to_str(event_type) << " " << iu->uid() << ")" << std::endl;
	if (event_type == IU_UPDATED) {
		std::set<std::string> parent_uids = iu->get_links("GRIN");
		if (parent_uids.size() > 0) {
			std::string parent_uid = *(parent_uids.begin());
			std::cout << "updating parent ..." << std::endl;
			std::set<std::string> next_uids = iu->get_links("SUCCESSOR");
			if (next_uids.size() > 0) {
				std::string next_uid = *(next_uids.begin());
				IUInterface::ptr next_iu = _ob->get(next_uid);
				std::set<std::string> next_letter_grin_links = next_iu->get_links("GRIN");
				if (next_letter_grin_links.count(parent_uid) == 0) {
					// next letter belongs to new word
					IUInterface::ptr parent_iu = _ib->get(parent_uid);
					parent_iu->payload()["STATE"] = "REALIZED";
				} else {
					IUInterface::ptr parent_iu = _ib->get(parent_uid);
					parent_iu->payload()["STATE"] = "STARTED";
				}
			} else {
				// there are no more letters, this is the end of the final word
				IUInterface::ptr parent_iu = _ib->get(parent_uid);
				parent_iu->payload()["STATE"] = "REALIZED";
			}
			std::cout << " ... done." << std::endl;
		}
	} else {
	}
}

void TextSender::inbuffer_handle_iu_event(IUInterface::ptr iu, IUEventType event_type, bool local)
{
	if (event_type == IU_LINKSUPDATED) {
		std::cout << "links updated" << std::endl;
	} else if (event_type == IU_ADDED) {
		std::string word = iu->payload()["WORD"];
		std::cout << "Received new word: " << word << std::endl;
		publish_text_to_print(word, iu->uid());
	} else if (event_type == IU_RETRACTED) {
		std::string retracted_uid = iu->uid();
	} else {
		std::cout << "(IU event " << iu_event_type_to_str(event_type) << " " << iu->uid() << ")" << std::endl;
	}
}

IUInterface::ptr TextSender::find_last_iu() {
	std::set<IUInterface::ptr> ius = _ob->get_ius();
	for (std::set<IUInterface::ptr>::iterator it = ius.begin(); it!=ius.end(); ++it) {
		if ((*it)->get_links("SUCCESSOR").size() == 0) return *it;
	}
	return IUInterface::ptr();
}

void TextSender::publish_text_to_print(const std::string& text, const std::string& parent_iu_uid) {
	IUInterface::ptr previous_iu = find_last_iu();
	if (previous_iu) {
		// insert a blank if we already have words in the buffer
		IU::ptr iu = IU::create( SEND_CATEGORY );
		iu->payload()["CONTENT"] = " ";
		_ob->add(iu);
		previous_iu->add_link( "SUCCESSOR", iu->uid() );
		iu->add_link( "PREDECESSOR", previous_iu->uid() );
		if (parent_iu_uid != "") iu->add_link( "GRIN", parent_iu_uid );
		previous_iu = iu;
	}
	for (int i=0; i<text.size(); ++i) {
		IU::ptr iu = IU::create( SEND_CATEGORY );
		iu->payload()["CONTENT"] = std::string(1, text.at(i));
		_ob->add(iu);
		if (previous_iu) {
			previous_iu->add_link( "SUCCESSOR", iu->uid() );
			iu->add_link( "PREDECESSOR", previous_iu->uid() );
			if (parent_iu_uid != "") iu->add_link( "GRIN", parent_iu_uid );
		}
		if (previous_iu) std::cout << "previous IU: " << *previous_iu << std::endl;
		previous_iu = iu;
	}
}
	
int old_main() {
	TextSender sender;
	sleep(1);
	sender.publish_text_to_print("(INIT)");
	std::cout << "Press Ctrl-C to cancel..." << std::endl;
	while (true) sleep(1);
}

int main() {
	std::cerr << "TODO: implement Ipaaca C++ test cases." << std::endl;
	return 0;
}

