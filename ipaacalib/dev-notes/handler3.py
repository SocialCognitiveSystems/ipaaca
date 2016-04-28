class IUEventHandler(object):

	def __init__(self, handler_function):
		self._handler_function = handler_function

	def __call__(self, event):
		if self._filter(event):
			self._handler_function(
				#iu=iu,
				#event_type=event_type,
				#local=local
				)


class IUUpdate(object):
	def __init__(self):
		iu # future if not yet received
		changed_items#: path, value, operation, argument


class Transaction(object):

	def __init__(self, ):
		items = [iu1, iu2, iu_update]
		
	def categories(): pass

	def __len__(): pass

	def new_ius(categories=None):
		pass

	def updates(categories=None):
		pass