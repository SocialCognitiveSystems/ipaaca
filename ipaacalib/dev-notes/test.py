import ipaaca3_pb2

p = ipaaca3_pb2.ProtocolInformation()
p.version = "3.2"
p.auth_token = "test"
print(p.SerializeToString())

t = ipaaca3_pb2.TransactionItem()

for x in t.added_iu:
	print(x)
print(t.added_iu)

print(t.SerializeToString())