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


import java.util.Set;


class BufferConfiguration {
	String _owningComponentName;
	Set<String> _category_interests;
	String _channel;
	boolean _resendActive;
	// protected:
	// 	IPAACA_MEMBER_VAR_EXPORT std::string _basename;
	// 	IPAACA_MEMBER_VAR_EXPORT std::vector<std::string> _category_interests;
	// 	IPAACA_MEMBER_VAR_EXPORT std::string _channel;

	public BufferConfiguration(String owningComponentName) {
		this._owningComponentName = owningComponentName;
		this._channel = "default";
		this._resendActive = false;
	}

	public String getOwningComponentName() {
		return this._owningComponentName;
	}

	public Set<String> getCategoryInterests() {
		return this._category_interests;
	}

	public String getChannel() {
		return this._channel;
	}

	public boolean getResendActive() {
		return this._resendActive;
	}
	// public:
	// 	IPAACA_HEADER_EXPORT inline BufferConfiguration(const std::string basename) { _basename = basename; }
	// 	IPAACA_HEADER_EXPORT const std::string get_basename() const { return _basename; }
	// 	IPAACA_HEADER_EXPORT const std::vector<std::string> get_category_interests() const { return _category_interests; }
	// 	IPAACA_HEADER_EXPORT const std::string get_channel() const { return _channel; }

}
