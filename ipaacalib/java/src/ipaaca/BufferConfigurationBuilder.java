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


package ipaaca;

class BufferConfigurationBuilder extends BufferConfiguration {

	public BufferConfigurationBuilder(String owningComponentName) {
		super(owningComponentName);
	}

	public void setOwningComponentName(String owningComponentName) {
		this._owningComponentName = owningComponentName;
	}

	public void addCategoryInterest(String category) {
		this._category_interests.add(category);
	}

	public void setChannel(String channel) {
		this._channel = channel;
	}

	public void setResendActive(boolean resendActive) {
		this._resendActive = resendActive;
	}

	public BufferConfiguration getBufferConfiguration() {
		return this;
	}

	// public:
	// 	IPAACA_HEADER_EXPORT inline BufferConfigurationBuilder(const std::string basename):BufferConfiguration(basename) {}
	// 	IPAACA_HEADER_EXPORT inline void set_basename(const std::string& basename)
	// 	{
	// 		_basename = basename;
	// 	}
	// 	IPAACA_HEADER_EXPORT inline void add_category_interest(const std::string& category)
	// 	{
	// 		_category_interests.push_back(category);
	// 	}
	// 	IPAACA_HEADER_EXPORT inline void set_channel(const std::string& channel)
	// 	{
	// 		_channel = channel;
	// 	}

	// 	IPAACA_HEADER_EXPORT const BufferConfiguration& get_BufferConfiguration() { return *this; }
}
