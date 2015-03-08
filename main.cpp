// Dan Krupsky, 108
#include <iostream>
#include "pbhash.pb.h"
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

void get_dir_list(fs::directory_iterator iterator) {
	for (; iterator != fs::directory_iterator(); ++iterator)
	{
		std::cout << (iterator->path().filename()) << fs::is_directory(iterator->status()) << std::endl;
		if (fs::is_directory(iterator->status())) {
			std::cout << "KIK" << std::endl;
			fs::directory_iterator sub_dir(iterator->path());
			get_dir_list(sub_dir);

		}

	}
}

int main() {
	fs::directory_iterator home_dir(std::string("."));
	get_dir_list(home_dir);
	std::cin.get();
	return 0;
}
