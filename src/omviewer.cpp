#include "OMViewerApp.hpp"

using namespace omviewer;

int main(int argc, char * argv[])
{
	std::string image_path = (argc > 1) ? argv[1] : "./";
	OMViewerApp app(image_path);
	app.run();
}