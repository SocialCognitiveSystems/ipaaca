#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
Created on Dec 20, 2012

@author: hvanwelbergen
'''
from __future__ import with_statement
import threading

import ipaaca

NotificationState = ipaaca.enum(
    NEW = 'new',
    OLD = 'old',
    DOWN = 'down'
)

class ComponentNotifier(object):

    NOTIFY_CATEGORY = "componentNotify"
    SEND_CATEGORIES = "send_categories"
    RECEIVE_CATEGORIES = "recv_categories"
    STATE = "state"
    NAME = "name"
    FUNCTION = "function"
    
    def __init__(self, componentName, componentFunction, sendCategories, receiveCategories, outBuffer=None, inBuffer=None):
        self.componentName = componentName
        self.componentFunction = componentFunction
        self.sendCategories = frozenset(sendCategories)
        self.receiveCategories = frozenset(receiveCategories)
        self.inBuffer = inBuffer if inBuffer is not None else ipaaca.InputBuffer(componentName + 'Notifier')
        self.outBuffer = outBuffer if outBuffer is not None else ipaaca.OutputBuffer(componentName + 'Notifier')
        self.initialized = False
        self.notificationHandlers = []
        self.initializeLock = threading.Lock()
        self.notificationHandlerLock = threading.Lock()
        self.submitLock = threading.Lock()        
    
    def _submit_notify(self, isNew):
        with self.submitLock:
            notifyIU = ipaaca.Message(ComponentNotifier.NOTIFY_CATEGORY)
            notifyIU.payload = {
                ComponentNotifier.NAME: self.componentName,
                ComponentNotifier.FUNCTION: self.componentFunction,
                ComponentNotifier.SEND_CATEGORIES: ",".join(self.sendCategories),
                ComponentNotifier.RECEIVE_CATEGORIES:  ",".join(self.receiveCategories),
                ComponentNotifier.STATE: NotificationState.NEW if isNew else NotificationState.OLD,
            }          
            self.outBuffer.add(notifyIU)
    
    def _handle_iu_event(self, iu, event_type, local):
        if iu.payload[ComponentNotifier.NAME] == self.componentName:
            return
        with self.notificationHandlerLock:
            for h in self.notificationHandlers:
                h(iu, event_type, local)
        if iu.payload[ComponentNotifier.STATE] == "new":
            #print("submitting")
            self._submit_notify(False)

    def add_notification_handler(self, handler):
        with self.notificationHandlerLock:
            self.notificationHandlers.append(handler)
                
    def initialize(self):
        with self.initializeLock:
            if (not self.initialized):
                self.inBuffer.register_handler(self._handle_iu_event, ipaaca.IUEventType.MESSAGE, ComponentNotifier.NOTIFY_CATEGORY)
                self._submit_notify(isNew=True)
                self.initialized = True


