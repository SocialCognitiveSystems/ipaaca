'''
Created on Dec 20, 2012

@author: hvanwelbergen
'''
from __future__ import with_statement
import threading

from ipaaca import IUEventType
from ipaaca import Message
class ComponentNotifier(object):
    '''
    classdocs
    '''

    NOTIFY_CATEGORY = "componentNotify";
    SEND_CATEGORIES = "send_categories";
    RECEIVE_CATEGORIES = "recv_categories";
    STATE = "state";
    NAME = "name";
    FUNCTION = "function";    
    
    def __init__(self, componentName, componentFunction, sendCategories, receiveCategories, outBuffer, inBuffer):
        '''
        Constructor
        '''
        self.componentName = componentName
        self.componentFunction = componentFunction
        self.sendCategories = frozenset(sendCategories)
        self.receiveCategories = frozenset(receiveCategories)
        self.inBuffer = inBuffer
        self.outBuffer = outBuffer
        self.initialized = False
        self.notificationHandlers = []
        self.initializeLock = threading.Lock()
        self.notificationHandlerLock = threading.Lock()
        self.submitLock = threading.Lock()        
    
    def _submit_notify(self, isNew):
        with self.submitLock:
            notifyIU = Message(ComponentNotifier.NOTIFY_CATEGORY)
            notifyIU.payload[ComponentNotifier.NAME] = self.componentName
            notifyIU.payload[ComponentNotifier.FUNCTION] = self.componentFunction
            notifyIU.payload[ComponentNotifier.SEND_CATEGORIES] = ",".join(self.sendCategories)
            notifyIU.payload[ComponentNotifier.RECEIVE_CATEGORIES] =  ",".join(self.receiveCategories)
            notifyIU.payload[ComponentNotifier.STATE] = "new" if isNew else "old"                
            
            self.outBuffer.add(notifyIU)
    
    def _handle_iu_event(self, iu, event_type, local):
        print("handle, iuname:"+iu.payload[ComponentNotifier.NAME]+" component name: "+self.componentName+" state "+iu.payload[ComponentNotifier.STATE])
        if iu.payload[ComponentNotifier.NAME] == self.componentName:
            return
        with self.notificationHandlerLock:
            for h in self.notificationHandlers:
                h(iu, event_type, local)
        if iu.payload[ComponentNotifier.STATE] == "new":
            print("submitting")
            self._submit_notify(False)            

    def add_notification_handler(self, handler):
        with self.notificationHandlerLock:
            self.notificationHandlers.append(handler)
                
    def initialize(self):
        with self.initializeLock:
            if (not self.initialized):
                self.inBuffer.register_handler(self._handle_iu_event, [IUEventType.MESSAGE], ComponentNotifier.NOTIFY_CATEGORY)
                self._submit_notify(True)
                self.initialized = True
            
    
    
            
        