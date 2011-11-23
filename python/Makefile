all:
	@echo "Compiling protobuf spec for python..."
	protoc --proto_path=../proto ../proto/ipaaca.proto --python_out=build/
	cp src/ipaaca.py build/
	( cd build; python -c "import ipaaca" )
	rm build/ipaaca.py
	rm build/ipaaca_pb2.py

clean:
	rm -f build/*.pyc build/*_pb2.py build/ipaaca.py

