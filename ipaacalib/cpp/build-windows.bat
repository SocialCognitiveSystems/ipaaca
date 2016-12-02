@echo off
md build
cd build
md ipaaca

\Users\ryaghoub\Documents\Develop\repo\protobuf\bin\protoc.exe --proto_path=../../proto ../../proto/ipaaca.proto --cpp_out=build/ipaaca/

cmake .. -DBoost_DEBUG:bool=True -DBOOST_LIBRARYDIR:string=C:\Users\ryaghoub\Downloads\boost_1_57_0__built__boost_subdir_moved_to_stage_include\stage\lib -DBoost_USE_STATIC_LIBS=ON -DBoost_USE_MULTITHREADED=ON -DPROTOBUF_ROOT:string=C:\Users\ryaghoub\Documents\Develop\repo\protobuf

msbuild INSTALL.vcxproj

