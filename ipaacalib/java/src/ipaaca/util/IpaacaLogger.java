/*
 * This file is part of IPAACA, the
 *  "Incremental Processing Architecture
 *   for Artificial Conversational Agents".
 *
 * Copyright (c) 2009-2016 Social Cognitive Systems Group
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

package ipaaca.util;

import ipaaca.LocalMessageIU;
import ipaaca.OutputBuffer;

import java.util.HashMap;
import java.util.UUID;

import org.apache.commons.lang.StringUtils;

public class IpaacaLogger {

	private static OutputBuffer ob;
	private static final Object lock = new Object();

	private static boolean SEND_IPAACA_LOGS = true;
	private static String MODULE_NAME = "???";

	private static void initializeOutBuffer() {
		synchronized (lock) {
			if (ob == null) {
				ob = new OutputBuffer("LogSender");
			}
		}
	}

	public static void setModuleName(String name) {
		synchronized (lock) {
			MODULE_NAME = name;
		}
	}

	public static void setLogFileName(String fileName, String logMode) {
		initializeOutBuffer();
		LocalMessageIU msg = new LocalMessageIU("log");
		HashMap<String, String> pl = new HashMap<String, String>();
		pl.put("cmd", "open_log_file");
		pl.put("filename", fileName);
		if (logMode != null) {
			if (logMode.equals("append") ||
					logMode.equals("overwrite") ||
					logMode.equals("timestamp")) {
				pl.put("existing", logMode);
			} else {

				return;
			}
		}
		ob.add(msg);
	}

	public static void sendIpaacaLogs(boolean flag) {
		synchronized (lock) {
			SEND_IPAACA_LOGS = flag;
		}
	}

	private static void logConsole(String level, String text, float now, String function, String thread) {
		for(String line: text.split("\n")) {
			   System.out.println("[" + level + "] " + thread + " " + function + " " + line);
			   function = StringUtils.leftPad("", function.length(), ' ');
			   thread = StringUtils.leftPad("", thread.length(), ' ');
		}
	}

	private static void logIpaaca(String level, String text, float now, String function, String thread) {
		initializeOutBuffer();
		LocalMessageIU msg = new LocalMessageIU("log");
		HashMap<String, String> pl = new HashMap<String, String>();
		pl.put("module", MODULE_NAME);
		pl.put("function", function);
		pl.put("level", level);
		pl.put("time", String.format("%.3f", now));
		pl.put("thread", thread);
		pl.put("uuid", UUID.randomUUID().toString());
		pl.put("text", text);
		msg.setPayload(pl);
		ob.add(msg);
	}

	private static String getCallerName() {
		String function = Thread.currentThread().getStackTrace()[3].getClassName();
		function += "." + Thread.currentThread().getStackTrace()[3].getMethodName();
		return function;
	}

	public static void logError(String msg) {
		logError(msg, System.currentTimeMillis(), getCallerName());
	}

	public static void logError(String msg, float now) {
		logError(msg, now, getCallerName());
	}

	private static void logError(String msg, float now, String callerName) {
		String thread = Thread.currentThread().getName();
		if (SEND_IPAACA_LOGS) {
			logIpaaca("ERROR", msg, now, callerName, thread);
		}
		logConsole("ERROR", msg, now, callerName, thread);
	}


	public static void logWarn(String msg) {
		logWarn(msg, System.currentTimeMillis(), getCallerName());
	}

	public static void logWarn(String msg, float now) {
		logWarn(msg, now, getCallerName());
	}

	private static void logWarn(String msg, float now, String callerName) {
		String thread = Thread.currentThread().getName();
		if (SEND_IPAACA_LOGS) {
			logIpaaca("WARN", msg, now, callerName, thread);
		}
		logConsole("WARN", msg, now, callerName, thread);
	}


	public static void logInfo(String msg) {
		logInfo(msg, System.currentTimeMillis(), getCallerName());
	}

	public static void logInfo(String msg, float now) {
		logInfo(msg, now, getCallerName());
	}

	private static void logInfo(String msg, float now, String callerName) {
		String thread = Thread.currentThread().getName();
		if (SEND_IPAACA_LOGS) {
			logIpaaca("INFO", msg, now, callerName, thread);
		}
		logConsole("INFO", msg, now, callerName, thread);
	}


	public static void logDebug(String msg) {
		logDebug(msg, System.currentTimeMillis(), getCallerName());
	}

	public static void logDebug(String msg, float now) {
		logDebug(msg, now, getCallerName());
	}

	private static void logDebug(String msg, float now, String callerName) {
		String thread = Thread.currentThread().getName();
		if (SEND_IPAACA_LOGS) {
			logIpaaca("DEBUG", msg, now, callerName, thread);
		}
		logConsole("DEBUG", msg, now, callerName, thread);
	}

}
