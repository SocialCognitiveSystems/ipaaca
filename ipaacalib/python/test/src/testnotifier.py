'''
Created on Dec 20, 2012

@author: hvanwelbergen
'''
import unittest
from mockito import mock
from mockito import verify
from mockito import any
from mockito import when
from mockito import times
from notifier import ComponentNotifier
from hamcrest.core.base_matcher import BaseMatcher
from ipaaca import IUEventType
from ipaaca import Message
from ipaaca import InputBuffer
from ipaaca import OutputBuffer
import time
import os

class IUCategoryMatcher(BaseMatcher):
    def __init__(self, expected_cat):
        self.expected_cat = expected_cat
    
    def _matches_(self, iu):
        return iu.category==self.expected_cat
    
    def describe_to(self, description):
        description.append_text("IU with category :"+self.expected_cat)
        
class ComponentNotifierTest(unittest.TestCase):

    RECV_CAT = {"testrec1", "testrc2"}
    SEND_CAT = {"testsnd1", "testsnd2", "testsnd3"}
    def setUp(self):
        self.mockOutBuffer = mock()
        self.mockInBuffer = mock()
        self.notifier = ComponentNotifier("testcomp","testfunc", ComponentNotifierTest.SEND_CAT, ComponentNotifierTest.RECV_CAT, self.mockOutBuffer, self.mockInBuffer)
        self.notifier.initialize()
    
    def tearDown(self):
        pass
    
    def _sendNotify(self, state, receiveCats):        
        mockIUNotify = Message(ComponentNotifier.NOTIFY_CATEGORY)
        mockIUNotify.payload[ComponentNotifier.STATE] = state;
        mockIUNotify.payload[ComponentNotifier.NAME] = "namex";
        mockIUNotify.payload[ComponentNotifier.SEND_CATEGORIES] = "";
        mockIUNotify.payload[ComponentNotifier.RECEIVE_CATEGORIES] = ",".join(receiveCats);
        self.notifier._handle_iu_event(mockIUNotify, IUEventType.ADDED, False)        
    
    def testNotifyAtInit(self):        
        verify(self.mockOutBuffer).add(any())   
        #TODO: python mockito cannot yet use hamcrest matchers, so cannot easily test if the message is correct :(
        #assertEquals(ComponentNotifier.NOTIFY_CATEGORY, iu.getCategory());
        #assertEquals("new", iu.getPayload().get(ComponentNotifier.STATE));
        #assertThat(ImmutableSet.copyOf(iu.getPayload().get(ComponentNotifier.RECEIVE_CATEGORIES).split(",")),
        #        IsIterableContainingInAnyOrder.containsInAnyOrder(RECV_CAT.toArray(new String[0])));
        #assertThat(ImmutableSet.copyOf(iu.getPayload().get(ComponentNotifier.SEND_CATEGORIES).split(",")),
        #        IsIterableContainingInAnyOrder.containsInAnyOrder(SEND_CAT.toArray(new String[0])));
      
    def testNotifyAtNotifyNew(self):
        self._sendNotify("new", {"testsnd1"});
        verify(self.mockOutBuffer, times(2)).add(any())
        #TODO: python mockito cannot yet use hamcrest matchers, so cannot easily test if the message is correct :(
        #ArgumentCaptor<LocalIU> argument = ArgumentCaptor.forClass(LocalIU.class);
        #verify(mockOutBuffer, times(2)).add(argument.capture());
        #LocalIU iu = argument.getAllValues().get(1);
        #assertEquals("componentNotify", iu.getCategory());
        #assertEquals("old", iu.getPayload().get("state"));
    
    def testNoNotifyAtNotifyOld(self):
        self._sendNotify("old", {"testsnd1"});
        verify(self.mockOutBuffer, times(1)).add(any())

class MyListener(object):
    def __init__(self):
        self.numCalled = 0
    def handle(self, iu, mytype, local):
        self.numCalled += 1 
    
class ComponentNotifierIntegrationTest(unittest.TestCase):
    def setUp(self):
        pass
    
    def tearDown(self):
        pass
    
    def _setupCompNotifier(self, idx, sendList, recvList):
        inBuffer = InputBuffer(idx+"in", {ComponentNotifier.NOTIFY_CATEGORY})
        outBuffer = OutputBuffer(idx+"out")
        return ComponentNotifier(idx, "testfunction", sendList, recvList, outBuffer, inBuffer)

#    bug: this somehow remains active after running
#    def testSelf(self):
#        notifier = self._setupCompNotifier("not", {"a1","b1"}, {"a3","b1"});
#        listener = MyListener()
#        notifier.add_notification_handler(listener.handle);
#        
#        notifier.initialize();    
#        time.sleep(0.5);
#        
#        self.assertEquals(0, listener.numCalled);
    
    def testTwo(self):
        notifier1 = self._setupCompNotifier("not1", {"a1", "b1"}, {"a3", "b2"});
        notifier2 = self._setupCompNotifier("not2", {"a2", "b2"}, {"a3", "b1"});
        listener1 = MyListener()
        listener2 = MyListener()
        notifier1.add_notification_handler(listener1.handle)
        notifier2.add_notification_handler(listener2.handle)
        
        notifier1.initialize()
        time.sleep(0.5)
        notifier2.initialize()
        time.sleep(0.5)        
        self.assertEqual(1, listener1.numCalled)
        self.assertEqual(1, listener2.numCalled)        
        
if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()
    os._exit(0) 