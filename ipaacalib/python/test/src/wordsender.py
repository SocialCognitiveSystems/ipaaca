# This file is part of IPAACA, the
#  "Incremental Processing Architecture
#   for Artificial Conversational Agents".	
#
# Copyright (c) 2009-2013 Sociable Agents Group
#                         CITEC, Bielefeld University	
#
# http://opensource.cit-ec.de/projects/ipaaca/
# http://purl.org/net/ipaaca
#
# This file may be licensed under the terms of of the
# GNU Lesser General Public License Version 3 (the ``LGPL''),
# or (at your option) any later version.
#
# Software distributed under the License is distributed
# on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
# express or implied. See the LGPL for the specific language
# governing rights and limitations.
#
# You should have received a copy of the LGPL along with this
# program. If not, go to http://www.gnu.org/licenses/lgpl.html
# or write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.	
#
# The development of this software was supported by the
# Excellence Cluster EXC 277 Cognitive Interaction Technology.
# The Excellence Cluster EXC 277 is a grant of the Deutsche
# Forschungsgemeinschaft (DFG) in the context of the German
# Excellence Initiative.

import sys
import time
import unittest

import ipaaca

SEND_CATEGORY = 'WORD'

import gobject
import gtk
import cairo

class WordWindow(gtk.Window):
	
	def __init__(self, component):
		super(WordWindow, self).__init__()
		self.running = True
		self.set_title("Word Sender")
		self.set_size_request(640, 200)
		self.set_position(gtk.WIN_POS_CENTER)
		self.connect("destroy", self.quit)
		darea = gtk.DrawingArea()
		darea.connect("expose-event", self.expose)
		darea.set_flags(gtk.CAN_FOCUS)
		darea.grab_focus()
		self.drawing_area = darea
		self.connect("key_press_event",self.key_press)
		self.add(darea)
		self.show_all()
		self.words = []
		self.current_letters = ""
		self.component = component
	
	def quit(self):
		self.running = False
	
	def rounded_rectangle(self, cr, x, y, w, h, r=10):
		cr.move_to(x+r,y)
		cr.line_to(x+w-r,y)
		cr.curve_to(x+w,y,x+w,y,x+w,y+r)
		cr.line_to(x+w,y+h-r)
		cr.curve_to(x+w,y+h,x+w,y+h,x+w-r,y+h)
		cr.line_to(x+r,y+h)
		cr.curve_to(x,y+h,x,y+h,x,y+h-r)
		cr.line_to(x,y+r)
		cr.curve_to(x,y,x,y,x+r,y)

	def new_word(self, word):
		uid = self.component.publish_new_word(word)
		self.words.append([uid, word, 'NEW'])
	
	def key_press(self, widget, event):
		if event.keyval == gtk.keysyms.Escape:
			self.quit()
			return
		elif event.keyval == gtk.keysyms.BackSpace:
			if len(self.current_letters) > 0:
				self.current_letters = ""
		elif event.keyval > 32 and event.keyval < 127:
			self.current_letters += chr(event.keyval)
		elif event.keyval == 32 or event.keyval == gtk.keysyms.Return:
			if len(self.current_letters) > 0:
				self.new_word(self.current_letters)
				self.current_letters = ""
		else:
			print event, event.keyval
			pass
		x, y, w, h = widget.get_allocation()
		widget.queue_draw_area(x, y, w, h)
	
	def update_word_state(self, uid, state):
		for t in self.words:
			u, w, s = t
			if u == uid:
				print "new state for "+w+": "+state
				t[2] = state
		x, y, w, h = self.drawing_area.get_allocation()
		self.drawing_area.queue_draw_area(x, y, w, h)

	def render_words(self, cr, x=20, y=100):
		for wp in self.words:
			uid, word, state = wp
			x_bearing, y_bearing, width, _ = cr.text_extents(word)[:4]
			_, _, _, height = cr.text_extents("Wy")[:4]
			self.rounded_rectangle(cr, x, y-height-4, width+10, height+10)
			cr.set_line_width(1.0)
			cr.set_source_rgb(0.0, 0.0, 0.0)
			cr.stroke_preserve()
			if state == 'NEW' or state == '':
				cr.set_source_rgb(1.0, 1.0, 1.0)
			elif state == 'STARTED':
				cr.set_source_rgb(0.3, 0.3, 1.0)
			elif state == 'REALIZED':
				cr.set_source_rgb(0.7, 0.7, 0.7)
			else:
				cr.set_source_rgb(1.0, 0.5, 0.5)
			cr.fill()
			cr.move_to(x+5, y)
			cr.set_source_rgb(0.0, 0.0, 0.0)
			cr.show_text(word)
			x += width + 10 + 3
		cr.move_to(x+5, y)
		cr.show_text(self.current_letters)
	
	def expose(self, widget, event):
		cr = widget.window.cairo_create()
		cr.set_source_rgb(0.0, 0.0, 0.0)
		cr.set_font_size(13)
		cr.move_to(20, 30)
		cr.show_text("Type words. Space sends a word. Backspace deletes one.")
		
		self.render_words(cr)

class WordSender(object):
	def __init__(self):
		self.ob = ipaaca.OutputBuffer('WordSenderOut')
		self.ob.register_handler(self.outbuffer_handle_iu_event)
		self.window = None
	
	def outbuffer_handle_iu_event(self, iu, event_type, local):
		if event_type == "UPDATED":
			print(iu.payload['WORD']+': '+iu.payload['STATE'])
			self.window.update_word_state(iu.uid, iu.payload['STATE'])
		else:
			print('(own IU event '+event_type+' '+str(iu.uid)+')')
	
	def find_last_iu(self):
		for iu in self.ob.iu_store.values():
			if len(iu.get_links('SUCCESSOR')) == 0:
				return iu
		return None
	
	def publish_new_word(self, word):
		previous_iu = self.find_last_iu()
		iu = ipaaca.IU( SEND_CATEGORY )
		iu.payload = { 'WORD': word }
		self.ob.add(iu)
		if previous_iu is not None:
			previous_iu.add_links( 'SUCCESSOR', [iu.uid] )
			iu.add_links( 'PREDECESSOR', [previous_iu.uid] )
		return iu.uid

	def publish_words(self, words):
		previous_iu = self.find_last_iu()
		for word in words:
			iu = ipaaca.IU( SEND_CATEGORY )
			iu.payload = { 'WORD': word }
			self.ob.add(iu)
			if previous_iu is not None:
				previous_iu.add_links( 'SUCCESSOR', [iu.uid] )
				iu.add_links( 'PREDECESSOR', [previous_iu.uid] )
			previous_iu = iu

if __name__ == '__main__':
	sender = WordSender()
	window = WordWindow(sender)
	sender.window = window
	#sender.publish_words(['this','is','a','demonstration','of','incremental','generation'])
	#print sender.ob.unique_name
	print "Press Ctrl-C to cancel..."
	#gtk.main()
	lc = gobject.main_context_default()
	while window.running:
		lc.iteration(False)
		time.sleep(0.01)
	#while True:
	#	time.sleep(0.1)
	#

