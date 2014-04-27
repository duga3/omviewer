#include "ImageList.hpp"

namespace omviewer
{
class OMViewerApp
{
public:
	OMViewerApp(const std::string& image_dir) : im_list_(image_dir) {};
	void run();
	int display(cimg_library::CImg<float>& im, cimg_library::CImgDisplay & disp);
private:
	ImageList im_list_;



};
} // namespace