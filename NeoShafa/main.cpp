#include <print>
#include <cstdint>

#include "Router.hpp"

int32_t main(int32_t argc, char** argv) {
	NeoShafa::Router router{ argc, argv };

	router.run();
	
		             
	return 0;
}