cmake_minimum_required (VERSION 2.6)

# project name
project (ipaaca_soa_cpp)

install(
	DIRECTORY include
	DESTINATION /
	FILES_MATCHING PATTERN "*.h" PATTERN "*.hh" PATTERN "*.hpp" PATTERN "*.inl"
	)

